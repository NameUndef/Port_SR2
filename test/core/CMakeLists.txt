cmake_minimum_required(VERSION 3.20)

add_library(${TEST_CORE_LIB_NAME} OBJECT 
    "test_subprogram_locator.cpp"
    "test_thread_pool_subprogram.cpp"
    "test_worker_subprogram.cpp")
target_link_libraries(${TEST_CORE_LIB_NAME} 
    PRIVATE Catch2::Catch2
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
