#include "worker_subprogram.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "core/thread_pool_subprogram.hpp"
#include "id.hpp"

using namespace core;
using namespace core::subprograms;

enum class States {
    WORK,
    STOPING,
    STOP,
    PAUSING,
    PAUSE
};

struct WorkerPrivateData {
    
    std::atomic<States> state;
    ID thread_id;
    std::mutex mutex;
    std::condition_variable cv;
    Process process;
};

static bool init(const Process& process, const ID& thread_id, SubprogramLocator* locator)
{
    auto sptr = std::make_shared<WorkerPrivateData>();
    WorkerPrivateData& data = *sptr;
    data.process = process;
    data.thread_id = thread_id;

    ThreadPool* thread_pool = get_parent<ThreadPool>(locator);

    if (thread_pool->have_named_thread(thread_id)) {
        if (!thread_pool->add_named_thread_unit(data.thread_id)) {
            return false;
        }
    }

    *locator->get_private_data() = std::move(sptr);

    return true;
}

static bool deinit(SubprogramLocator* locator) 
{ 
    locator->get_private_data()->reset();
    return true; 
}

static bool start(bool start_as_paused, SubprogramLocator* locator)
{
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();
    
    SubprogramLocator::ParentsData* parents_data = locator->get_parents_data();

    if (start_as_paused) {
        data.state.store(States::PAUSE, std::memory_order_relaxed);
    } {
        data.state.store(States::WORK, std::memory_order_release);        
    }

    ThreadPool* thread_pool = get_parent<ThreadPool>(locator);

    ThreadPool::Activity activity = [&data, locator, parents_data] {
       
        States state;
        while ((state = data.state.load(std::memory_order_acquire)) != States::STOP) {
            
            if (state == States::WORK) {
                do {
                    data.process(locator->get_data(), parents_data);
                } while ((state = data.state.load(std::memory_order_relaxed)) == States::WORK);
            }

            if (state == States::PAUSING) {
                data.state.store(States::PAUSE, std::memory_order_release);
                data.cv.notify_one();
                state = States::PAUSE;

            } else if (state == States::STOPING) {
                data.state.store(States::STOP, std::memory_order_release);
                data.cv.notify_one();
                state = States::STOP;
            }
            
            if (state == States::PAUSE) {
                std::unique_lock<std::mutex> lock(data.mutex);
                data.cv.wait(lock, [&data] { 
                        return data.state.load(std::memory_order_relaxed) != States::PAUSE; 
                    });
            }
        }

    };

    if (!thread_pool->try_run_activity(data.thread_id, activity)) {
        return false;
    }

    return true;
}

static bool stop(SubprogramLocator* locator)
{
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();
    States state = data.state.load(std::memory_order_relaxed);

    if (state == States::WORK) {
        data.state.store(States::STOPING, std::memory_order_relaxed);
        std::unique_lock<std::mutex> lock(data.mutex);
        data.cv.wait(lock, [&data] { return data.state.load(std::memory_order_acquire) == States::STOP; });

    } else if (state == States::PAUSE) {
        data.state.store(States::STOP, std::memory_order_relaxed);
        data.cv.notify_one();
    }

    return true;
}

static bool pause(SubprogramLocator* locator)
{
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();

    data.state.store(States::PAUSING, std::memory_order_relaxed);
    std::unique_lock<std::mutex> lock(data.mutex);
    data.cv.wait(lock, [&data]{ return data.state.load(std::memory_order_acquire) == States::PAUSE; });

    return true;
}

static bool resume(SubprogramLocator* locator)
{
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();
    data.state.store(States::WORK, std::memory_order_release);
    data.cv.notify_one();
    return true;
}

void core::subprograms::install_worker(SubprogramInfo &info, const Process &process, const ID& thread_id)
{
    AnyArgsFunction<bool> start_func = &start;
    AnyArgsFunction<bool> init_func = &init;
    info.set_func(SubprogramFuncNames::INIT, init_func, process, thread_id);
    info.set_func(SubprogramFuncNames::DEINIT, make_any_args_func<SubprogramLocator*>(deinit));
    info.set_func(SubprogramFuncNames::START, start_func, false);
    info.set_func(SubprogramFuncNames::START_AS_PAUSED, start_func, true);
    info.set_func(SubprogramFuncNames::STOP, make_any_args_func<SubprogramLocator*>(stop));
    info.set_func(SubprogramFuncNames::PAUSE, make_any_args_func<SubprogramLocator*>(pause));
    info.set_func(SubprogramFuncNames::RESUME, make_any_args_func<SubprogramLocator*>(resume));

    info.add_dependency(THREAD_POOL_SUBPROGRAM_NAME);
}
