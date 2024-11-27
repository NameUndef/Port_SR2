cmake_minimum_required(VERSION 3.20)

add_executable(${TEST_APP_NAME} "main.cpp")
target_link_libraries(${TEST_APP_NAME} 
    PRIVATE ${TEST_ENGINE_LIB_NAME}
    PRIVATE Catch2::Catch2)
install(TARGETS ${TEST_APP_NAME})

include(CTest)
include(Catch)
catch_discover_tests(${TEST_APP_NAME})
