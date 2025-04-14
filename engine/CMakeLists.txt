cmake_minimum_required(VERSION 3.20)

add_library(${HEADERS_ENGINE_LIB_NAME} INTERFACE)
target_include_directories(${HEADERS_ENGINE_LIB_NAME} INTERFACE "include")

add_subdirectory("lib/common")
add_subdirectory("lib/graph")
add_subdirectory("src/core")

add_library(${ENGINE_LIB_NAME} SHARED $<TARGET_OBJECTS:${COMMON_DUMMY_LIB_NAME}>)
target_link_libraries(${ENGINE_LIB_NAME}
    PUBLIC ${COMMON_LIB_NAME}
    PUBLIC ${GRAPH_LIB_NAME} 
    PUBLIC ${CORE_LIB_NAME}
    INTERFACE ${HEADERS_ENGINE_LIB_NAME})
install(TARGETS ${ENGINE_LIB_NAME})

add_subdirectory("test")
