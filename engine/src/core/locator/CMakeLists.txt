cmake_minimum_required(VERSION 3.20)

add_library(${LOCATOR_LIB_NAME} STATIC 
    "locator.cpp")

target_link_libraries(${LOCATOR_LIB_NAME} 
    PRIVATE ${COMMON_LIB_NAME})