cmake_minimum_required(VERSION 3.20)

add_library(${TURN_BASED_RPG_LIB_NAME} STATIC "controller/controller_main.cpp")
target_link_libraries(${TURN_BASED_RPG_LIB_NAME} 
    PRIVATE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME}
    PRIVATE ${SDL_RUNTIME_MANAGER_LIB_NAME})
