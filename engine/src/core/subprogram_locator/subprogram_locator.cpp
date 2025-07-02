#include "subprogram_locator.hpp"
#include "error_code.hpp"
#include <queue>
#include <set>
#include <list>

SubprogramStates SubprogramLocator::get_subprogram(
    const ID& subprogram_name, 
    Subprogram** subprogram)
{
    if (subprogram != nullptr)
        *subprogram = nullptr;
        
    SubprogramStates state = SubprogramStates::NOT_EXISTED;
        
    if (auto it = subprograms_.find(subprogram_name); it != subprograms_.end()) {
        if (subprogram != nullptr)
            *subprogram = &it->second;
        state = it->second.state_;
    }

    return state;
}

bool SubprogramLocator::add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram)
{
    ReturnOrErrorCode<int> res = dependencies_graph_.get_free_vertex_id();
    if (is_error_code(res))
        return false;

    int new_vertex_id = get_return(res);
    dependencies_graph_.add_vertex(new_vertex_id);
    vertexes_to_subprogram_names_[new_vertex_id] = info.name_;

    subprograms_[info.name_] = Subprogram{
        info,
        is_undefined_subprogram? SubprogramStates::UNDEFINED : SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT,
        new_vertex_id
    };
    
    SubprogramInfo& subprogram_info = subprograms_[info.name_].info_;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) 
        if (!subprogram_info.funcs[i]) 
            subprogram_info.funcs[i] = empty_func_;

    return true;
}

void SubprogramLocator::update_undefined_subprogram(const SubprogramInfo& info)
{
    Subprogram& dest_subprogram = subprograms_[info.name_];

    dest_subprogram.info_.dependencies_ = info.dependencies_;
    dest_subprogram.state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT; // dest_subprogram.instances_[0] = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i)
        if (info.funcs[i])
            dest_subprogram.info_.funcs[i] = info.funcs[i];
}

/* 
 * Make dependent subprograms status from "defined without defined parent" to "ready to common initialize"
 * for desired result in hierarchy of dependency subprograms all parent subprograms of child subprograms 
 * must be ready.
 */
void SubprogramLocator::try_make_dependent_subprograms_ready(const ID& ready_parent_subprogram_name)
{
    inverse_dependencies_graph_.dfs(
        subprograms_[ready_parent_subprogram_name].vertex_, 
        [&](int vertex, int, Colors, bool) {
                
        auto &current_subprogram = subprograms_[vertexes_to_subprogram_names_[vertex]];
        if (current_subprogram.state_ != SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT)
            return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;

        bool all_parent_subprograms_defined = true;
        for (auto& parent_subprogram_name : current_subprogram.info_.dependencies_) {
            auto& parent_subprogram = subprograms_[parent_subprogram_name];

            if (parent_subprogram.state_ == SubprogramStates::UNDEFINED 
             || parent_subprogram.state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
                    all_parent_subprograms_defined = false;
                    break;
            }
        }

        if (all_parent_subprograms_defined) {
            current_subprogram.state_ = SubprogramStates::READY_TO_INITIALIZE;
            return CallbackCommand::CONTINUE;
        }

        return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;
    });
}

void SubprogramLocator::restore_subprogram_to_undefined(
    Subprogram& subprogram, 
    const Subprogram& undefined_subprogram, 
    const std::unordered_set<ID>& added_undefined_subprograms)
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

bool SubprogramLocator::add(const SubprogramInfo& info)
{
    bool it_was_undefined = false;
    Subprogram undefined_subprogram;

    if (auto it = subprograms_.find(info.name_); it != subprograms_.end()) {

        if (it->second.state_ != SubprogramStates::UNDEFINED)
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

        } else if (dep_sub->second.state_ == SubprogramStates::UNDEFINED 
                || dep_sub->second.state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
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
        //subprogram.state_ = SubprogramStates::READY_TO_INITIALIZE;

        if (it_was_undefined) {
            try_make_dependent_subprograms_ready(info.name_);
        }
    }

    return true;
}


bool SubprogramLocator::remove(const SubprogramInfo& info)
{

    return false;
}

void SubprogramLocator::get_dependency_order(
    Subprogram* subprogram, 
    SubprogramStates* target_states, 
    std::size_t target_states_count,
    std::deque<Subprogram*>& dependency_order)
{
    dependencies_graph_.dfs(subprogram->vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

        auto& subprogram_node = subprograms_[vertexes_to_subprogram_names_[vertex]];

        if (is_backtracking) {
            dependency_order.push_front(&subprogram_node);
            return CallbackCommand::CONTINUE;
        }

        if (color != Colors::WHITE)
            return CallbackCommand::CONTINUE;

        bool need_skip = false;
        for (size_t i = 0; i < target_states_count; ++i) {
            if (subprogram_node.state_ == target_states[i]) {
                need_skip = true;
                break;
            }
        }

        if (need_skip)
            return CallbackCommand::SKIP_CHILDRENS;

        return CallbackCommand::CONTINUE;
    }, true);
}

bool SubprogramLocator::call_func(Subprogram &subprogram, SubprogramFuncNames handler_name, AnyArgs &args)
{
    AnyArgs empty_args;
    AnyArgs* default_args = &subprogram.info_.default_args_[static_cast<int>(handler_name)];
    AnyArgs* target_args = &empty_args;

    if (!args.empty()) 
        target_args = &args;
    else if (!default_args->empty())
        target_args = default_args;

    if (get_return(call_any_args_func(subprogram.info_.funcs[static_cast<int>(handler_name)], target_args)))
        return false;
}

bool SubprogramLocator::accept_states_by_dep_order(
    std::deque<Subprogram*> &dependency_order, 
    SubprogramStates from_state, 
    SubprogramStates to_state, 
    SubprogramFuncNames handler_name,
    AnyArgs& args)
{
    for (Subprogram* subprogram : dependency_order) {

        if (subprogram->state_ != from_state)
            continue;

        subprogram->state_ = to_state;

        if (!call_func(*subprogram, handler_name, args))
            return false;
    }
}

bool SubprogramLocator::init(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    if (get_subprogram(subprogram_name, &subprogram) != SubprogramStates::READY_TO_INITIALIZE)
        return false;

    std::deque<Subprogram*> dependency_order;
    SubprogramStates states[] = {SubprogramStates::READY_TO_INITIALIZE};
    
    get_dependency_order(subprogram, states, 1, dependency_order);
    return accept_states_by_dep_order(
        dependency_order, 
        SubprogramStates::READY_TO_INITIALIZE, 
        SubprogramStates::STOPPED, 
        SubprogramFuncNames::INIT, 
        args);
}

bool SubprogramLocator::deinit(const ID &subprogram_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::start(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state != SubprogramStates::READY_TO_INITIALIZE && state != SubprogramStates::STOPPED)
        return false;

    std::deque<Subprogram*> dependency_order;
    SubprogramStates states[] = {SubprogramStates::READY_TO_INITIALIZE, SubprogramStates::STOPPED};
    
    get_dependency_order(subprogram, states, 2, dependency_order);

    AnyArgs empty_args;
    if (accept_states_by_dep_order(
        dependency_order, 
        SubprogramStates::READY_TO_INITIALIZE, 
        SubprogramStates::STOPPED, 
        SubprogramFuncNames::INIT, 
        empty_args))
        return false;

    if (accept_states_by_dep_order(
        dependency_order, 
        SubprogramStates::STOPPED, 
        SubprogramStates::STARTED, 
        SubprogramFuncNames::START, 
        args))
        return false;

    std::list<Subprogram*> inverse_order;
    std::unordered_map<int, std::set<int>> vertexes_with_tmp_stopped_parents;
    std::unordered_map<int, std::pair<std::size_t, std::size_t>> vertexes_with_direct_tmp_stopped_parents_count;
    std::vector<Subprogram*> current_tmp_stopped_parents;

    inverse_dependencies_graph_.dfs(
        subprogram->vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

        Subprogram* cur_subprogram = &subprograms_[vertexes_to_subprogram_names_[vertex]];

        if (cur_subprogram->vertex_ == subprogram->vertex_)
            return CallbackCommand::CONTINUE;
            
        if (is_backtracking) {
            inverse_order.push_back(&subprograms_[vertexes_to_subprogram_names_[vertex]]);
            if (vertexes_with_direct_tmp_stopped_parents_count.find(vertex) != vertexes_with_direct_tmp_stopped_parents_count.end()) {
                current_tmp_stopped_parents.pop_back();
            }
            return CallbackCommand::CONTINUE;
        }

        bool is_tmp_stopped = 
            cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED
            || cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED;

        if (color == Colors::BLACK) {
            if (is_tmp_stopped) {
                vertexes_with_direct_tmp_stopped_parents_count[vertex].second++;
            }
            return CallbackCommand::CONTINUE;
        }   

        if (!is_tmp_stopped)
            return CallbackCommand::SKIP_CHILDRENS;

        if (!current_tmp_stopped_parents.empty()) {
            for (Subprogram* subprogram : current_tmp_stopped_parents) {
                vertexes_with_tmp_stopped_parents[vertex].insert(subprogram->vertex_);
            }
        }

        bool have_stopped_parents = false;
        std::size_t tmp_stopped_parent_count = 0;
        for (auto& dependency : cur_subprogram->info_.dependencies_) {
            Subprogram* parent_subprogram = &subprograms_[dependency];
            if (parent_subprogram->state_ == SubprogramStates::STOPPED) {
                have_stopped_parents = true;
                break;
            }
            if (parent_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED
                || parent_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED)
                tmp_stopped_parent_count++;
        }

        if (have_stopped_parents)
            return CallbackCommand::SKIP_CHILDRENS;

        if (tmp_stopped_parent_count > 1) {
            vertexes_with_direct_tmp_stopped_parents_count[vertex].first = tmp_stopped_parent_count;
            vertexes_with_direct_tmp_stopped_parents_count[vertex].second = 1;
            current_tmp_stopped_parents.push_back(cur_subprogram);
        }

        return CallbackCommand::CONTINUE;

    }, true);

    if (inverse_order.empty()) {
        return true;
    }

    AnyArgs empty;

    for (auto it = inverse_order.rbegin(); it != inverse_order.rend(); ++it) {
        bool need_skip = false;
        Subprogram* cur_subprogram = *it;
        auto tmp_stopped_parents = vertexes_with_tmp_stopped_parents.find(cur_subprogram->vertex_);
        if (tmp_stopped_parents != vertexes_with_tmp_stopped_parents.end()) {
            for (int parent_vertex : tmp_stopped_parents->second) {
                auto tmp_stopped_parent = vertexes_with_direct_tmp_stopped_parents_count.find(parent_vertex);
                if (tmp_stopped_parent->second.first > tmp_stopped_parent->second.second) {
                    need_skip = true;
                    break;
                }
            }
        }

        if (need_skip) {
            continue;
        }
        
        if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
            cur_subprogram->state_ = SubprogramStates::STARTED;
            call_func(*cur_subprogram, SubprogramFuncNames::START, empty);
        } else if (cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
            cur_subprogram->state_ = SubprogramStates::STARTED;
            call_func(*cur_subprogram, SubprogramFuncNames::START, empty);
            cur_subprogram->state_ = SubprogramStates::PAUSED;
            call_func(*cur_subprogram, SubprogramFuncNames::PAUSE, empty);
        }
    }

    return true;
}

bool SubprogramLocator::stop(const ID &subprogram_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::resume(const ID &subprogram_name, AnyArgs& args)
{
    return false;
}

bool SubprogramLocator::pause(const ID &subprogram_name, AnyArgs& args)
{
    return false;
}
