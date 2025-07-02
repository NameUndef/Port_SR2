#ifndef INCLUDE_SUBPROGRAM_LOCATOR_HPP_
#define INCLUDE_SUBPROGRAM_LOCATOR_HPP_

#include "any_args_func.hpp"
#include "id.hpp"
#include "graph.hpp"

#include <string>
#include <vector>
#include <unordered_map>

enum class SubprogramFuncNames {
    INIT,
    DEINIT,
    START,
    STOP,
    RESUME,
    PAUSE,
    COUNT
};

struct SubprogramInfo {
    AnyArgsFunc<bool> funcs[static_cast<int>(SubprogramFuncNames::COUNT)];
    DefaultArgs default_args_;
    std::vector<ID> dependencies_;
    ID name_;
};

using DefaultArgs = AnyArgs[static_cast<int>(SubprogramFuncNames::COUNT)];

enum class SubprogramStates {
    NOT_EXISTED,
    UNDEFINED,
    DEFINED_WITHOUT_DEFINED_PARENT,
    READY_TO_INITIALIZE,
    STOPPED,
    STARTED,
    PAUSED,
    STARTED_WHEN_PARENT_PAUSED, // [START] -> PAUSE ; [PAUSE] -> RESUME
    STARTED_WHEN_PARENT_STOPPED, // [START] -> STOP ; [STOP] -> START
    PAUSED_WHEN_PARENT_STOPPED // [PAUSE] -> STOP ; [STOP] -> START -> PAUSE
};

class SubprogramLocator {

    struct Subprogram {
        SubprogramInfo info_;
        SubprogramStates state_;
        int vertex_;
    };

    static const AnyArgsFunc<bool> empty_func_;
    std::unordered_map<ID, Subprogram> subprograms_;
    Graph dependencies_graph_, inverse_dependencies_graph_;
    std::unordered_map<int, ID> vertexes_to_subprogram_names_;
    
public:
    bool add(const SubprogramInfo& info);
    bool remove(const SubprogramInfo& info);

    bool init(const ID& subprogram_name, AnyArgs& args);
    bool deinit(const ID& subprogram_name, AnyArgs& args);
    bool start(const ID& subprogram_name, AnyArgs& args);
    bool stop(const ID& subprogram_name, AnyArgs& args);
    bool resume(const ID& subprogram_name, AnyArgs& args);
    bool pause(const ID& subprogram_name, AnyArgs& args);

    template <typename... ArgsT>
    inline bool init(const ID& subprogram_name, ArgsT&&... args)
    {
        return init(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool deinit(const ID& subprogram_name, ArgsT&&... args)
    {
        return deinit(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool start(const ID& subprogram_name, ArgsT&&... args)
    {
        return start(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool stop(const ID& subprogram_name, ArgsT&&... args)
    {
        return stop(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool resume(const ID& subprogram_name, ArgsT&&... args)
    {
        return resume(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool pause(const ID& subprogram_name, ArgsT&&... args)
    {
        return pause(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

private:
    SubprogramStates get_subprogram(
        const ID& subprogram_name, 
        Subprogram** subprogram = nullptr);

    bool add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram = true);
    void update_undefined_subprogram(const SubprogramInfo& info);
    void try_make_dependent_subprograms_ready(const ID& ready_parent_subprogram_name);
    void restore_subprogram_to_undefined(
        Subprogram& subprogram, 
        const Subprogram& undefined_subprogram, 
        const std::unordered_set<ID>& added_undefined_subprograms);
    void get_dependency_order(
        Subprogram* subprogram, 
        SubprogramStates* target_states, 
        std::size_t target_states_count,
        std::deque<Subprogram*>& dependency_order);
    bool accept_states_by_dep_order(
        std::deque<Subprogram*>& dependency_order, 
        SubprogramStates from_state, 
        SubprogramStates to_state, 
        SubprogramFuncNames handler_name,
        AnyArgs& args);
    bool call_func(Subprogram& subprogram, SubprogramFuncNames handler_name, AnyArgs& args);
};

const AnyArgsFunc<bool> SubprogramLocator::empty_func_ = [](auto...) { return true; };

#endif  // INCLUDE_SUBPROGRAM_LOCATOR_HPP_
