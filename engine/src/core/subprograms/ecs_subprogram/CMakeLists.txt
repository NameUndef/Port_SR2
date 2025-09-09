cmake_minimum_required(VERSION 3.20)

add_library(${ECS_SUBPROGRAM_LIB_NAME} STATIC 
    "ecs_world_subprogram.cpp"
    "ecs_worker_subprogram.cpp")
target_link_libraries(${ECS_SUBPROGRAM_LIB_NAME} 
    PRIVATE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME}
    PRIVATE ${WORKER_SUBPROGRAM_LIB_NAME}
    PUBLIC flecs::flecs)
