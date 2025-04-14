cmake_minimum_required(VERSION 3.20)

add_library(${SUBPROGRAM_LOCATOR_LIB_NAME} STATIC 
    "subprogram_locator.cpp")

target_link_libraries(${SUBPROGRAM_LOCATOR_LIB_NAME} 
    PRIVATE ${COMMON_LIB_NAME}
    PRIVATE ${GRAPH_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
    