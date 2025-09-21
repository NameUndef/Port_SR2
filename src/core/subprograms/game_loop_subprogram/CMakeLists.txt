cmake_minimum_required(VERSION 3.20)

add_library(${GAME_LOOP_SUBPROGRAM_LIB_NAME} STATIC "game_loop_subprogram.cpp")
target_link_libraries(${GAME_LOOP_SUBPROGRAM_LIB_NAME} 
    PRIVATE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME}
    PRIVATE ${SDL_SUBPROGRAM_LIB_NAME})
