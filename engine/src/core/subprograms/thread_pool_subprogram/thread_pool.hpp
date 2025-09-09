#ifndef INCLUDE_THREAD_POOL_HPP_
#define INCLUDE_THREAD_POOL_HPP_

#include "id.hpp"

#include <thread>
#include <unordered_map>
#include <list>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

class ThreadPool {

public:
        using Activity = std::function<void()>;

private:

    enum class States {
        IDLE,
        BUSY,
        STOP
    };

    enum class CommonStates {
        NOT_HAVE_THREADS,
        WORK,
        STOP
    };

    struct NamedThreadUnit {
        std::thread thread;
        Activity activity;
        ID id;
        std::atomic<States> state;
        std::mutex mutex;
        std::condition_variable cv;
    };

    struct CommonThreadUnit { // todo check alignas(64)
        std::thread thread;
        Activity activity;
    };

    struct CommonThreadsControl {
        std::list<CommonThreadUnit> common_thread_units;
        std::condition_variable common_cv;
        std::atomic<CommonStates> state{CommonStates::NOT_HAVE_THREADS};
        std::atomic<int> activity_count{0};
        std::queue<Activity> new_activities;
    };

protected:
    ThreadPool() = default;
    ~ThreadPool();
    bool all_threads_can_stop();

private:
    void stop_all_threads();

public:
    std::size_t get_named_threads_number() const;
    bool named_thread_unit_is_busy(const ID& thread_id) const;
    bool add_named_thread_unit(const ID& thread_id);
    bool try_run_activity(const ID& thread_id, const Activity& activity);
    
    std::size_t get_common_threads_number() const;
    std::size_t get_busy_common_threads_number() const;
    bool add_common_threads(std::size_t count);
    bool run_activity(const Activity& activity);

private:
    std::unordered_map<ID, NamedThreadUnit> named_thread_units_;
    CommonThreadsControl common_threads_;
    mutable std::mutex named_mutex_;
    mutable std::mutex common_mutex_;
};

#endif  // INCLUDE_THREAD_POOL_HPP_