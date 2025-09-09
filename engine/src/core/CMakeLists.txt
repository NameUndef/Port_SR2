cmake_minimum_required(VERSION 3.20)

add_subdirectory("subprogram_locator")
add_subdirectory("subprograms/worker_subprogram")
add_subdirectory("subprograms/thread_pool_subprogram")
add_subdirectory("subprograms/sdl_subprogram")
add_subdirectory("subprograms/ecs_subprogram")
add_subdirectory("subprograms/game_loop_subprogram")

add_library(${CORE_LIB_NAME} INTERFACE)
target_link_libraries(${CORE_LIB_NAME} 
    INTERFACE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    INTERFACE ${THREAD_POOL_SUBPROGRAM_LIB_NAME}
    INTERFACE ${WORKER_SUBPROGRAM_LIB_NAME}
    INTERFACE ${SDL_SUBPROGRAM_LIB_NAME}
    INTERFACE ${ECS_SUBPROGRAM_LIB_NAME}
    INTERFACE ${GAME_LOOP_SUBPROGRAM_LIB_NAME})
