#ifndef INCLUDE_SUBPROGRAM_LOCATOR_HPP_
#define INCLUDE_SUBPROGRAM_LOCATOR_HPP_

#include "any_args_func.hpp"
#include "id.hpp"
#include "graph.hpp"

#include <string>
#include <vector>
#include <unordered_map>

enum class SubprogramFuncNames {
    COMMON_INIT,
    COMMON_DEINIT,
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
    std::vector<ID> dependencies_;
    ID name_;
    bool is_multiinstance_supported_ = false;
};

using DefaultArgs = AnyArgs[static_cast<int>(SubprogramFuncNames::COUNT)];

enum class SubprogramStates {
    NOT_EXISTED,
    UNDEFINED,
    DEFINED_WITHOUT_DEFINED_PARENT,
    READY_TO_COMMON_INITIALIZE,
    READY_TO_INSTANCE_INITIALIZE,
    STOPPED,
    STARTED,
    PAUSED,
    STARTED_WHEN_PARENT_PAUSED,
    STARTED_WHEN_PARENT_STOPPED,
    PAUSED_WHEN_PARENT_STOPPED
};

class SubprogramLocator {

    struct SubprogramInstance {
        DefaultArgs default_args_;
        SubprogramStates state_;
        ID name_;
    };

    struct Subprogram {
        SubprogramInfo info_;
        std::vector<SubprogramInstance> instances_;
        int vertex_;
    };

    static constexpr std::size_t default_instance_idx = 0;

    static const AnyArgsFunc<bool> empty_func_;
    static const char default_instance_name[];

    std::unordered_map<ID, Subprogram> subprograms_;
    Graph dependencies_graph_, inverse_dependencies_graph_;
    std::unordered_map<int, ID> vertexes_to_subprogram_names_;
    
public:
    bool add(const SubprogramInfo& info);
    bool remove(const SubprogramInfo& info);

    bool common_init(const ID& subprogram_name, AnyArgs& args);
    bool common_deinit(const ID& subprogram_name, AnyArgs& args);

    bool init(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);
    bool deinit(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);
    bool start(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);
    bool stop(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);
    bool resume(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);
    bool pause(const ID& subprogram_name, const ID& instance_name, AnyArgs& args);

    inline bool init(const ID& subprogram_name, AnyArgs& args) 
    {
        init(subprogram_name, default_instance_name, args);
    }

    inline bool deinit(const ID& subprogram_name, AnyArgs& args)
    {
        deinit(subprogram_name, default_instance_name, args);
    }

    inline bool start(const ID& subprogram_name, AnyArgs& args)
    {
        start(subprogram_name, default_instance_name, args);
    }

    inline bool stop(const ID& subprogram_name, AnyArgs& args)
    {
        stop(subprogram_name, default_instance_name, args);
    }

    inline bool resume(const ID& subprogram_name, AnyArgs& args)
    {
        resume(subprogram_name, default_instance_name, args);
    }

    inline bool pause(const ID& subprogram_name, AnyArgs& args)
    {
        pause(subprogram_name, default_instance_name, args);
    }

    template <typename... ArgsT>
    inline bool common_init(const ID& subprogram_name, ArgsT&&... args)
    {
        return common_init(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>
    inline bool common_deinit(const ID& subprogram_name, ArgsT&&... args)
    {
        return common_deinit(subprogram_name, make_any_args(std::forward<ArgsT>(args)...));
    }

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

    template <typename... ArgsT>    
    inline bool init(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return init(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>    
    inline bool deinit(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return deinit(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>    
    inline bool start(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return start(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>    
    inline bool stop(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return stop(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>    
    inline bool resume(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return resume(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }

    template <typename... ArgsT>    
    inline bool pause(const ID& subprogram_name, const ID& instance_name, ArgsT&&... args)
    {
        return pause(subprogram_name, instance_name, make_any_args(std::forward<ArgsT>(args)...));
    }
    
    bool set_default_args(const ID& subprogram_name, const ID& instance_name, DefaultArgs& default_args);
    inline bool set_default_args(const ID& subprogram_name, DefaultArgs& default_args) 
    {
        return set_default_args(subprogram_name, default_instance_name, default_args);
    }

    SubprogramStates get_subprogram_state(const ID& subprogram_name, const ID& instance_name = default_instance_name) const;

private:
    bool add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram = true);
    void update_undefined_subprogram(const SubprogramInfo& info);
    void try_make_dependent_subprograms_ready(const ID& ready_parent_subprogram_name);
    void restore_subprogram_to_undefined(
        Subprogram& subprogram, 
        const Subprogram& undefined_subprogram, 
        const std::unordered_set<ID>& added_undefined_subprograms);
};

const AnyArgsFunc<bool> SubprogramLocator::empty_func_ = [](auto...) { return true; };
const char SubprogramLocator::default_instance_name[] = "default";

#endif  // INCLUDE_SUBPROGRAM_LOCATOR_HPP_
