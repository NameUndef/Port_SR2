cmake_minimum_required(VERSION 3.20)

add_library(${TEST_COMMON_LIB_NAME} OBJECT 
    "test_id.cpp"
    "test_any_args_func.cpp")
target_link_libraries(${TEST_COMMON_LIB_NAME} 
    PRIVATE Catch2::Catch2
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
