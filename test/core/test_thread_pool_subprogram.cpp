#include <catch2/catch_test_macros.hpp>
#include "core/subprogram_locator.hpp"
#include "core/thread_pool_subprogram.hpp"
#include "any_args_func.hpp"
#include <iostream>
#include <atomic>

using namespace core;
using namespace core::subprograms;

SCENARIO("Thread Pool Subprogram", "[core][subprogram][thread_pool][multithreading]") {
    GIVEN("Thread Pool subprogram") {
        SubprogramLocator locator;
        install_thread_pool_subprogram(locator);

        WHEN("Add user1") {
            std::atomic<bool> user1_activity_done;
            SubprogramInfo info;

            user1_activity_done.store(false, std::memory_order_relaxed);
            AnyArgsFunc<bool> init_func = make_any_args_func<SubprogramLocator*>([&user1_activity_done](SubprogramLocator* locator) 
            {
                ThreadPool* pool = get_parent<ThreadPool>(locator);

                if (!pool->add_named_thread_unit("user1_thread")) {
                    return false;
                }

                if (!pool->try_run_activity("user1_thread", [&user1_activity_done] { 
                        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
                        user1_activity_done.store(true, std::memory_order_relaxed);
                    })) {
                    return false;
                }

                return true;
            });

            info.set_func(SubprogramFuncNames::INIT, init_func);
            info.set_name("user1");
            info.add_dependency(THREAD_POOL_SUBPROGRAM_NAME);
            
            REQUIRE(locator.add(info));

            AND_WHEN("Start user1") {
                user1_activity_done.store(false, std::memory_order_relaxed);
                REQUIRE(locator.init("user1"));

                AND_WHEN("Deinit thread pool") {

                    while (!user1_activity_done.load(std::memory_order_relaxed));

                    REQUIRE(locator.deinit(THREAD_POOL_SUBPROGRAM_NAME, true));
                }
            }
        }

        WHEN("Add user2") {
            std::atomic<int> global_count = 0;

            SubprogramInfo info;
            auto init_func = make_any_args_func<SubprogramLocator*>([&global_count](SubprogramLocator* locator) 
            {
                auto pool = get_parent<ThreadPool>(locator);

                pool->add_common_threads(10);

                ThreadPool::Activity activity = [&global_count] { 
                    
                    for (int i = 0; i < 5; i++) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        global_count.fetch_add(1, std::memory_order_relaxed);
                    }
                };

                for (int i = 0; i < 10; i++) {
                    pool->run_activity(activity);
                }

                return true; 
            });

            auto deinit_func = make_any_args_func<SubprogramLocator*>([&global_count](SubprogramLocator* locator) 
            {
                while (global_count.load(std::memory_order_relaxed) < 50);
                
                return true; 
            });

            info.set_func(SubprogramFuncNames::INIT, init_func);
            info.set_func(SubprogramFuncNames::DEINIT, deinit_func);
            info.set_name("user2");
            info.add_dependency(THREAD_POOL_SUBPROGRAM_NAME);

            REQUIRE(locator.add(info));

            THEN("Work user2") {
                
                REQUIRE(locator.init("user2"));
                REQUIRE(locator.deinit(THREAD_POOL_SUBPROGRAM_NAME));
            }
        }
    }
}
