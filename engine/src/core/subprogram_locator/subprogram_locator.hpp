#ifndef INCLUDE_SUBPROGRAM_LOCATOR_HPP_
#define INCLUDE_SUBPROGRAM_LOCATOR_HPP_

#include "any_args_func.hpp"
#include "id.hpp"
#include "graph.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <queue>

enum class SubprogramFuncNames {
    INIT,
    DEINIT,
    START,
    START_AS_PAUSED,
    STOP,
    RESUME,
    PAUSE
};

constexpr std::size_t FUNCS_COUNT = 7;

using DefaultArgs = std::vector<AnyArgs>;

class SubprogramInfo {
    friend class SubprogramLocator;
private:
    std::vector<AnyArgsFunc<bool>> funcs_;
    DefaultArgs default_args_;
    std::vector<ID> dependencies_;
    ID name_;

public:
    SubprogramInfo()
    :
        funcs_(),
        default_args_(),
        dependencies_(),
        name_("unnamed_subprogram") 
    {
        funcs_.resize(FUNCS_COUNT);
        default_args_.resize(FUNCS_COUNT);

        std::function<bool(AnyArgs&)> undefined_func;
        for (std::size_t i = 0; i < FUNCS_COUNT; ++i) {
            funcs_[i] = undefined_func;
            default_args_[i] = AnyArgs();
        }
    }

    void set_func(SubprogramFuncNames func_name, const AnyArgsFunc<bool>& func) 
    {
        funcs_[static_cast<std::size_t>(func_name)] = func;   
    }

    void set_default_args(SubprogramFuncNames func_name, const AnyArgs& args)
    {
        default_args_[static_cast<std::size_t>(func_name)] = args;
        default_args_[static_cast<std::size_t>(func_name)].reserve(args.size() + 1);
    } 

    void set_func(SubprogramFuncNames func_name, const AnyArgsFunc<bool>& func, const AnyArgs& default_args) 
    {
        funcs_[static_cast<std::size_t>(func_name)] = func;
        set_default_args(func_name, default_args);    
    }

    template <typename... ArgsT>
    void set_default_args(SubprogramFuncNames func_name, ArgsT&&... args)
    {
        default_args_[static_cast<std::size_t>(func_name)] = std::move(make_any_args(std::forward<ArgsT>(args)...));
        default_args_[static_cast<std::size_t>(func_name)].reserve(sizeof...(args) + 1);
    }

    template <typename... ArgsT>
    void set_func(SubprogramFuncNames func_name, const AnyArgsFunc<bool>& func, ArgsT&&... default_args) 
    {
        funcs_[static_cast<std::size_t>(func_name)] = func;
        set_default_args(func_name, std::forward<ArgsT>(default_args)...);    
    }

    void set_name(const ID& name)
    {
        name_ = name;
    }

    void set_name(ID&& name)
    {
        name_ = std::move(name);
    }

    void add_dependency(const ID& dependency)
    {
        dependencies_.push_back(dependency);
    }

    void add_dependency(ID&& dependency)
    {
        dependencies_.push_back(std::move(dependency));
    }
};

enum class SubprogramStates {
    NOT_EXISTED,
    UNDEFINED,
    DEFINED_WITHOUT_DEFINED_PARENT,
    READY_TO_INITIALIZE,
    STOPPED,
    STARTED,
    PAUSED,
    STARTED_WHEN_PARENT_PAUSED,
    STARTED_WHEN_PARENT_STOPPED,
    PAUSED_WHEN_PARENT_STOPPED
};

class SubprogramLocator {

    struct Subprogram {
        std::any data_{}; // output data
        std::unordered_map<ID, std::any*> parents_data_{};
        SubprogramInfo info_{};
        SubprogramStates state_{SubprogramStates::NOT_EXISTED};
        int vertex_{0};
    };

    struct BaseOperation {
        ID subprogram_name_;
        AnyArgs args_;
        SubprogramFuncNames func_name_;
    };

    struct AddOperation {
        SubprogramInfo info_;
    };

    struct RemoveOperation {
        ID subprogram_name_;
    };

    using CallOperation = std::variant<AddOperation, RemoveOperation, BaseOperation>;

    static const AnyArgsFunc<bool> empty_func_;
    std::unordered_map<ID, Subprogram> subprograms_{};
    Graph dependencies_graph_{}, inverse_dependencies_graph_{};
    std::unordered_map<int, ID> vertexes_to_subprogram_names_{};
    std::queue<CallOperation> call_queue_{};
    Subprogram* current_subprogram_{nullptr};
    bool is_on_call_{false};
    
public:
    SubprogramStates get_subprogram_state(const ID& subprogram_name);

    bool add(const SubprogramInfo& info);
    bool remove(const ID& subprogram_name);

    bool set_default_args(const ID& subprogram_name, SubprogramFuncNames func_name, const AnyArgs& args);
    bool set_default_args(const ID& subprogram_name, SubprogramFuncNames func_name, AnyArgs&& args);
    template <typename... ArgsT>
    bool set_default_args(const ID& subprogram_name, SubprogramFuncNames func_name, ArgsT&&... args)
    {
        return set_default_args(subprogram_name, func_name, std::move(make_any_args(std::forward<ArgsT>(args)...)));
    }

    std::unordered_map<ID, std::any*>* get_parents_data();
    std::any* get_data();

    bool init(const ID& subprogram_name, AnyArgs& args);
    bool deinit(const ID& subprogram_name, AnyArgs& args);
    bool start(const ID& subprogram_name, AnyArgs& args);
    bool start_as_paused(const ID& subprogram_name, AnyArgs& args);
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
    inline bool start_as_paused(const ID& subprogram_name, ArgsT&&... args)
    {
        return start_as_paused(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
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

    bool init(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return init(subprogram_name, empty_args);
    }

    bool deinit(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return deinit(subprogram_name, empty_args);
    }

    bool start(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return start(subprogram_name, empty_args);
    }

    bool start_as_paused(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return start_as_paused(subprogram_name, empty_args);
    }

    bool stop(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return stop(subprogram_name, empty_args);
    }

    bool resume(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return resume(subprogram_name, empty_args);
    }

    bool pause(const ID& subprogram_name)
    {
        AnyArgs empty_args;
        return pause(subprogram_name, empty_args);
    }

private:
    SubprogramStates get_subprogram(
        const ID& subprogram_name, 
        Subprogram** subprogram = nullptr);

    bool add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram = false);
    void update_undefined_subprogram(const SubprogramInfo& info);
    void try_make_dependent_subprograms_ready(Subprogram* ready_parent_subprogram_name);
    void restore_subprogram_to_undefined(
        Subprogram& subprogram, 
        const Subprogram& undefined_subprogram, 
        const std::unordered_set<ID>& added_undefined_subprograms);
    void get_dependency_order(
        Subprogram* subprogram, 
        SubprogramStates* target_states, 
        std::size_t target_states_count,
        std::deque<Subprogram*>& dependency_order,
        bool push_front = false,
        bool inverse_dependencies_graph = false);
    bool call_func(Subprogram& subprogram, SubprogramFuncNames handler_name, AnyArgs& args);
    void get_target_inverse_dependency_order(
        Subprogram* subprogram, 
        std::list<Subprogram *>& inverse_order,
        SubprogramStates* target_states, 
        std::size_t target_states_count, 
        SubprogramStates* skip_parent_states, 
        std::size_t skip_parent_states_count,
        struct IgnoredParentForSkipCountingRule* ignore_parent_for_skip_counting_rule,
        std::size_t ignore_parent_for_skip_counting_rule_count);
    bool deinit_change_states(Subprogram* cur_subprogram, AnyArgs& cur_args);
    bool start_change_state(Subprogram* cur_subprogram, AnyArgs& init_args, AnyArgs& start_args);
    bool process_calls_from_queue();
};

#endif  // INCLUDE_SUBPROGRAM_LOCATOR_HPP_
