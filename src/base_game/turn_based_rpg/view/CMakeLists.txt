cmake_minimum_required(VERSION 3.20)

add_library(${VIEW_SUBPROGRAM_LIB_NAME} STATIC "view_subprogram.cpp")
target_link_libraries(${VIEW_SUBPROGRAM_LIB_NAME}
    PRIVATE ${CORE_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
