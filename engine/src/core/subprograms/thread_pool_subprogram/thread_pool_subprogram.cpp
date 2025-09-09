#include "thread_pool_subprogram.hpp"
#include "thread_pool.hpp"

class ThreadPoolInner : public ThreadPool {
public:
    bool all_threads_can_stop() { return ThreadPool::all_threads_can_stop(); }
};

static bool init(SubprogramLocator* locator)
{
    auto sptr = std::make_shared<ThreadPoolInner>();
    *locator->get_data() = std::move(sptr);
    return true; 
}

static bool deinit(SubprogramLocator* locator) 
{
    ThreadPoolInner* data = locator->get_data_from_sptr<ThreadPoolInner>();
    
    if (!data->all_threads_can_stop()) {
        return false;
    } 

    locator->get_data()->reset();
    return true;
}

void core::subprograms::install_thread_pool_subprogram(SubprogramLocator& subprogram_locator)
{
    SubprogramInfo info;
    info.set_name(THREAD_POOL_SUBPROGRAM_NAME);

    info.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>(init));
    info.set_func(SubprogramFuncNames::DEINIT, make_any_args_func<SubprogramLocator*>(deinit));

    subprogram_locator.add_new_subprogram(info);
}

template <>
ThreadPool* core::subprograms::get_parent<ThreadPool>(SubprogramLocator* locator)
{
    return locator->get_parent_data_from_sptr<ThreadPool, ThreadPoolInner>(THREAD_POOL_SUBPROGRAM_NAME);
}
