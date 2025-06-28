#include "subprogram_locator.hpp"
#include "error_code.hpp"
#include <queue>

bool SubprogramLocator::set_default_args(const ID &subprogram_name, const ID &instance_name, DefaultArgs &default_args)
{
    auto subprogram_it = subprograms_.find(subprogram_name);
    if (subprogram_it == subprograms_.end())
        return false;

    Subprogram& subprogram = subprograms_[subprogram_name];

    std::size_t instance_idx;
    for (instance_idx = 0; instance_idx < subprogram.instances_.size(); ++instance_idx) {
        if (subprogram.instances_[instance_idx].name_ == instance_name)
            break;
    }

    if (instance_idx == subprogram.instances_.size())
        return false;


    for (auto& instance : subprogram.instances_) {
        if (instance.name_ == instance_name) {

            for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) {
                if (!default_args[i].empty()) {
                    instance.default_args_[i] = std::move(default_args[i]);
                }
            }

            break;
        }
    }

    return true;
}

SubprogramStates SubprogramLocator::get_subprogram_state(const ID &subprogram_name, const ID& instance_name) const
{
    if (auto it = subprograms_.find(subprogram_name); it != subprograms_.end()) {
        for (const auto& instance : it->second.instances_) {
            if (instance.name_ == instance_name)
                return instance.state_;
        }
    } else {
        return SubprogramStates::NOT_EXISTED;
    }
}

bool SubprogramLocator::add_new_subprogram(const SubprogramInfo &info, bool is_undefined_subprogram)
{
    ReturnOrErrorCode<int> res = dependencies_graph_.get_free_vertex_id();
    if (is_error_code(res))
        return false;

    int new_vertex_id = get_return(res);
    dependencies_graph_.add_vertex(new_vertex_id);
    vertexes_to_subprogram_names_[new_vertex_id] = info.name_;

    subprograms_[info.name_] = Subprogram{
        info,
        {
            SubprogramInstance{
                {
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                    AnyArgs{},
                }, 
                is_undefined_subprogram? SubprogramStates::UNDEFINED : SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT, 
                default_instance_name
            }
        },
        new_vertex_id
    };
    
    SubprogramInfo& subprogram_info = subprograms_[info.name_].info_;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) 
        if (!subprogram_info.funcs[i]) 
            subprogram_info.funcs[i] = empty_func_;

    return true;
}

void SubprogramLocator::update_undefined_subprogram(const SubprogramInfo &info)
{
    Subprogram& dest_subprogram = subprograms_[info.name_];

    dest_subprogram.info_.dependencies_ = info.dependencies_;
    dest_subprogram.info_.is_multiinstance_supported_ = info.is_multiinstance_supported_;
    dest_subprogram.instances_[default_instance_idx].state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT; // dest_subprogram.instances_[0] = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i)
        if (info.funcs[i])
            dest_subprogram.info_.funcs[i] = info.funcs[i];
}

/* 
 * Make dependent subprograms status from "defined without defined parent" to "ready to common initialize"
 * for desired result in hierarchy of dependency subprograms all parent subprograms of child subprograms 
 * must be ready.
 */
void SubprogramLocator::try_make_dependent_subprograms_ready(const ID &ready_parent_subprogram_name)
{
    inverse_dependencies_graph_.dfs(
        subprograms_[ready_parent_subprogram_name].vertex_, 
        [&](int vertex, int, Colors, bool) {
                
        auto &current_subprogram = subprograms_[vertexes_to_subprogram_names_[vertex]];
        if (current_subprogram.instances_[default_instance_idx].state_ != SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT)
            return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;

        bool all_parent_subprograms_defined = true;
        for (auto& parent_subprogram_name : current_subprogram.info_.dependencies_) {
            auto& parent_subprogram = subprograms_[parent_subprogram_name];

            if (parent_subprogram.instances_[default_instance_idx].state_ == SubprogramStates::UNDEFINED 
             || parent_subprogram.instances_[default_instance_idx].state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
                    all_parent_subprograms_defined = false;
                    break;
            }
        }

        if (all_parent_subprograms_defined) {
            current_subprogram.instances_[default_instance_idx].state_ = SubprogramStates::READY_TO_COMMON_INITIALIZE;
            return CallbackCommand::CONTINUE;
        }

        return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;
    });
}

void SubprogramLocator::restore_subprogram_to_undefined(Subprogram &subprogram, const Subprogram &undefined_subprogram, const std::unordered_set<ID> &added_undefined_subprograms)
{
        for (const ID& dependency_name : subprogram.info_.dependencies_) {
            Subprogram& dependency  = subprograms_[dependency_name];
            
            dependencies_graph_.remove_edge(subprogram.vertex_, dependency.vertex_);
            inverse_dependencies_graph_.remove_edge(dependency.vertex_, subprogram.vertex_);

            if (added_undefined_subprograms.find(dependency_name) != added_undefined_subprograms.end()) {
                inverse_dependencies_graph_.remove_vertex(dependency.vertex_);
                vertexes_to_subprogram_names_.erase(dependency.vertex_);
                subprograms_.erase(dependency_name);
            }
        }

        subprogram = undefined_subprogram;
}

bool SubprogramLocator::add(const SubprogramInfo &info)
{
    bool it_was_undefined = false;
    Subprogram undefined_subprogram;

    if (auto it = subprograms_.find(info.name_); it != subprograms_.end()) {

        if (it->second.instances_[default_instance_idx].state_ != SubprogramStates::UNDEFINED)
            return false;

        update_undefined_subprogram(info);

        it_was_undefined = true;

    } else {

        undefined_subprogram = subprograms_[info.name_];
        if (!add_new_subprogram(info))
            return false;

    }

    bool all_parent_subprograms_defined = true;
    std::unordered_set<ID> added_undefined_subprograms;
    Subprogram& subprogram = subprograms_[info.name_];

    for (const auto& dependency : info.dependencies_) {

        if (auto dep_sub = subprograms_.find(dependency); dep_sub == subprograms_.end()) {

            SubprogramInfo undefined_subprogram_info;
            undefined_subprogram_info.name_ = dependency;
            add_new_subprogram(undefined_subprogram_info, false);
            added_undefined_subprograms.insert(dependency);
            all_parent_subprograms_defined = false;

        } else if (dep_sub->second.instances_[default_instance_idx].state_ == SubprogramStates::UNDEFINED 
                || dep_sub->second.instances_[default_instance_idx].state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
                    all_parent_subprograms_defined = false;
        }

        dependencies_graph_.add_edge(subprogram.vertex_, subprograms_[dependency].vertex_);
        inverse_dependencies_graph_.add_edge(subprograms_[dependency].vertex_, subprogram.vertex_);
    }

    if (it_was_undefined && get_return(dependencies_graph_.is_have_structure(subprogram.vertex_, CheckStructureCommand::CYCLE))) {

        restore_subprogram_to_undefined(subprogram, undefined_subprogram, added_undefined_subprograms);
        return false;
    }

    if (all_parent_subprograms_defined) {
        subprogram.instances_[default_instance_idx].state_ = SubprogramStates::READY_TO_COMMON_INITIALIZE;

        if (it_was_undefined)
            try_make_dependent_subprograms_ready(info.name_);
    }

    return true;
}


bool SubprogramLocator::remove(const SubprogramInfo &info)
{

    return false;
}

bool SubprogramLocator::common_init(const ID &subprogram_name, AnyArgs& args)
{
    if (get_subprogram_state(subprogram_name) != SubprogramStates::READY_TO_COMMON_INITIALIZE)
        return false;

    std::queue<int> dependencies_init_order;
    dependencies_graph_.dfs(subprograms_[subprogram_name].vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

        if (is_backtracking) {
            dependencies_init_order.push(vertex);
            return CallbackCommand::CONTINUE;
        }

        if (color != Colors::WHITE)
            return CallbackCommand::CONTINUE;

        auto& subprogram = subprograms_[vertexes_to_subprogram_names_[vertex]];
        if (subprogram.instances_[default_instance_idx].state_ != SubprogramStates::READY_TO_COMMON_INITIALIZE)
            return CallbackCommand::SKIP_CHILDRENS;

        return CallbackCommand::CONTINUE;
    }, true);

    while (!dependencies_init_order.empty()) {
        auto& subprogram = subprograms_[vertexes_to_subprogram_names_[dependencies_init_order.front()]];
        dependencies_init_order.pop();

        AnyArgs empty_args;
        AnyArgs& any_args = empty_args;
        if (!args.empty()) 
            any_args = args;
        else if (!subprogram.instances_[default_instance_idx].default_args_[static_cast<int>(SubprogramFuncNames::COMMON_INIT)].empty())
            any_args = subprogram.instances_[default_instance_idx].default_args_[static_cast<int>(SubprogramFuncNames::COMMON_INIT)];

        if (get_return(call_any_args_func(subprogram.info_.funcs[static_cast<int>(SubprogramFuncNames::COMMON_INIT)], any_args)))
            return false;
    }

    return true;
}

bool SubprogramLocator::common_deinit(const ID &subprogram_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::init(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    /*
        1. проверить и получить инстанс
        2. проверить на мультиинстанцируемость
        3. проверить, что все зависимости готовы к инициализации
        4. получить порядок инстанцируемых родительских подпрограмм с READY_TO_COMMON_INITIALIZE и с READY_TO_INIT
        5. инициализировать все подпрограммы с READY_TO_COMMON_INITIALIZE до READ_TO_INIT
        6. инициализировать все подпрограммы с READY_TO_INIT до STOPPED
    */
    return false;
}

bool SubprogramLocator::deinit(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::start(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::stop(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::resume(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::pause(const ID &subprogram_name, const ID& instance_name, AnyArgs& args)
{
    return false;
}
