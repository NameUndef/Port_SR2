#include <catch2/catch_test_macros.hpp>
#include "core/subprogram_locator.hpp"
#include "any_args_func.hpp"
#include <iostream>

SCENARIO("Subprogram_Locator", "[core][locator]") {
    GIVEN("Subprogram Locator") {
        SubprogramLocator subprogram_locator;
        WHEN("Add subprogram") {
            SubprogramInfo subprogram_template;
            subprogram_template.set_name("A");
            subprogram_locator.add(subprogram_template);
            THEN("Subprogram was added") {
                REQUIRE(subprogram_locator.get_subprogram_state("A") == SubprogramStates::READY_TO_INITIALIZE);
            }
        }
        WHEN("Add undefined subprograms") {
            SubprogramInfo A, B, C;
            A.set_name("A");
            B.set_name("B");
            C.set_name("C");
            B.add_dependency("A");
            C.add_dependency("B");
            bool result = subprogram_locator.add(C);
            THEN("Subprogram C was added and is DEFINED_WITHOUT_DEFINED_PARENT") {
                REQUIRE(result);
                REQUIRE(subprogram_locator.get_subprogram_state("A") == SubprogramStates::NOT_EXISTED);
                REQUIRE(subprogram_locator.get_subprogram_state("B") == SubprogramStates::UNDEFINED);
                REQUIRE(subprogram_locator.get_subprogram_state("C") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
            }
            AND_WHEN("Added B") {
                result = subprogram_locator.add(B);
                THEN("Subprogram B was added and B, C is DEFINED_WITHOUT_DEFINED_PARENT") {
                    REQUIRE(result);
                    REQUIRE(subprogram_locator.get_subprogram_state("A") == SubprogramStates::UNDEFINED);
                    REQUIRE(subprogram_locator.get_subprogram_state("B") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                    REQUIRE(subprogram_locator.get_subprogram_state("C") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                }
                AND_WHEN("Added A") {
                    result = subprogram_locator.add(A);
                    THEN("Subprogram A, B, C was added and is READY_TO_INITIALIZE") {
                        REQUIRE(result);
                        REQUIRE(subprogram_locator.get_subprogram_state("A") == SubprogramStates::READY_TO_INITIALIZE);
                        REQUIRE(subprogram_locator.get_subprogram_state("B") == SubprogramStates::READY_TO_INITIALIZE);
                        REQUIRE(subprogram_locator.get_subprogram_state("C") == SubprogramStates::READY_TO_INITIALIZE);
                    }
                }
            }
        }
    }
    GIVEN("Subprogram graph") {
        SubprogramLocator locator;
        SubprogramInfo A, B, C, D, E, F, G, H, I, J;

        A.set_name("A");
        B.set_name("B");
        C.set_name("C");
        D.set_name("D");
        E.set_name("E");
        F.set_name("F");
        G.set_name("G");
        H.set_name("H");
        I.set_name("I");
        J.set_name("J");

        B.add_dependency("A");
        C.add_dependency("B");
        D.add_dependency("A");
        E.add_dependency("C");
        E.add_dependency("J");
        E.add_dependency("D");
        F.add_dependency("D");
        F.add_dependency("I");
        G.add_dependency("E");
        H.add_dependency("G");
        H.add_dependency("F");

        std::string order;
        auto func = make_any_args_func<const std::string&>([&order](auto value) {order += value; order.push_back('\n'); return true;});

        A.set_func(SubprogramFuncNames::INIT, func, std::string("A: Init"));
        B.set_func(SubprogramFuncNames::INIT, func, std::string("B: Init"));
        C.set_func(SubprogramFuncNames::INIT, func, std::string("C: Init"));
        D.set_func(SubprogramFuncNames::INIT, func, std::string("D: Init"));
        E.set_func(SubprogramFuncNames::INIT, func, std::string("E: Init"));
        F.set_func(SubprogramFuncNames::INIT, func, std::string("F: Init"));
        G.set_func(SubprogramFuncNames::INIT, func, std::string("G: Init"));
        H.set_func(SubprogramFuncNames::INIT, func, std::string("H: Init"));
        I.set_func(SubprogramFuncNames::INIT, func, std::string("I: Init"));
        J.set_func(SubprogramFuncNames::INIT, func, std::string("J: Init"));

        A.set_func(SubprogramFuncNames::DEINIT, func, std::string("A: Deinit"));
        B.set_func(SubprogramFuncNames::DEINIT, func, std::string("B: Deinit"));
        C.set_func(SubprogramFuncNames::DEINIT, func, std::string("C: Deinit"));
        D.set_func(SubprogramFuncNames::DEINIT, func, std::string("D: Deinit"));
        E.set_func(SubprogramFuncNames::DEINIT, func, std::string("E: Deinit"));
        F.set_func(SubprogramFuncNames::DEINIT, func, std::string("F: Deinit"));
        G.set_func(SubprogramFuncNames::DEINIT, func, std::string("G: Deinit"));
        H.set_func(SubprogramFuncNames::DEINIT, func, std::string("H: Deinit"));
        I.set_func(SubprogramFuncNames::DEINIT, func, std::string("I: Deinit"));
        J.set_func(SubprogramFuncNames::DEINIT, func, std::string("J: Deinit"));

        A.set_func(SubprogramFuncNames::START, func, std::string("A: Start"));
        B.set_func(SubprogramFuncNames::START, func, std::string("B: Start"));
        C.set_func(SubprogramFuncNames::START, func, std::string("C: Start"));
        D.set_func(SubprogramFuncNames::START, func, std::string("D: Start"));
        E.set_func(SubprogramFuncNames::START, func, std::string("E: Start"));
        F.set_func(SubprogramFuncNames::START, func, std::string("F: Start"));
        G.set_func(SubprogramFuncNames::START, func, std::string("G: Start"));
        H.set_func(SubprogramFuncNames::START, func, std::string("H: Start"));
        I.set_func(SubprogramFuncNames::START, func, std::string("I: Start"));
        J.set_func(SubprogramFuncNames::START, func, std::string("J: Start"));

        A.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("A: Start as paused"));
        B.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("B: Start as paused"));
        C.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("C: Start as paused"));
        D.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("D: Start as paused"));
        E.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("E: Start as paused"));
        F.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("F: Start as paused"));
        G.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("G: Start as paused"));
        H.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("H: Start as paused"));
        I.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("I: Start as paused"));
        J.set_func(SubprogramFuncNames::START_AS_PAUSED, func, std::string("J: Start as paused"));

        A.set_func(SubprogramFuncNames::STOP, func, std::string("A: Stop"));
        B.set_func(SubprogramFuncNames::STOP, func, std::string("B: Stop"));
        C.set_func(SubprogramFuncNames::STOP, func, std::string("C: Stop"));
        D.set_func(SubprogramFuncNames::STOP, func, std::string("D: Stop"));
        E.set_func(SubprogramFuncNames::STOP, func, std::string("E: Stop"));
        F.set_func(SubprogramFuncNames::STOP, func, std::string("F: Stop"));
        G.set_func(SubprogramFuncNames::STOP, func, std::string("G: Stop"));
        H.set_func(SubprogramFuncNames::STOP, func, std::string("H: Stop"));
        I.set_func(SubprogramFuncNames::STOP, func, std::string("I: Stop"));
        J.set_func(SubprogramFuncNames::STOP, func, std::string("J: Stop"));

        A.set_func(SubprogramFuncNames::PAUSE, func, std::string("A: Pause"));
        B.set_func(SubprogramFuncNames::PAUSE, func, std::string("B: Pause"));
        C.set_func(SubprogramFuncNames::PAUSE, func, std::string("C: Pause"));
        D.set_func(SubprogramFuncNames::PAUSE, func, std::string("D: Pause"));
        E.set_func(SubprogramFuncNames::PAUSE, func, std::string("E: Pause"));
        F.set_func(SubprogramFuncNames::PAUSE, func, std::string("F: Pause"));
        G.set_func(SubprogramFuncNames::PAUSE, func, std::string("G: Pause"));
        H.set_func(SubprogramFuncNames::PAUSE, func, std::string("H: Pause"));
        I.set_func(SubprogramFuncNames::PAUSE, func, std::string("I: Pause"));
        J.set_func(SubprogramFuncNames::PAUSE, func, std::string("J: Pause"));

        A.set_func(SubprogramFuncNames::RESUME, func, std::string("A: Resume"));
        B.set_func(SubprogramFuncNames::RESUME, func, std::string("B: Resume"));
        C.set_func(SubprogramFuncNames::RESUME, func, std::string("C: Resume"));
        D.set_func(SubprogramFuncNames::RESUME, func, std::string("D: Resume"));
        E.set_func(SubprogramFuncNames::RESUME, func, std::string("E: Resume"));
        F.set_func(SubprogramFuncNames::RESUME, func, std::string("F: Resume"));
        G.set_func(SubprogramFuncNames::RESUME, func, std::string("G: Resume"));
        H.set_func(SubprogramFuncNames::RESUME, func, std::string("H: Resume"));
        I.set_func(SubprogramFuncNames::RESUME, func, std::string("I: Resume"));
        J.set_func(SubprogramFuncNames::RESUME, func, std::string("J: Resume"));

        /* 
             A
             /\
            B |
            | D  I
         J  C |\ |
          \ | / \|
           \|/   F
            E   /
            |  /
            G /
            |/
            H
        */
       
        WHEN("Make graph without I and J") {
            bool result;
            result = locator.add(H);
            REQUIRE(result);
            result = locator.add(G);
            REQUIRE(result);
            result = locator.add(F);
            REQUIRE(result);
            result = locator.add(E);
            REQUIRE(result);
            result = locator.add(C);
            REQUIRE(result);
            result = locator.add(D);
            REQUIRE(result);
            result = locator.add(B);
            REQUIRE(result);
            result = locator.add(A);
            REQUIRE(result);
            THEN("All nodes ready to init") {
                REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::READY_TO_INITIALIZE);
                REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::READY_TO_INITIALIZE);
                REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::READY_TO_INITIALIZE);
                REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::READY_TO_INITIALIZE);
                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::UNDEFINED);
                REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::UNDEFINED);
            }
            AND_WHEN("Complete graph with I and J") {
                result = locator.add(I);
                bool result_2 = locator.add(J);
                THEN("E, G, F, H ready to init") {
                    REQUIRE(result);
                    REQUIRE(result_2);
                    REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::READY_TO_INITIALIZE);
                    REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::READY_TO_INITIALIZE);
                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::READY_TO_INITIALIZE);
                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::READY_TO_INITIALIZE);
                    REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::READY_TO_INITIALIZE);
                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::READY_TO_INITIALIZE);
                }

                AND_WHEN("Init I") {
                    result = locator.init("I");
                    THEN("I have STOPPED status") {
                        REQUIRE(result);
                        REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STOPPED);
                        REQUIRE(order == "I: Init\n");
                    }

                    AND_WHEN("Init D") {
                        order.clear();
                        result = locator.init("D");
                        THEN("A, D have STOPPED status") {
                            REQUIRE(result);
                            REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STOPPED);
                            REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STOPPED);
                            REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::READY_TO_INITIALIZE);
                            REQUIRE(order == "A: Init\nD: Init\n");
                        }

                        AND_WHEN("Init H") {
                            order.clear();
                            result = locator.init("H");
                            THEN("H have STOPPED status") {
                                REQUIRE(result);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STOPPED);
                                REQUIRE(order == 
                                        "B: Init\n"
                                        "C: Init\n"
                                        "J: Init\n"
                                        "E: Init\n"
                                        "G: Init\n"
                                        "F: Init\n"
                                        "H: Init\n");
                            }
                        }
                    }
                }

                AND_WHEN("Start graph") {
                    order.clear();
                    result = locator.start("H");
                    THEN("All have STARTED status") {
                        REQUIRE(result);
                        REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                        REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                        REQUIRE(order == 
                                "A: Init\n"
                                "A: Start\n"
                                "B: Init\n"
                                "B: Start\n"
                                "C: Init\n"
                                "C: Start\n"
                                "J: Init\n"
                                "J: Start\n"
                                "D: Init\n"
                                "D: Start\n"
                                "E: Init\n"
                                "E: Start\n"
                                "G: Init\n"
                                "G: Start\n"
                                "I: Init\n"
                                "I: Start\n"
                                "F: Init\n"
                                "F: Start\n"
                                "H: Init\n"
                                "H: Start\n");
                    }

                    AND_WHEN("Stop D") {
                        order.clear();
                        result = locator.stop("D");
                        THEN("D have STOPPED status") {
                            REQUIRE(result);
                            REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STOPPED);
                            REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                            REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                            REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                            REQUIRE(order == 
                                "H: Stop\n"
                                "F: Stop\n"
                                "G: Stop\n"
                                "E: Stop\n"
                                "D: Stop\n");
                        }
                        AND_WHEN("Stop G") {
                            order.clear();
                            result = locator.stop("G");
                            THEN("G have STOPPED status") {
                                REQUIRE(result);
                                REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STOPPED);
                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STOPPED);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                REQUIRE(order == "");
                            }
                            AND_WHEN("Pause I") {
                                order.clear();
                                result = locator.pause("I");
                                THEN("I have PAUSED status") {
                                    REQUIRE(result);
                                    REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STOPPED);
                                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                    REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STOPPED);
                                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                    REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                    REQUIRE(order == "I: Pause\n");
                                }

                                AND_WHEN("Start D") {
                                    order.clear();
                                    result = locator.start("D");
                                    THEN("D Started, F is STARTED_WHEN_PARENT_PAUSED") {
                                        REQUIRE(result);
                                        REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                        REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                        REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                        REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                        REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                                        REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_PAUSED);
                                        REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STOPPED);
                                        REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                                        REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::PAUSED);
                                        REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                        REQUIRE(
                                            "D: Start\n"
                                            "E: Start\n"
                                            "F: Start as paused\n");
                                    }

                                    AND_WHEN("Start G") {
                                        order.clear();
                                        result = locator.start("G");
                                        THEN("G started, H is STARTED_WHEN_PARENT_PAUSED") {
                                            REQUIRE(result);
                                            REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_PAUSED);
                                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED);
                                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_PAUSED);
                                            REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::PAUSED);
                                            REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                            REQUIRE(
                                                "G: Start\n"
                                                "H: Start as paused\n");
                                        }

                                        AND_WHEN("Resume I") {
                                            order.clear();
                                            result = locator.resume("I");
                                            THEN("I resumed") {
                                                REQUIRE(result);
                                                REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                                REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                                REQUIRE(order ==
                                                    "I: Resume\n"
                                                    "F: Resume\n"
                                                    "H: Resume\n");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AND_WHEN("Pause H, G, E") {
                        order.clear();
                        result = locator.pause("H");
                        REQUIRE(result);
                        result = locator.pause("G");
                        REQUIRE(result);
                        result = locator.pause("E");
                        REQUIRE(result);

                        THEN("H, G, E is PAUSED") {
                            REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::PAUSED);
                            REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::PAUSED);
                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::PAUSED);
                            REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                            REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                            REQUIRE(order ==
                            "H: Pause\n"
                            "G: Pause\n"
                            "E: Pause\n");
                        }

                        AND_WHEN("Stop J, C, pause D") {
                            order.clear();
                            result = locator.stop("J");
                            REQUIRE(result);
                            result = locator.stop("C");
                            REQUIRE(result);
                            result = locator.pause("D");
                            REQUIRE(result);

                            THEN("E, G, H is PAUSED_THEN_PARENT_STOPPED, F is STARTED_WHEN_PARENT_STOPPED") {
                                REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STOPPED);
                                REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::PAUSED);
                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED_WHEN_PARENT_PAUSED);
                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED);
                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STOPPED);
                                REQUIRE(order == 
                                        "H: Stop\n"
                                        "G: Stop\n"
                                        "E: Stop\n"
                                        "J: Stop\n"
                                        "C: Stop\n"
                                        "F: Pause\n"
                                        "D: Pause\n");
                            }

                            AND_WHEN("Resume D, Start J, C") {
                                order.clear();
                                result = locator.resume("D");
                                REQUIRE(result);
                                result = locator.start("J");
                                REQUIRE(result);
                                result = locator.start("C");
                                REQUIRE(result);

                                THEN("H, G, E is paused") {
                                    REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                    REQUIRE(order == 
                                            "D: Resume\n"
                                            "F: Resume\n"
                                            "J: Start\n"
                                            "C: Start\n"
                                            "E: Start as paused\n"
                                            "G: Start as paused\n"
                                            "H: Start as paused\n");
                                }
                            }

                            AND_WHEN("Start J, C, Resume D") {
                                order.clear();
                                result = locator.start("J");
                                REQUIRE(result);
                                result = locator.start("C");
                                REQUIRE(result);
                                result = locator.resume("D");
                                REQUIRE(result);

                                THEN("H, G, E is paused") {
                                    REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::PAUSED);
                                    REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                    REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                    REQUIRE(order == 
                                            "J: Start\n"
                                            "C: Start\n"
                                            "E: Start as paused\n"
                                            "G: Start as paused\n"
                                            "H: Start as paused\n"
                                            "D: Resume\n"
                                            "F: Resume\n");
                                }
                            }
                        }
                    }

                    AND_WHEN("Pause E, F, stop I") {
                        order.clear();
                        result = locator.pause("E");
                        REQUIRE(result);
                        result = locator.pause("F");
                        REQUIRE(result);
                        result = locator.stop("I");
                        REQUIRE(result);

                        THEN("H is STARTED_WHEN_PARENT_STOPPED, G is STARTED_WHEN_PARENT_PAUSED, F is PAUSED_WHEN_PARENT_STOPPED") {
                            REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::PAUSED);
                            REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STOPPED);
                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED_WHEN_PARENT_PAUSED);
                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED_WHEN_PARENT_STOPPED);
                            REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::PAUSED_WHEN_PARENT_STOPPED);
                            REQUIRE(order == 
                                    "H: Pause\n"
                                    "G: Pause\n"
                                    "E: Pause\n"
                                    "F: Pause\n"
                                    "H: Stop\n"
                                    "F: Stop\n"
                                    "I: Stop\n");
                        }

                        AND_WHEN("Resume E, Start I, Resume F") {
                            order.clear();
                            result = locator.resume("E");
                            REQUIRE(result);
                            result = locator.start("I");
                            REQUIRE(result);
                            result = locator.resume("F");
                            REQUIRE(result);

                            THEN("All started") {
                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                REQUIRE(order == 
                                        "E: Resume\n"
                                        "G: Resume\n"
                                        "I: Start\n"
                                        "F: Start as paused\n"
                                        "H: Start as paused\n"
                                        "F: Resume\n"
                                        "H: Resume\n");
                            }
                        }

                        AND_WHEN("Start I, Resume E, F") {
                            order.clear();
                            result = locator.start("I");
                            REQUIRE(result);
                            result = locator.resume("E");
                            REQUIRE(result);
                            result = locator.resume("F");
                            REQUIRE(result);

                            THEN("All started") {
                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                REQUIRE(order == 
                                        "I: Start\n"
                                        "F: Start as paused\n"
                                        "H: Start as paused\n"
                                        "E: Resume\n"
                                        "G: Resume\n"
                                        "F: Resume\n"
                                        "H: Resume\n");
                            }
                        }
                    }

                    AND_WHEN("deinit and remove graph") {
                        order.clear();
                        result = locator.deinit("E");
                        THEN("G, H is READY_TO_INITIALIZE") {
                            REQUIRE(result);
                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::READY_TO_INITIALIZE);
                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::READY_TO_INITIALIZE);
                            REQUIRE(order == 
                                    "H: Stop\n"
                                    "H: Deinit\n"
                                    "G: Stop\n"
                                    "G: Deinit\n"
                                    "E: Stop\n"
                                    "E: Deinit\n");
                        }

                        AND_WHEN("Remove C") {
                            order.clear();
                            result = locator.remove("C");
                            REQUIRE(result);

                            THEN("All stopped") {
                                REQUIRE(locator.get_subprogram_state("A") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("B") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::UNDEFINED);
                                REQUIRE(locator.get_subprogram_state("D") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                REQUIRE(locator.get_subprogram_state("F") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                REQUIRE(locator.get_subprogram_state("I") == SubprogramStates::STARTED);
                                REQUIRE(locator.get_subprogram_state("J") == SubprogramStates::STARTED);
                                REQUIRE(order ==
                                    "C: Stop\n"
                                    "C: Deinit\n");
                            }

                            AND_WHEN("Remove H, G, E") {
                                result = locator.remove("H");
                                REQUIRE(result);
                                result = locator.remove("G");
                                REQUIRE(result);
                                result = locator.remove("E");
                                REQUIRE(result);
                                THEN("H, G, E, C deleted") {
                                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::NOT_EXISTED);
                                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::NOT_EXISTED);
                                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::NOT_EXISTED);
                                    REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::NOT_EXISTED);
                                }
                            }

                            AND_WHEN("Remove E") {
                                result = locator.remove("E");
                                THEN("C not existed, E undefined, G, H DEFINED_WITHOUT_DEFINED_PARENT") {
                                    REQUIRE(result);
                                    REQUIRE(locator.get_subprogram_state("C") == SubprogramStates::NOT_EXISTED);
                                    REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::UNDEFINED);
                                    REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                    REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                }

                                AND_WHEN("Remove G") {
                                    result = locator.remove("G");
                                    THEN("E not existed, G undefined, H DEFINED_WITHOUT_DEFINED_PARENT") {    
                                        REQUIRE(result);                               
                                        REQUIRE(locator.get_subprogram_state("E") == SubprogramStates::NOT_EXISTED);
                                        REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::UNDEFINED);
                                        REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::DEFINED_WITHOUT_DEFINED_PARENT);
                                    }

                                    AND_WHEN("Remove H") {
                                        result = locator.remove("H");
                                        THEN("G, H not existed") {
                                            REQUIRE(result);
                                            REQUIRE(locator.get_subprogram_state("G") == SubprogramStates::NOT_EXISTED);
                                            REQUIRE(locator.get_subprogram_state("H") == SubprogramStates::NOT_EXISTED);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}