#ifndef INCLUDE_SUBPROGRAM_LOCATOR_HPP_
#define INCLUDE_SUBPROGRAM_LOCATOR_HPP_

#include "any_args_func.hpp"
#include "id.hpp"
#include "graph.hpp"

#include <string>
#include <vector>
#include <unordered_map>

struct SubprogramInfo {

    AnyArgsFunc<bool> common_init_,
                      common_deinit_,

                      init_,
                      deinit_,

                      start_,
                      stop_,

                      resume_,
                      pause_;

    std::vector<ID> dependencies_;
    ID name_;
    bool is_multiinstance_supported_ = false;
};

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

    struct Subprogram {
        SubprogramStates state_;
        ID name_;
    };

    struct SubprogramCommon {
        SubprogramInfo info_;
        std::vector<Subprogram> instances_;
        SubprogramStates state_;
        int vertex_;
    };

    static const AnyArgsFunc<bool> empty_func_;

    std::unordered_map<ID, SubprogramCommon> subprograms_;
    Graph dependencies_graph_, inverse_dependencies_graph_;
    std::unordered_map<int, ID> vertexes_to_subprogram_names_;
    
public:
    bool add(const SubprogramInfo& info);
    bool remove(const SubprogramInfo& info);

    bool common_init(const ID& subprogram_name);
    bool common_deinit(const ID& subprogram_name);
    bool init(const ID& subprogram_name);
    bool deinit(const ID& subprogram_name);
    bool start(const ID& subprogram_name);
    bool stop(const ID& subprogram_name);
    bool resume(const ID& subprogram_name);
    bool pause(const ID& subprogram_name);

    SubprogramStates get_subprogram_state(const ID& subprogram_name) const;

private:
    bool add_new_subprogram(const SubprogramInfo& info, bool is_undefined_subprogram = true);
    void update_undefined_subprogram(const SubprogramInfo& info);
    void try_make_dependent_subprograms_ready(const ID& ready_parent_subprogram_name);
    void restore_subprogram_to_undefined(
        SubprogramCommon& subprogram, 
        const SubprogramCommon& undefined_subprogram, 
        const std::unordered_set<ID>& added_undefined_subprograms);
};

const AnyArgsFunc<bool> SubprogramLocator::empty_func_ = [](auto...) { return true; };

#endif  // INCLUDE_SUBPROGRAM_LOCATOR_HPP_
