cmake_minimum_required(VERSION 3.20)

add_subdirectory("lib")
add_subdirectory("core")  

add_library(${TEST_ENGINE_LIB_NAME} INTERFACE)
target_link_libraries(${TEST_ENGINE_LIB_NAME} 
    INTERFACE ${ENGINE_LIB_NAME}
    INTERFACE $<TARGET_OBJECTS:${TEST_COMMON_LIB_NAME}>
    INTERFACE $<TARGET_OBJECTS:${TEST_CORE_LIB_NAME}>
    )
