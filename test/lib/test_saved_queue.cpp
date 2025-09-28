#include <catch2/catch_test_macros.hpp>
#include "saved_queue.hpp"

SCENARIO("Saved Queue", "[inner_lib][common][saved_queue]")
{
    GIVEN("A saved queue")
    {
        SavedQueue<int> queue;
        WHEN("Queue is empty")
        {
            THEN("Queue is empty") {
                REQUIRE(queue.empty());
            }
            THEN("front and back is nullptr") {
                REQUIRE(queue.front() == nullptr);
                REQUIRE(queue.back() == nullptr);
            }
            THEN("queue size is 0") {
                REQUIRE(queue.size() == 0);
            }
        }
        WHEN("Add 3 elements") {
            queue.add_element(1);
            queue.add_element(2);
            queue.add_element(3);

            THEN("Queue is empty") {
                REQUIRE(queue.empty());
            }
            THEN("front and back is nullptr") {
                REQUIRE(queue.front() == nullptr);
                REQUIRE(queue.back() == nullptr);
            }
            THEN("elements size is 3") {
                REQUIRE(queue.elements_size() == 3);
            }
            THEN("elements is not empty") {
                REQUIRE(!queue.elements_empty());
            }

            AND_WHEN("push 3 elements in queue") {
                queue.push();
                queue.push();
                queue.push();
                THEN("front and back is not nullptr") {
                    REQUIRE(queue.front() != nullptr);
                    REQUIRE(queue.back() != nullptr);
                }
                THEN("queue size is 3") {
                    REQUIRE(queue.size() == 3);
                }

                AND_WHEN("push 1 in queue") {
                    queue.push();

                    THEN("elements size is 4") {
                        REQUIRE(queue.elements_size() == 4);
                    }
                }
            }
        }
    }
}