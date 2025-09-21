cmake_minimum_required(VERSION 3.20)

add_subdirectory("turn_based_rpg")

add_library(${BASE_GAME_LIB_NAME} INTERFACE)
target_link_libraries(${BASE_GAME_LIB_NAME} 
    INTERFACE ${TURN_BASED_RPG_LIB_NAME})
