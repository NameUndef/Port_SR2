cmake_minimum_required(VERSION 3.20)

add_subdirectory("locator")
#add_subdirectory("managers")

add_library(${CORE_LIB_NAME} INTERFACE)
target_link_libraries(${CORE_LIB_NAME} INTERFACE ${LOCATOR_LIB_NAME})
