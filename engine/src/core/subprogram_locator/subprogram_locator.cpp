#include "subprogram_locator.hpp"
#include "error_code.hpp"
#include <queue>
#include <set>
#include <list>

SubprogramStates SubprogramLocator::get_subprogram(
    const ID& subprogram_name, 
    Subprogram** subprogram)
{
    if (subprogram != nullptr) {
        *subprogram = nullptr;
    }
        
    SubprogramStates state = SubprogramStates::NOT_EXISTED;
        
    if (auto it = subprograms_.find(subprogram_name); it != subprograms_.end()) {
        if (subprogram != nullptr) {
            *subprogram = &it->second;
        }
        state = it->second.state_;
    }

    return state;
}

bool SubprogramLocator::add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram)
{
    ReturnOrErrorCode<int> res = dependencies_graph_.get_free_vertex_id();
    if (is_error_code(res)) {
        return false;
    }

    int new_vertex_id = get_return(res);
    dependencies_graph_.add_vertex(new_vertex_id);
    vertexes_to_subprogram_names_[new_vertex_id] = info.name_;

    subprograms_[info.name_] = Subprogram{
        info,
        is_undefined_subprogram? SubprogramStates::UNDEFINED : SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT,
        new_vertex_id
    };
    
    SubprogramInfo& subprogram_info = subprograms_[info.name_].info_;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) {
        if (!subprogram_info.funcs[i]) {
            subprogram_info.funcs[i] = empty_func_;
        }
    }

    return true;
}

void SubprogramLocator::update_undefined_subprogram(const SubprogramInfo& info)
{
    Subprogram& dest_subprogram = subprograms_[info.name_];

    dest_subprogram.info_.dependencies_ = info.dependencies_;
    dest_subprogram.state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;

    for (int i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) {
        if (info.funcs[i]) {
            dest_subprogram.info_.funcs[i] = info.funcs[i];
        }
    }
}

/* 
 * Make dependent subprograms status from "defined without defined parent" to "ready to common initialize"
 * for desired result in hierarchy of dependency subprograms all parent subprograms of child subprograms 
 * must be ready.
 */
void SubprogramLocator::try_make_dependent_subprograms_ready(Subprogram* subprogram)
{
    inverse_dependencies_graph_.dfs(
        subprogram->vertex_, 
        [&](int vertex, int, Colors, bool) {

        if (vertex == subprogram->vertex_) {
            return CallbackCommand::CONTINUE;
        }
                
        auto &cur_subprogram = subprograms_[vertexes_to_subprogram_names_[vertex]];

        bool all_parent_subprograms_defined = true;
        for (auto& parent_subprogram_name : cur_subprogram.info_.dependencies_) {
            auto& parent_subprogram = subprograms_[parent_subprogram_name];

            if (parent_subprogram.state_ == SubprogramStates::UNDEFINED 
             || parent_subprogram.state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT) {
                    all_parent_subprograms_defined = false;
                    break;
            }
        }

        if (all_parent_subprograms_defined) {
            cur_subprogram.state_ = SubprogramStates::READY_TO_INITIALIZE;
            return CallbackCommand::CONTINUE;
        }

        return CallbackCommand::SKIP_CHILDRENS;
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

        if (it->second.state_ != SubprogramStates::UNDEFINED) {
            return false;
        }

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
        subprogram.state_ = SubprogramStates::READY_TO_INITIALIZE;

        if (it_was_undefined) {
            try_make_dependent_subprograms_ready(&subprogram);
        }
    }

    return true;
}

bool SubprogramLocator::remove(const ID& subprogram_name)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state == SubprogramStates::UNDEFINED || state == SubprogramStates::NOT_EXISTED) {
        return false;
    }

    std::deque<Subprogram*> inverse_dependency_order;
    AnyArgs empty_args;
    get_dependency_order(subprogram, nullptr, 0, inverse_dependency_order);

    for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
        if (!deinit_change_states(*it, empty_args)) {
            return false;
        }

        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;
    }

    deinit_change_states(subprogram, empty_args);
    subprogram->state_ == SubprogramStates::UNDEFINED;
    
    for (auto it = subprogram->info_.dependencies_.begin(); it != subprogram->info_.dependencies_.end(); it++) {
        Subprogram* parent = &subprograms_[*it];
        inverse_dependencies_graph_.remove_edge(parent->vertex_, subprogram->vertex_);
        dependencies_graph_.remove_edge(subprogram->vertex_, parent->vertex_);
        if (parent->state_ == SubprogramStates::UNDEFINED 
            && inverse_dependencies_graph_.get_edge_count(parent->vertex_) == 0) {
            
            dependencies_graph_.remove_vertex(parent->vertex_);
            inverse_dependencies_graph_.remove_vertex(parent->vertex_);
            vertexes_to_subprogram_names_.erase(parent->vertex_);
            subprograms_.erase(*it);
        }
    }

    if (inverse_dependency_order.empty()) {
        for (std::size_t i = 0; i < static_cast<int>(SubprogramFuncNames::COUNT); ++i) {
            subprogram->info_.funcs[i] = empty_func_;
            subprogram->info_.default_args_[i].clear();
        }
        subprogram->info_.dependencies_.clear();
    } else {
        dependencies_graph_.remove_vertex(subprogram->vertex_);
        inverse_dependencies_graph_.remove_vertex(subprogram->vertex_);
        vertexes_to_subprogram_names_.erase(subprogram->vertex_);
        subprograms_.erase(subprogram->info_.name_);
    }

    return true;
}

void SubprogramLocator::get_dependency_order(
    Subprogram* subprogram, 
    SubprogramStates* target_states, 
    std::size_t target_states_count,
    std::deque<Subprogram*>& dependency_order,
    bool inverse_dependencies_graph)
{
    auto& dependencies_graph = inverse_dependencies_graph ? inverse_dependencies_graph_ : dependencies_graph_;

    dependencies_graph.dfs(subprogram->vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

        auto& subprogram_node = subprograms_[vertexes_to_subprogram_names_[vertex]];

        if (is_backtracking) {

            if (subprogram_node.vertex_ == subprogram->vertex_) {
                return CallbackCommand::CONTINUE;
            }

            dependency_order.push_front(&subprogram_node);
            return CallbackCommand::CONTINUE;
        }

        if (color != Colors::WHITE) {
            return CallbackCommand::CONTINUE;
        }

        bool need_skip = true;
        for (size_t i = 0; i < target_states_count; ++i) {
            if (subprogram_node.state_ == target_states[i]) {
                need_skip = false;
                break;
            }
        }
        if (target_states_count == 0) {
            need_skip = false;
        }

        if (need_skip) {
            return CallbackCommand::SKIP_CHILDRENS;
        }

        return CallbackCommand::CONTINUE;
    }, true);
}

bool SubprogramLocator::call_func(Subprogram &subprogram, SubprogramFuncNames handler_name, AnyArgs &args)
{
    AnyArgs empty_args;
    AnyArgs* default_args = &subprogram.info_.default_args_[static_cast<int>(handler_name)];
    AnyArgs* target_args = &empty_args;

    if (!args.empty()) {
        target_args = &args;
    } else if (!default_args->empty()) {
        target_args = default_args;
    }

    if (get_return(call_any_args_func(subprogram.info_.funcs[static_cast<int>(handler_name)], target_args))) {
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

    AnyArgs empty_args;
    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        
        cur_subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::INIT, empty_args)) {
            return false;
        }
    }

    subprogram->state_ = SubprogramStates::STOPPED;
    if (!call_func(*subprogram, SubprogramFuncNames::INIT, args)) {
        return false;
    }

    return true;
}

bool SubprogramLocator::deinit_change_states(Subprogram* cur_subprogram, AnyArgs& cur_args) 
{
    if (cur_subprogram->state_ == SubprogramStates::STARTED
        || cur_subprogram->state_ == SubprogramStates::PAUSED
        || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
        cur_subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, cur_args)) {
            return false;
        }
    } else if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        || cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
        cur_subprogram->state_ = SubprogramStates::STOPPED;
    }

    if (cur_subprogram->state_ == SubprogramStates::STOPPED) {
        cur_subprogram->state_ = SubprogramStates::READY_TO_INITIALIZE;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::DEINIT, cur_args)) {
            return false;
        }
    }

    return true;
}

bool SubprogramLocator::deinit(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    
    if (state != SubprogramStates::STOPPED 
        && state != SubprogramStates::STARTED 
        && state != SubprogramStates::PAUSED
        && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        && state != SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        && state != SubprogramStates::PAUSED_WHEN_PARENT_STOPPED)
        return false;

    SubprogramStates states[] = {
        SubprogramStates::STOPPED, 
        SubprogramStates::STARTED, 
        SubprogramStates::PAUSED, 
        SubprogramStates::STARTED_WHEN_PARENT_PAUSED, 
        SubprogramStates::STARTED_WHEN_PARENT_STOPPED, 
        SubprogramStates::PAUSED_WHEN_PARENT_STOPPED
    };

    std::deque<Subprogram*> inverse_dependency_order;
    get_dependency_order(subprogram, states, 6, inverse_dependency_order, true);
    AnyArgs empty_args;

    for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
        if (!deinit_change_states(*it, empty_args)) {
            return false;
        }
    }

    if (!deinit_change_states(subprogram, args)) {
        return false;
    }

    return true;
}

void SubprogramLocator::get_potential_tmp_inverse_dependency_order(
    Subprogram *subprogram, 
    SubprogramStates parent_state_skip,
    SubprogramStates *tmp_target_states, 
    std::size_t tmp_target_states_count, 
    std::list<Subprogram *> &inverse_order)
{
    std::unordered_map<int, std::set<int>> vertexes_with_tmp_state_parents;
    std::unordered_map<int, std::pair<std::size_t, std::size_t>> vertexes_with_direct_tmp_state_parents_count;
    std::vector<Subprogram*> current_tmp_state_parents;

    inverse_dependencies_graph_.dfs(
        subprogram->vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

        Subprogram* cur_subprogram = &subprograms_[vertexes_to_subprogram_names_[vertex]];

        if (cur_subprogram->vertex_ == subprogram->vertex_) {
            return CallbackCommand::CONTINUE;
        }
            
        if (is_backtracking) {
            inverse_order.push_back(&subprograms_[vertexes_to_subprogram_names_[vertex]]);
            if (vertexes_with_direct_tmp_state_parents_count.find(vertex) != vertexes_with_direct_tmp_state_parents_count.end()) {
                current_tmp_state_parents.pop_back();
            }
            return CallbackCommand::CONTINUE;
        }

        bool is_tmp_state = false;
        for (size_t i = 0; i < tmp_target_states_count; ++i) {
            if (cur_subprogram->state_ == tmp_target_states[i]) {
                is_tmp_state = true;
                break;
            }
        }

        if (color == Colors::BLACK) {
            if (is_tmp_state) {
                vertexes_with_direct_tmp_state_parents_count[vertex].second++;
            }
            return CallbackCommand::CONTINUE;
        }   

        if (!is_tmp_state)
            return CallbackCommand::SKIP_CHILDRENS;

        if (!current_tmp_state_parents.empty()) {
            for (Subprogram* subprogram : current_tmp_state_parents) {
                vertexes_with_tmp_state_parents[vertex].insert(subprogram->vertex_);
            }
        }

        bool have_state_skip_parents = false;
        std::size_t tmp_state_parent_count = 0;
        for (auto& dependency : cur_subprogram->info_.dependencies_) {
            Subprogram* parent_subprogram = &subprograms_[dependency];
            if (parent_subprogram->state_ == parent_state_skip) {
                have_state_skip_parents = true;
                break;
            }

            for (size_t i = 0; i < tmp_target_states_count; ++i) {
                if (parent_subprogram->state_ == tmp_target_states[i]) {
                    tmp_state_parent_count++;
                    break;
                }
            }
        }

        if (have_state_skip_parents) {
            return CallbackCommand::SKIP_CHILDRENS;
        }

        if (tmp_state_parent_count > 1) {
            vertexes_with_direct_tmp_state_parents_count[vertex].first = tmp_state_parent_count;
            vertexes_with_direct_tmp_state_parents_count[vertex].second = 1;
            current_tmp_state_parents.push_back(cur_subprogram);
        }

        return CallbackCommand::CONTINUE;

    }, true);

    for (auto it = inverse_order.begin(); it != inverse_order.end();) {
        bool need_skip = false;
        Subprogram* cur_subprogram = *it;
        auto tmp_state_parents = vertexes_with_tmp_state_parents.find(cur_subprogram->vertex_);
        if (tmp_state_parents != vertexes_with_tmp_state_parents.end()) {
            for (int parent_vertex : tmp_state_parents->second) {
                auto tmp_stopped_parent = vertexes_with_direct_tmp_state_parents_count.find(parent_vertex);
                if (tmp_stopped_parent->second.first > tmp_stopped_parent->second.second) {
                    need_skip = true;
                    break;
                }
            }
        }

        if (need_skip) {
            it = inverse_order.erase(it);
        } else {
            it++;
        }
    }
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

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;

        if (cur_subprogram->state_ == SubprogramStates::READY_TO_INITIALIZE) {
            cur_subprogram->state_ = SubprogramStates::STOPPED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::INIT, empty_args)) {
                return false;
            }
        }
        cur_subprogram->state_ = SubprogramStates::STARTED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::START, empty_args)) {
            return false;
        }
    }

    if (state == SubprogramStates::READY_TO_INITIALIZE) {
        subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*subprogram, SubprogramFuncNames::INIT, empty_args)) {
            return false;
        }
    }

    subprogram->state_ = SubprogramStates::STARTED;
    if (!call_func(*subprogram, SubprogramFuncNames::START, args)) {
        return false;
    }

    std::list<Subprogram*> inverse_order, pause_order_copy;
    AnyArgs empty;
    SubprogramStates tmp_states[] = {SubprogramStates::STARTED_WHEN_PARENT_STOPPED, SubprogramStates::PAUSED_WHEN_PARENT_STOPPED};

    get_potential_tmp_inverse_dependency_order(subprogram, SubprogramStates::STOPPED, tmp_states, 2, inverse_order);

    for (auto it = inverse_order.rbegin(); it != inverse_order.rend(); it++) {
        Subprogram* cur_subprogram = *it;

        if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
            cur_subprogram->state_ = SubprogramStates::STARTED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::START, empty)) {
                return false;
            }
        } else if (cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
            cur_subprogram->state_ = SubprogramStates::STARTED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::START, empty)) {
                return false;
            }
            pause_order_copy.push_front(cur_subprogram);
        }
    }

    for (auto it = pause_order_copy.begin(); it != pause_order_copy.end(); it++) {
        Subprogram* cur_subprogram = *it;
         
        cur_subprogram->state_ = SubprogramStates::PAUSED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::PAUSE, empty)) {
            return false;
        }
    }

    return true;
}

bool SubprogramLocator::stop(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state != SubprogramStates::STARTED
        && state != SubprogramStates::PAUSED
        && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        && state != SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        && state != SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
        return false;
    }

    if (state == SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        || state == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
        state = SubprogramStates::STOPPED;
        return true;
    }

    std::deque<Subprogram*> inverse_dependency;
    SubprogramStates states[] = {SubprogramStates::STARTED, SubprogramStates::PAUSED, SubprogramStates::STARTED_WHEN_PARENT_PAUSED};
    
    get_dependency_order(subprogram, states, 3, inverse_dependency, true);

    for (auto it = inverse_dependency.rbegin(); it != inverse_dependency.rend(); it++) {
        Subprogram* cur_subprogram = *it;
        if (cur_subprogram->state_ == SubprogramStates::STARTED 
            || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
            cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_STOPPED;
        } else if (cur_subprogram->state_ == SubprogramStates::PAUSED) {
            cur_subprogram->state_ = SubprogramStates::PAUSED_WHEN_PARENT_STOPPED;
        }
        if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, args)) {
            return false;
        }
    }

    subprogram->state_ = SubprogramStates::STOPPED;
    if (!call_func(*subprogram, SubprogramFuncNames::STOP, args)) {
        return false;
    }
    
    return true;
}

bool SubprogramLocator::resume(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    if (get_subprogram(subprogram_name, &subprogram) != SubprogramStates::PAUSED) {
        return false;
    }
    
    std::deque<Subprogram*> dependency_order;
    SubprogramStates states[] = {SubprogramStates::PAUSED};
    
    get_dependency_order(subprogram, states, 1, dependency_order);

    AnyArgs empty_args;

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ = SubprogramStates::STARTED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, empty_args)) {
            return false;
        }
    }

    if (!call_func(*subprogram, SubprogramFuncNames::RESUME, args)) {
        return false;
    }

    std::list<Subprogram*> inverse_order;
    SubprogramStates tmp_states[] = {SubprogramStates::STARTED_WHEN_PARENT_PAUSED};
    get_potential_tmp_inverse_dependency_order(subprogram, SubprogramStates::PAUSED, tmp_states, 1, inverse_order);

    for (auto it = inverse_order.rbegin(); it != inverse_order.rend(); it++) {
        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ = SubprogramStates::STARTED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, empty_args)) {
            return false;
        }
    }

    return true;
}

bool SubprogramLocator::pause(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state != SubprogramStates::STARTED && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
        return false;
    }

    if (state == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
        subprogram->state_ = SubprogramStates::PAUSED;
        return true;
    }

    SubprogramStates target_states[] = {SubprogramStates::STARTED};
    std::deque<Subprogram*> inverse_dependency_order;
    get_dependency_order(subprogram, target_states, 1, inverse_dependency_order, true);

    for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::PAUSE, args)) {
            return false;
        }
    }

    subprogram->state_ = SubprogramStates::PAUSED;
    if (!call_func(*subprogram, SubprogramFuncNames::PAUSE, args)) {
        return false;
    }

    return true;
}
