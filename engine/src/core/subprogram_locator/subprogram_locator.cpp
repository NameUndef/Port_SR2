#include "subprogram_locator.hpp"
#include "error_code.hpp"

bool SubprogramLocator::add_new_subprogram(const SubprogramInfo &info, bool is_undefined_subprogram)
{
    ReturnOrErrorCode<int> res = dependencies_graph_.get_free_vertex_id();
    if (is_error_code(res))
        return false;

    int new_vertex_id = get_return(res);
    dependencies_graph_.add_vertex(new_vertex_id);
    vertexes_to_subprogram_names_[new_vertex_id] = info.name_;

    subprograms_[info.name_] = SubprogramCommon{
        info,
        {},
        is_undefined_subprogram? SubprogramStates::UNDEFINED : SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT,
        new_vertex_id
    };
    

    SubprogramInfo& subprogram_info = subprograms_[info.name_].info_;

    if (!subprogram_info.common_init_)
        subprogram_info.common_init_ = empty_func_;

    if (!subprogram_info.common_deinit_)
        subprogram_info.common_deinit_ = empty_func_;

    if (!subprogram_info.init_)
        subprogram_info.init_ = empty_func_;

    if (!subprogram_info.deinit_)
        subprogram_info.deinit_ = empty_func_;

    if (!subprogram_info.start_)
        subprogram_info.start_ = empty_func_;

    if (!subprogram_info.stop_)
        subprogram_info.stop_ = empty_func_;

    if (!subprogram_info.resume_)
        subprogram_info.resume_ = empty_func_;

    if (!subprogram_info.pause_)
        subprogram_info.pause_ = empty_func_;   

    return true;
}

void SubprogramLocator::update_undefined_subprogram(const SubprogramInfo &info)
{
    SubprogramCommon& dest_subprogram = subprograms_[info.name_];

    dest_subprogram.info_.dependencies_ = info.dependencies_;
    dest_subprogram.info_.is_multiinstance_supported_ = info.is_multiinstance_supported_;
    dest_subprogram.state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;

    if (info.common_init_)
        dest_subprogram.info_.common_init_ = info.common_init_;

    if (info.common_deinit_)
        dest_subprogram.info_.common_deinit_ = info.common_deinit_;

    if (info.init_)
        dest_subprogram.info_.init_ = info.init_;

    if (info.deinit_)
        dest_subprogram.info_.deinit_ = info.deinit_;

    if (info.start_)
        dest_subprogram.info_.start_ = info.start_;

    if (info.stop_)
        dest_subprogram.info_.stop_ = info.stop_;

    if (info.resume_)
        dest_subprogram.info_.resume_ = info.resume_;

    if (info.pause_)
        dest_subprogram.info_.pause_ = info.pause_;
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
        if (current_subprogram.state_ != SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT)
            return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;

        bool all_parent_subprograms_defined = true;
        for (auto& parent_subprogram_name : current_subprogram.info_.dependencies_) {
            auto& parent_subprogram = subprograms_[parent_subprogram_name];

            if (
                parent_subprogram.state_ == SubprogramStates::UNDEFINED 
                || parent_subprogram.state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
                    all_parent_subprograms_defined = false;
                    break;
            }
        }

        if (all_parent_subprograms_defined) {
            current_subprogram.state_ = SubprogramStates::READY_TO_COMMON_INITIALIZE;
            return CallbackCommand::CONTINUE;
        }

        return CallbackCommand::SKIP_CHILDRENS_AND_NEXT_NEIGHBORS;
    });
}

void SubprogramLocator::restore_subprogram_to_undefined(SubprogramCommon &subprogram, const SubprogramCommon &undefined_subprogram, const std::unordered_set<ID> &added_undefined_subprograms)
{
        for (const ID& dependency_name : subprogram.info_.dependencies_) {
            SubprogramCommon& dependency  = subprograms_[dependency_name];
            
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
    SubprogramCommon undefined_subprogram;

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
    SubprogramCommon& subprogram = subprograms_[info.name_];

    for (const auto& dependency : info.dependencies_) {

        if (auto dep_sub = subprograms_.find(dependency); dep_sub == subprograms_.end()) {

            SubprogramInfo undefined_subprogram_info;
            undefined_subprogram_info.name_ = dependency;
            add_new_subprogram(undefined_subprogram_info, false);
            added_undefined_subprograms.insert(dependency);
            all_parent_subprograms_defined = false;

        } else if (
            dep_sub->second.state_ == SubprogramStates::UNDEFINED 
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
        subprogram.state_ = SubprogramStates::READY_TO_COMMON_INITIALIZE;

        if (it_was_undefined)
            try_make_dependent_subprograms_ready(info.name_);
    }

    return true;
}


bool SubprogramLocator::remove(const SubprogramInfo &info)
{

    return false;
}

bool SubprogramLocator::common_init(const ID &subprogram_name)
{

    return false;
}

bool SubprogramLocator::common_deinit(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::init(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::deinit(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::start(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::stop(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::resume(const ID &subprogram_name)
{
    return false;
}

bool SubprogramLocator::pause(const ID &subprogram_name)
{
    return false;
}

SubprogramStates SubprogramLocator::get_subprogram_state(const ID &subprogram_name) const
{
    if (auto it = subprograms_.find(subprogram_name); it != subprograms_.end())
        return it->second.state_;
    else
        return SubprogramStates::NOT_EXISTED;
}
