#include "worker_subprogram.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "core/thread_pool_subprogram.hpp"
#include "id.hpp"

// #include <iostream>

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

    if (!thread_pool->have_named_thread(data.thread_id)) {
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
                //std::cout << "Worker in work" << std::endl;
                do {
                    data.process(locator->get_data(), parents_data);
                } while ((state = data.state.load(std::memory_order_acquire)) == States::WORK);
            }

            if (state == States::PAUSING) {
                //std::cout << "state = PAUSE" << std::endl;
                data.state.store(States::PAUSE, std::memory_order_relaxed);
                //std::cout << "notify" << std::endl;
                {
                    std::unique_lock<std::mutex> lock(data.mutex);
                    data.cv.notify_one();
                }
                state = States::PAUSE;
                //std::cout << "Worker: pausing end" << std::endl;

            } else if (state == States::STOPING) {
                //std::cout << "Worker in stoping" << std::endl;
                data.state.store(States::STOP, std::memory_order_release);
                {
                    std::unique_lock<std::mutex> lock(data.mutex);
                    data.cv.notify_one();
                }
                state = States::STOP;
                //std::cout << "Worker: stoping end" << std::endl;
            }
            
            if (state == States::PAUSE) {
                //std::cout << "Worker in pause" << std::endl;
                std::unique_lock<std::mutex> lock(data.mutex);
                data.cv.wait(lock, [&data] { 
                        return data.state.load(std::memory_order_relaxed) != States::PAUSE; 
                    });
                //std::cout << "Worker: Pause end" << std::endl;
            }
        }
        //std::cout << "Worker: stop" << std::endl;
    };

    if (!thread_pool->try_run_activity(data.thread_id, activity)) {
        return false;
    }

    return true;
}

static bool stop(SubprogramLocator* locator)
{
    //std::cout << "MT Stop() begin" << std::endl;
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();
    States state = data.state.load(std::memory_order_acquire);

    if (state == States::WORK) {
        //std::cout << "MT send STOPING" << std::endl;
        data.state.store(States::STOPING, std::memory_order_release);
        std::unique_lock<std::mutex> lock(data.mutex);
        data.cv.wait(lock, [&data] { return data.state.load(std::memory_order_relaxed) == States::STOP; });
        //std::cout << "MT done STOP" << std::endl;

    } else if (state == States::PAUSE) {
        //std::cout << "MT send STOPING, when worker pause" << std::endl;
        data.state.store(States::STOP, std::memory_order_relaxed);
        {
            std::unique_lock<std::mutex> lock(data.mutex);
            data.cv.notify_one();
        }
        //std::cout << "MT done STOP" << std::endl;
    }
    //std::cout << "MT Stop() end" << std::endl;
    return true;
}

static bool pause(SubprogramLocator* locator)
{
    //std::cout << "MT Pause() begin" << std::endl;
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();

    //std::cout << "MT out CS" << std::endl;
    data.state.store(States::PAUSING, std::memory_order_release);
    std::unique_lock<std::mutex> lock(data.mutex);
    //std::cout << "MT in CS" << std::endl;
    data.cv.wait(lock, [&data] {
        return data.state.load(std::memory_order_relaxed) == States::PAUSE; });
    //std::cout << "MT done pause" << std::endl;
    //std::cout << "MT Pause() end" << std::endl;
    return true;
}

static bool resume(SubprogramLocator* locator)
{
    //std::cout << "Resume begin" << std::endl;
    WorkerPrivateData& data = *locator->get_private_data_from_sptr<WorkerPrivateData>();
    data.state.store(States::WORK, std::memory_order_relaxed);
    {
        std::unique_lock<std::mutex> lock(data.mutex);
        data.cv.notify_one();
    }
    //std::cout << "Resume end" << std::endl;
    return true;
}

void core::subprograms::install_worker(SubprogramInfo &info, const Process &process, const ID& thread_id)
{
    AnyArgsFunction<bool> start_func = &start;
    AnyArgsFunction<bool> init_func = &init;

    info.set_func(SubprogramFuncNames::INIT, init_func, process, ID(thread_id));
    info.set_func(SubprogramFuncNames::DEINIT, make_any_args_func<SubprogramLocator*>(deinit));
    info.set_func(SubprogramFuncNames::START, start_func, false);
    info.set_func(SubprogramFuncNames::START_AS_PAUSED, start_func, true);
    info.set_func(SubprogramFuncNames::STOP, make_any_args_func<SubprogramLocator*>(stop));
    info.set_func(SubprogramFuncNames::PAUSE, make_any_args_func<SubprogramLocator*>(pause));
    info.set_func(SubprogramFuncNames::RESUME, make_any_args_func<SubprogramLocator*>(resume));

    info.add_dependency(THREAD_POOL_SUBPROGRAM_NAME);
}
