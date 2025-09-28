#ifndef INCLUDE_MULTITHREAD_SUBPROGRAM_LOCATOR_HPP_
#define INCLUDE_MULTITHREAD_SUBPROGRAM_LOCATOR_HPP_

#include "subprogram_locator.hpp"
#include <atomic>
#include <functional>
#include <unordered_map>
#include "id.hpp"
#include "any_args_func.hpp"
#include "saved_queue.hpp"

namespace core {

class MTSubprogramLocator {

public:
    using Task = std::function<void(core::SubprogramLocator*)>;

private:

    struct TaskUnit {
        ID name;
        AnyArgs args;
        std::atomic<bool> done;
        std::size_t type;
    };

    std::unordered_map<ID, Task> tasks_;
    SavedQueue<TaskUnit> runned_tasks_;
    SubprogramLocator* locator_;

protected:
    MTSubprogramLocator(SubprogramLocator* locator);
    ~MTSubprogramLocator() = default;
    void process_tasks();

public:
    bool add_task(const ID& id, const Task& task);
    bool run_task(const ID& id);

public:

};

}

#endif  // INCLUDE_MULTITHREAD_SUBPROGRAM_LOCATOR_HPP_