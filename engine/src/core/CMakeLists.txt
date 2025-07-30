cmake_minimum_required(VERSION 3.20)

add_subdirectory("subprogram_locator")
add_subdirectory("managers/sdl_runtime_manager")

add_library(${CORE_LIB_NAME} INTERFACE)
target_link_libraries(${CORE_LIB_NAME} 
    INTERFACE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    INTERFACE ${SDL_RUNTIME_MANAGER_LIB_NAME})
