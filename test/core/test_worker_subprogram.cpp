#include <catch2/catch_test_macros.hpp>
#include "core/subprogram_locator.hpp"
#include "core/thread_pool_subprogram.hpp"
#include "core/worker_subprogram.hpp"
//#include <iostream>

using namespace core;
using namespace core::subprograms;

SCENARIO("Worker Subprogram", "[core][subprogram][worker][multithreading]") {

    GIVEN("Worker Subprogram") {
        SubprogramLocator locator;

        install_thread_pool_subprogram(locator);

        WHEN("Add Worker") {

            SubprogramInfo info;
            info.set_name("worker");
            install_worker(info, [](std::any* data, SubprogramLocator::ParentsData* parents) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }, "worker_thread");
            locator.add(info);

            AND_WHEN("Init worker") {

                REQUIRE(locator.init("worker"));

                AND_WHEN("Start worker") {
                    REQUIRE(locator.start("worker"));
                    
                    AND_WHEN("Pause worker") {
                        REQUIRE(locator.pause("worker"));

                        AND_WHEN("Resume worker") {
                            REQUIRE(locator.resume("worker"));

                            AND_WHEN("Deinit thread_pool") {
                                REQUIRE(locator.deinit(THREAD_POOL_SUBPROGRAM_NAME, true));
                            }
                        }

                        AND_WHEN("Stop and deinit worker") {
                            REQUIRE(locator.stop("worker"));
                            REQUIRE(locator.deinit("worker"));
                        }
                    }

                    AND_WHEN("Stop worker") {
                        REQUIRE(locator.stop("worker"));

                        AND_WHEN("Deinit worker") {
                            REQUIRE(locator.deinit("worker"));
                        }
                    }
                }

                AND_WHEN("Start as paused worker") {
                    REQUIRE(locator.start_as_paused("worker"));

                    AND_WHEN("Resume worker") {
                        REQUIRE(locator.resume("worker"));
                        REQUIRE(locator.deinit("worker"));
                    }

                    AND_WHEN("Stop worker") {
                        REQUIRE(locator.stop("worker"));

                        AND_WHEN("Deinit worker") {
                            REQUIRE(locator.deinit("worker"));
                        }
                    }
                }
            }
        }

        WHEN("Add user common") {

            SubprogramInfo info_user_common;
            info_user_common.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>([](SubprogramLocator* locator){
                
                *locator->get_data() = 0;
                return true;
            }));
            info_user_common.set_name("user_common");
            locator.add(info_user_common);

            AND_WHEN("Add user1, user2 and user3") {

                SubprogramInfo info_user1;
                info_user1.set_name("user1_worker");
                install_worker(info_user1, [](std::any* data, SubprogramLocator::ParentsData* parents) {

                    int* status = SubprogramLocator::get_parent_data<int>(parents, "user_common");
                    *status = 1;

                },"user1_thread");
                info_user1.add_dependency("user_common");
                locator.add(info_user1);

                SubprogramInfo info_user2;
                info_user2.set_name("user2_worker");
                install_worker(info_user2, [](std::any* data, SubprogramLocator::ParentsData* parents) {

                    int* status = SubprogramLocator::get_parent_data<int>(parents, "user_common");
                    *status = 2;

                },"user2_thread");
                info_user2.add_dependency("user_common");
                locator.add(info_user2);

                SubprogramInfo info_user3;
                info_user3.set_name("user3_worker");
                std::atomic<int> status_result;
                install_worker(info_user3, [&status_result](std::any* data, SubprogramLocator::ParentsData* parents) {

                    int* status = SubprogramLocator::get_parent_data<int>(parents, "user_common");
                    status_result.store(*status, std::memory_order_release);

                },"user3_thread");
                info_user3.add_dependency("user_common");
                locator.add(info_user3);

                AND_WHEN("Start user1, user3") {
                    //std::cout << "Start user1, user3: begin" << std::endl;
                    //std::cout << "start" << std::endl;
                    REQUIRE(locator.start("user1_worker"));
                    //std::cout << "start" << std::endl;
                    REQUIRE(locator.start("user3_worker"));
                    //std::cout << "Start user1, user3: end" << std::endl;

                    THEN("Result is 1") {

                        int status = 0;
                        //std::cout << "Result is 1: Waiting for result" << std::endl;
                        while ((status = status_result.load(std::memory_order_relaxed)) == 0);
                        //std::cout << "Result is 1: Waiting end" << std::endl;

                        REQUIRE(status == 1);

                        AND_WHEN("Switch to user2") {
                            //std::cout << "Switch to user2: begin" << std::endl;
                            //std::cout << "pause" << std::endl;
                            REQUIRE(locator.pause("user1_worker"));
                            //std::cout << "start" << std::endl;
                            REQUIRE(locator.start("user2_worker"));
                            //std::cout << "Switch to user2: end" << std::endl;

                            //std::cout << "Switch to user2: Waiting for result" << std::endl;
                            while ((status = status_result.load(std::memory_order_relaxed)) == 1);
                            //std::cout << "Switch to user2: Waiting end" << std::endl;
                            REQUIRE(status == 2);

                            AND_WHEN("Switch to user1") {
                                //std::cout << "Switch to user1: begin" << std::endl;
                                //std::cout << "pause" << std::endl;
                                REQUIRE(locator.pause("user2_worker"));
                                //std::cout << "start" << std::endl;
                                REQUIRE(locator.resume("user1_worker"));
                                //std::cout << "Switch to user1: begin" << std::endl;
                                //std::cout << "Switch to user1: Waiting for result" << std::endl;
                                while ((status = status_result.load(std::memory_order_relaxed)) == 2);
                                //std::cout << "Switch to user1: Waiting end" << std::endl;
                                REQUIRE(status == 1);

                                //std::cout << "deinit begin" << std::endl;
                                REQUIRE(locator.deinit(THREAD_POOL_SUBPROGRAM_NAME));
                                //std::cout << "deinit end" << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
}
