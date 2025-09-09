#include "thread_pool.hpp"

ThreadPool::~ThreadPool()
{
    stop_all_threads();
}

bool ThreadPool::all_threads_can_stop()
{
    for (auto& thread_unit : named_thread_units_) {
        if (thread_unit.second.state.load(std::memory_order_acquire) == States::BUSY) {
            return false;
        }
    }
    
    if (get_busy_common_threads_number() > 0) {
        return false;
    }

    return true;
}

void ThreadPool::stop_all_threads()
{
    for (auto& thread_unit : named_thread_units_) {
        thread_unit.second.state.store(States::STOP, std::memory_order_acquire);
        thread_unit.second.cv.notify_one();
    }

    for (auto& thread_unit : named_thread_units_) {
        thread_unit.second.thread.join();
    }

    common_threads_.state.store(CommonStates::STOP, std::memory_order_release);
    common_threads_.common_cv.notify_all();

    for (auto& thread_unit : common_threads_.common_thread_units) {
        thread_unit.thread.join();
    }
}

std::size_t ThreadPool::get_named_threads_number() const
{
    return named_thread_units_.size();
}

bool ThreadPool::named_thread_unit_is_busy(const ID& thread_id) const
{
    decltype(named_thread_units_)::const_iterator thread_unit;

    {
        std::lock_guard<std::mutex> lock(named_mutex_);

        thread_unit = named_thread_units_.find(thread_id);
        if (thread_unit == named_thread_units_.end()) {
            return false;
        }
    }

    return thread_unit->second.state.load(std::memory_order_acquire) == States::BUSY;
}

bool ThreadPool::add_named_thread_unit(const ID& thread_id)
{
    decltype(named_thread_units_)::iterator thread_unit_it;

    {
        std::lock_guard<std::mutex> lock(named_mutex_);

        thread_unit_it = named_thread_units_.find(thread_id);
        if (thread_unit_it != named_thread_units_.end()) {
            return false;
        }

        thread_unit_it = named_thread_units_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(thread_id),
            std::forward_as_tuple()
        ).first;
    }

    NamedThreadUnit& unit = thread_unit_it->second;
    unit.state.store(States::IDLE, std::memory_order_relaxed);
    unit.id = thread_id;
    unit.thread = std::thread([&unit]{

        States state;

        while (true) {
            std::unique_lock<std::mutex> lock(unit.mutex);

            unit.cv.wait(lock, [&state, &unit] { return (state = unit.state.load(std::memory_order_acquire)) != States::IDLE; });
            lock.unlock();

            if (state == States::STOP) {
                break;
            }

            unit.activity();
            unit.state.store(States::IDLE, std::memory_order_release);
            lock.lock();
        }
    });

    return true;
}

std::size_t ThreadPool::get_common_threads_number() const
{
    return common_threads_.common_thread_units.size();
}

std::size_t ThreadPool::get_busy_common_threads_number() const
{
    return common_threads_.activity_count.load(std::memory_order_relaxed);
}

bool ThreadPool::add_common_threads(std::size_t count)
{
    if (!count) {
        return false;
    }

    if (common_threads_.state.load(std::memory_order_acquire) == CommonStates::NOT_HAVE_THREADS) {
        common_threads_.state.store(CommonStates::WORK, std::memory_order_release);
    }

    while (count--) {

        CommonThreadUnit* unit_ptr = nullptr;
        {
            std::lock_guard<std::mutex> lock(common_mutex_);
            CommonThreadUnit* unit_ptr = &common_threads_.common_thread_units.emplace_back();
        }

        CommonThreadUnit& unit = *unit_ptr;
        unit.thread = std::thread([this, &unit] {

            CommonStates state;

            while (true) {

                std::unique_lock<std::mutex> lock(this->common_mutex_);

                this->common_threads_.common_cv.wait(lock, [this, &unit, &state] {
                    state = this->common_threads_.state.load(std::memory_order_acquire);
                    return state == CommonStates::STOP || !this->common_threads_.new_activities.empty();
                });
                
                if (state == CommonStates::STOP) {
                    break;
                }

                unit.activity = std::move(this->common_threads_.new_activities.front());
                this->common_threads_.new_activities.pop();

                lock.unlock();

                this->common_threads_.activity_count.fetch_add(1, std::memory_order_relaxed);
                unit.activity();
                this->common_threads_.activity_count.fetch_sub(1, std::memory_order_relaxed);

                lock.lock();
            }
        });
    }

    return true;
}

bool ThreadPool::try_run_activity(const ID& thread_id, const Activity& activity)
{
    decltype(named_thread_units_)::iterator thread_unit_it;
    {
        std::lock_guard<std::mutex> lock(named_mutex_);
        thread_unit_it = named_thread_units_.find(thread_id);
        if (thread_unit_it == named_thread_units_.end()) {
            return false;
        }
    }

    NamedThreadUnit& thread_unit = thread_unit_it->second;

    {
        std::lock_guard<std::mutex> lock(thread_unit.mutex);

        if (thread_unit.state.load(std::memory_order_relaxed) == States::BUSY) {
            return false;
        }

        thread_unit.activity = activity;
        thread_unit.state.store(States::BUSY, std::memory_order_release);
        thread_unit.cv.notify_one();
    }

    return true;
}

bool ThreadPool::run_activity(const Activity &activity)
{
    {
        std::lock_guard<std::mutex> lock(common_mutex_);
        common_threads_.new_activities.push(activity);
    }

    common_threads_.common_cv.notify_one();

    return true;
}