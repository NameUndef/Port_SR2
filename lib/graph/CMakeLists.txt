cmake_minimum_required(VERSION 3.20)

add_library(${GRAPH_LIB_NAME} STATIC "graph.cpp")
target_link_libraries(${GRAPH_LIB_NAME} PRIVATE ${HEADERS_ENGINE_LIB_NAME})
