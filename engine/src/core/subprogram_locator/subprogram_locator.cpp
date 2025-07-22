#include "subprogram_locator.hpp"
#include "error_code.hpp"
#include <queue>
#include <set>
#include <list>

const AnyArgsFunc<bool> SubprogramLocator::empty_func_ = [](auto...) 
{ 
    return true; 
};

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

    for (std::size_t i = 0; i < FUNCS_COUNT; ++i) {
        if (!subprogram_info.funcs_[i]) {
            subprogram_info.funcs_[i] = empty_func_;
        }
    }

    return true;
}

void SubprogramLocator::update_undefined_subprogram(const SubprogramInfo& info)
{
    Subprogram& dest_subprogram = subprograms_[info.name_];

    dest_subprogram.info_.dependencies_ = info.dependencies_;
    dest_subprogram.state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;

    for (std::size_t i = 0; i < FUNCS_COUNT; ++i) {
        if (info.funcs_[i]) {
            dest_subprogram.info_.funcs_[i] = info.funcs_[i];
        }
        dest_subprogram.info_.default_args_[i] = info.default_args_[i];
    }
}

/* 
 * Make dependent subprograms status from "defined without defined parent" to "ready to common initialize"
 * for desired result in hierarchy of dependency subprograms all parent subprograms of child subprograms 
 * must be ready.
 */
void SubprogramLocator::try_make_dependent_subprograms_ready(Subprogram* subprogram)
{
    std::list<Subprogram*> order;
    SubprogramStates states[] = {SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT};
    SubprogramStates skip_states[] = {SubprogramStates::UNDEFINED};
    get_target_inverse_dependency_order(subprogram, order, states, 1, skip_states, 1, nullptr, 0);
    for (auto it = order.begin(); it != order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ = SubprogramStates::READY_TO_INITIALIZE;
    }
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

SubprogramStates SubprogramLocator::get_subprogram_state(const ID &subprogram_name)
{
    Subprogram* subprogram = nullptr;
    return get_subprogram(subprogram_name, &subprogram);
}

bool SubprogramLocator::add(const SubprogramInfo &info)
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
            add_new_subprogram(undefined_subprogram_info, true);
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
    get_dependency_order(subprogram, nullptr, 0, inverse_dependency_order, false, true);

    for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
        if (!deinit_change_states(*it, empty_args)) {
            return false;
        }

        Subprogram* cur_subprogram = *it;
        cur_subprogram->state_ = SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT;
    }

    deinit_change_states(subprogram, empty_args);
    subprogram->state_ = SubprogramStates::UNDEFINED;
    
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

    if (!inverse_dependency_order.empty()) {
        for (std::size_t i = 0; i < FUNCS_COUNT; ++i) {
            subprogram->info_.funcs_[i] = empty_func_;
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
    bool push_front,
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

            if (push_front) {
                dependency_order.push_front(&subprogram_node);
            } else {
                dependency_order.push_back(&subprogram_node);
            }

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
    AnyArgs* default_args = &subprogram.info_.default_args_[static_cast<std::size_t>(handler_name)];
    AnyArgs* target_args = &empty_args;

    if (!args.empty()) {
        target_args = &args;
    } else if (!default_args->empty()) {
        target_args = default_args;
    }

    if (!get_return(call_any_args_func(subprogram.info_.funcs_[static_cast<std::size_t>(handler_name)], *target_args))) {
        return false;
    }

    return true;
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
        || cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED)
    {
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
        && state != SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        && state != SubprogramStates::PAUSED_WHEN_PARENT_STOPPED)
        return false;

    SubprogramStates states[] = {
        SubprogramStates::STOPPED, 
        SubprogramStates::STARTED, 
        SubprogramStates::PAUSED, 
        SubprogramStates::STARTED_WHEN_PARENT_STOPPED, 
        SubprogramStates::STARTED_WHEN_PARENT_PAUSED,
        SubprogramStates::PAUSED_WHEN_PARENT_STOPPED
    };

    std::deque<Subprogram*> inverse_dependency_order;
    get_dependency_order(subprogram, states, 6, inverse_dependency_order, false, true);
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

struct IgnoredParentForSkipCountingRule {
    SubprogramStates target_state;
    SubprogramStates ignored_parent; 
};

void SubprogramLocator::get_target_inverse_dependency_order(
    Subprogram *subprogram, 
    std::list<Subprogram *> &inverse_order,
    SubprogramStates *target_states, 
    std::size_t target_states_count, 
    SubprogramStates* skip_parent_states, 
    std::size_t skip_parent_states_count,
    IgnoredParentForSkipCountingRule* ignore_parent_for_skip_counting_rule,
    std::size_t ignore_parent_for_skip_counting_rule_count)
{
    Graph tmp_graph;
    std::unordered_map<int, std::pair<std::size_t, std::unordered_map<SubprogramStates, std::size_t>>> target_parents_counts;

    inverse_dependencies_graph_.dfs(
    subprogram->vertex_, 
    [&](int vertex, int parent, Colors color, bool) {

        if (vertex == subprogram->vertex_) {
            return CallbackCommand::CONTINUE;
        }

        Subprogram* cur_subprogram = &subprograms_[vertexes_to_subprogram_names_[vertex]];
        {
            bool is_target_state = false;
            for (size_t i = 0; i < target_states_count; ++i) {
                if (cur_subprogram->state_ == target_states[i]) {
                    is_target_state = true;
                    break;
                }
            }

            if (target_states_count == 0) {
                is_target_state = true;
            }

            if (!is_target_state) {
                return CallbackCommand::SKIP_CHILDRENS;
            }
        }

        if (color != Colors::WHITE) {
            if (auto it = target_parents_counts.find(vertex); it != target_parents_counts.end()) {
                it->second.second[cur_subprogram->state_]--;
                it->second.first--;
            }
            if (tmp_graph.has_vertex(parent) && tmp_graph.has_vertex(vertex)) {
                tmp_graph.add_edge(parent, vertex);
            }
            return CallbackCommand::CONTINUE;
        }

        {
            bool skip = false;
            std::unordered_map<SubprogramStates, std::size_t> counts;
            std::size_t target_parents_count = 0;
            for (auto& parent_name : cur_subprogram->info_.dependencies_) {
                Subprogram* parent_subprogram = &subprograms_[parent_name];

                if (parent_subprogram->vertex_ == parent) {
                    continue;
                }

                for (size_t i = 0; i < skip_parent_states_count; ++i) {
                    if (parent_subprogram->state_ == skip_parent_states[i]) {
                        skip = true;
                        break;
                    }
                }

                if (skip) {
                    return CallbackCommand::SKIP_CHILDRENS;
                }

                for (size_t i = 0; i < target_states_count; ++i) {
                    if (parent_subprogram->state_ == target_states[i]) {
                        counts[parent_subprogram->state_]++;
                        target_parents_count++;
                        break;
                    }
                }
            }

            if (target_parents_count > 0) {
                target_parents_counts[vertex].second = counts;
                target_parents_counts[vertex].first = target_parents_count;
            }
        }

        tmp_graph.add_edge(parent, vertex);

        return CallbackCommand::CONTINUE;
    });

    std::unordered_set<int> skipped_vertices;
    for (auto it = target_parents_counts.begin(); it != target_parents_counts.end(); ++it) {
        if (it->second.first > 0) {

            bool ignore_skipping = false;
            if (ignore_parent_for_skip_counting_rule) {
                SubprogramStates state = subprograms_[vertexes_to_subprogram_names_[it->first]].state_;
                for (std::size_t i = 0; i < ignore_parent_for_skip_counting_rule_count; ++i) {
                    if (state == ignore_parent_for_skip_counting_rule[i].target_state) {
                        if (it->second.second[ignore_parent_for_skip_counting_rule[i].ignored_parent] == it->second.first) {
                            ignore_skipping = true;
                            break;
                        }
                    }
                }
            }

            if (ignore_skipping) {
                continue;
            }

            tmp_graph.dfs(it->first, [&](int vertex, int parent, Colors color, bool) {

                if (color != Colors::WHITE) {
                    return CallbackCommand::CONTINUE;
                }

                skipped_vertices.insert(vertex);
                return CallbackCommand::CONTINUE;
            });
        }
    }

    tmp_graph.dfs(
        subprogram->vertex_, 
        [&](int vertex, int parent, Colors color, bool is_backtracking) {

            if (is_backtracking) {
                if (vertex != subprogram->vertex_) {
                    inverse_order.push_back(&subprograms_[vertexes_to_subprogram_names_[vertex]]);
                }
                return CallbackCommand::CONTINUE;
            }

            if (color != Colors::WHITE) {
                return CallbackCommand::CONTINUE;
            }

            if (skipped_vertices.find(vertex) != skipped_vertices.end()) {
                return CallbackCommand::SKIP_CHILDRENS;
            }
            
            return CallbackCommand::CONTINUE;
        }, true);
}

bool SubprogramLocator::start_change_state(Subprogram* cur_subprogram, AnyArgs& init_args, AnyArgs& start_args)
{
    if (cur_subprogram->state_ == SubprogramStates::READY_TO_INITIALIZE) {
        cur_subprogram->state_ = SubprogramStates::STOPPED;

        if (!call_func(*cur_subprogram, SubprogramFuncNames::INIT, init_args)) {
            return false;
        }
    }

    if (cur_subprogram->state_ == SubprogramStates::STOPPED 
        || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        || cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
        
        cur_subprogram->state_ = SubprogramStates::STARTED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::START, start_args)) {
            return false;
        }

    } else if (cur_subprogram->state_ == SubprogramStates::PAUSED 
        || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {

        cur_subprogram->state_ = SubprogramStates::STARTED;
        if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, start_args)) {
            return false;
        }
    }

    return true;
}

bool SubprogramLocator::start(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state != SubprogramStates::READY_TO_INITIALIZE 
        && state != SubprogramStates::STOPPED 
        && state != SubprogramStates::STARTED_WHEN_PARENT_STOPPED
        && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        && state != SubprogramStates::PAUSED
        && state != SubprogramStates::PAUSED_WHEN_PARENT_STOPPED)
        return false;

    std::deque<Subprogram*> dependency_order;
    SubprogramStates states[] = {
        SubprogramStates::READY_TO_INITIALIZE, 
        SubprogramStates::STOPPED, 
        SubprogramStates::STARTED_WHEN_PARENT_STOPPED,
        SubprogramStates::STARTED_WHEN_PARENT_PAUSED,
        SubprogramStates::PAUSED,
        SubprogramStates::PAUSED_WHEN_PARENT_STOPPED
    };
    
    get_dependency_order(subprogram, states, 6, dependency_order);

    AnyArgs empty_args;

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        if (!start_change_state(cur_subprogram, empty_args, empty_args)) {
            return false;
        }
    }

    if (!start_change_state(subprogram, empty_args, args)) {
        return false;
    }

    dependency_order.push_back(subprogram);
    std::list<Subprogram*> inverse_dependency_order;

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        inverse_dependency_order.clear();
        SubprogramStates target_states[] = {
            SubprogramStates::STARTED_WHEN_PARENT_STOPPED, 
            SubprogramStates::STARTED_WHEN_PARENT_PAUSED,
            SubprogramStates::PAUSED_WHEN_PARENT_STOPPED
        };
        SubprogramStates skip_states[] = {
            SubprogramStates::STOPPED
        };

        IgnoredParentForSkipCountingRule ignore_skip_counting_parent[2];
        ignore_skip_counting_parent[0].target_state = SubprogramStates::STARTED_WHEN_PARENT_STOPPED;
        ignore_skip_counting_parent[0].ignored_parent = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;
        ignore_skip_counting_parent[1].target_state = SubprogramStates::PAUSED_WHEN_PARENT_STOPPED;
        ignore_skip_counting_parent[1].ignored_parent = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;

        get_target_inverse_dependency_order(
            cur_subprogram, 
            inverse_dependency_order, 
            target_states, 
            3, 
            skip_states, 
            1, 
            ignore_skip_counting_parent,
            2);

        for (auto it = inverse_dependency_order.rbegin(); it != inverse_dependency_order.rend(); it++) {
            Subprogram* cur_subprogram = *it;

            bool have_pause = false, 
                have_started_when_parent_paused = false, 
                have_started_when_parent_stopped = false,
                have_paused_when_parent_stopped = false;

            for (ID& parent_name : cur_subprogram->info_.dependencies_) {
                Subprogram& parent = subprograms_[parent_name];
                if (parent.state_ == SubprogramStates::PAUSED) {
                    have_pause = true;
                } else if (parent.state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
                    have_started_when_parent_paused = true;
                } else if (parent.state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
                    have_started_when_parent_stopped = true;
                } else if (parent.state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
                    have_paused_when_parent_stopped = true;
                }
            }

            if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
                if (!have_started_when_parent_stopped) {
                    if (have_pause || have_started_when_parent_paused) {
                        cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;
                        if (!call_func(*cur_subprogram, SubprogramFuncNames::START_AS_PAUSED, empty_args)) {
                            return false;
                        }
                    } else {
                        cur_subprogram->state_ = SubprogramStates::STARTED;
                        if (!call_func(*cur_subprogram, SubprogramFuncNames::START, empty_args)) {
                            return false;
                        }
                    }    
                }
            } else if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
                if (!have_pause && !have_started_when_parent_paused) {
                    cur_subprogram->state_ = SubprogramStates::STARTED;
                    if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, empty_args)) {
                        return false;
                    }
                }
            } else if (cur_subprogram->state_ == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED) {
                if (!have_started_when_parent_stopped && !have_paused_when_parent_stopped) {
                    cur_subprogram->state_ = SubprogramStates::PAUSED;
                    if (!call_func(*cur_subprogram, SubprogramFuncNames::START_AS_PAUSED, empty_args)) {
                            return false;
                    }
                }
            }
        }
    }

    return true;
}

bool SubprogramLocator::start_as_paused(const ID &subprogram_name, AnyArgs &args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
    if (state != SubprogramStates::READY_TO_INITIALIZE && state != SubprogramStates::STOPPED) {
        return false;
    }

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

        if (cur_subprogram->state_ == SubprogramStates::STOPPED) {
            cur_subprogram->state_ = SubprogramStates::PAUSED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::START_AS_PAUSED, empty_args)) {
                return false;
            }
        }
    }

    if (subprogram->state_ == SubprogramStates::READY_TO_INITIALIZE) {
        subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*subprogram, SubprogramFuncNames::INIT, empty_args)) {
            return false;
        }
    }

    if (subprogram->state_ == SubprogramStates::STOPPED) {
        subprogram->state_ = SubprogramStates::PAUSED;
        if (!call_func(*subprogram, SubprogramFuncNames::START_AS_PAUSED, args)) {
            return false;
        }
    }

    dependency_order.push_back(subprogram);
    std::list<Subprogram*> inverse_dependency_order;

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        inverse_dependency_order.clear();
        SubprogramStates target_states[] = {
            SubprogramStates::STARTED_WHEN_PARENT_STOPPED, 
            SubprogramStates::PAUSED_WHEN_PARENT_STOPPED
        };
        SubprogramStates skip_parent_states[] = {
            SubprogramStates::STOPPED
        };
        get_target_inverse_dependency_order(*it, inverse_dependency_order, target_states, 2, skip_parent_states, 1, nullptr, 0);

        for (auto it = inverse_dependency_order.rbegin(); it != inverse_dependency_order.rend(); it++) {
            Subprogram* cur_subprogram = *it;
            if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
                cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;    
            } else {
                cur_subprogram->state_ = SubprogramStates::PAUSED;
            }
            if (!call_func(*cur_subprogram, SubprogramFuncNames::START_AS_PAUSED, empty_args)) {
                return false;
            }
        }
    }

    return true;
}

bool SubprogramLocator::stop(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);

    if (state == SubprogramStates::STARTED) { 

        AnyArgs empty_args;

        SubprogramStates target_states[] = {
            SubprogramStates::STARTED, 
            SubprogramStates::PAUSED,
            SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        };
        std::deque<Subprogram*> inverse_dependency_order;
        get_dependency_order(subprogram, target_states, 3, inverse_dependency_order, false, true);

        for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
            Subprogram* cur_subprogram = *it;
            if (cur_subprogram->state_ == SubprogramStates::STARTED 
                || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {

                cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_STOPPED;
                if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, empty_args)) {
                    return false;

                }
            } else if (cur_subprogram->state_ == SubprogramStates::PAUSED) {

                cur_subprogram->state_ = SubprogramStates::PAUSED_WHEN_PARENT_STOPPED;
                if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, empty_args)) {
                    return false;
                }
            }
        }

        subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*subprogram, SubprogramFuncNames::STOP, args)) {
            return false;
        }
        return true;

    } else if (state == SubprogramStates::PAUSED || state == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
        AnyArgs empty_args;

        SubprogramStates target_states[] = {
            SubprogramStates::PAUSED,
            SubprogramStates::STARTED_WHEN_PARENT_PAUSED
        };
        std::deque<Subprogram*> inverse_dependency_order;

        get_dependency_order(subprogram, target_states, 2, inverse_dependency_order, false, true);
        for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
            Subprogram* cur_subprogram = *it;
            if (cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
                cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_STOPPED;

                if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, empty_args)) {
                    return false;
                }
            } else if (cur_subprogram->state_ == SubprogramStates::PAUSED) {

                cur_subprogram->state_ = SubprogramStates::PAUSED_WHEN_PARENT_STOPPED;
                if (!call_func(*cur_subprogram, SubprogramFuncNames::STOP, empty_args)) {
                    return false;
                }
            }
        }

        subprogram->state_ = SubprogramStates::STOPPED;
        if (!call_func(*subprogram, SubprogramFuncNames::STOP, args)) {
            return false;
        }

        return true;
    }
    else if (state == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED || state == SubprogramStates::STARTED_WHEN_PARENT_STOPPED) {
        subprogram->state_ = SubprogramStates::STOPPED;
        return true;
    }

    return false;
}

bool SubprogramLocator::resume(const ID &subprogram_name, AnyArgs& args)
{
   Subprogram* subprogram = nullptr;
   SubprogramStates state = get_subprogram(subprogram_name, &subprogram);
   if (state != SubprogramStates::PAUSED && state != SubprogramStates::STARTED_WHEN_PARENT_PAUSED)
       return false;

    std::deque<Subprogram*> dependency_order;
    SubprogramStates states[] = {SubprogramStates::PAUSED, SubprogramStates::STARTED_WHEN_PARENT_PAUSED};
    get_dependency_order(subprogram, states, 2, dependency_order);

    AnyArgs empty_args;
    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        if (cur_subprogram->state_ == SubprogramStates::PAUSED 
            || cur_subprogram->state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {

            cur_subprogram->state_ = SubprogramStates::STARTED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, empty_args)) {
                return false;
            }
        }
    }

    subprogram->state_ = SubprogramStates::STARTED;
    if (!call_func(*subprogram, SubprogramFuncNames::RESUME, args)) {
        return false;
    }

    dependency_order.push_back(subprogram);
    std::list<Subprogram*> inverse_dependency_order;

    for (auto it = dependency_order.begin(); it != dependency_order.end(); it++) {
        Subprogram* cur_subprogram = *it;
        SubprogramStates target_states[] = {SubprogramStates::STARTED_WHEN_PARENT_PAUSED};
        inverse_dependency_order.clear();
        get_target_inverse_dependency_order(cur_subprogram, inverse_dependency_order, target_states, 1, nullptr, 0, nullptr, 0);

        for (auto it = inverse_dependency_order.rbegin(); it != inverse_dependency_order.rend(); it++) {

            Subprogram* cur_subprogram = *it;
            bool have_pause = false, 
                 have_started_when_parent_paused = false;

            for (ID& parent_name : cur_subprogram->info_.dependencies_) {
                Subprogram& parent = subprograms_[parent_name];
                if (parent.state_ == SubprogramStates::PAUSED) {
                    have_pause = true;
                } else if (parent.state_ == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
                    have_started_when_parent_paused = true;
                }
            }

            if (!have_pause && !have_started_when_parent_paused) {
                cur_subprogram->state_ = SubprogramStates::STARTED;
                if (!call_func(*cur_subprogram, SubprogramFuncNames::RESUME, empty_args)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool SubprogramLocator::pause(const ID &subprogram_name, AnyArgs& args)
{
    Subprogram* subprogram = nullptr;
    SubprogramStates state = get_subprogram(subprogram_name, &subprogram);

    if (state == SubprogramStates::STARTED) {
        AnyArgs empty_args;
        std::deque<Subprogram*> inverse_dependency_order;
        SubprogramStates states[] = {SubprogramStates::STARTED};
        get_dependency_order(subprogram, states, 1, inverse_dependency_order, false, true);

        for (auto it = inverse_dependency_order.begin(); it != inverse_dependency_order.end(); it++) {
            Subprogram* cur_subprogram = *it;
            cur_subprogram->state_ = SubprogramStates::STARTED_WHEN_PARENT_PAUSED;
            if (!call_func(*cur_subprogram, SubprogramFuncNames::PAUSE, empty_args)) {
                return false;
            }
        }

        subprogram->state_ = SubprogramStates::PAUSED;
        if (!call_func(*subprogram, SubprogramFuncNames::PAUSE, args)) {
            return false;
        }

        return true;

    } else if (state == SubprogramStates::STARTED_WHEN_PARENT_PAUSED) {
        subprogram->state_ = SubprogramStates::PAUSED;
        return true;
    }

    return false;
}
