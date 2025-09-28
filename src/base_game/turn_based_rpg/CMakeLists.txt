cmake_minimum_required(VERSION 3.20)

add_subdirectory("model")
add_subdirectory("view")
add_subdirectory("controller")

add_library(${TURN_BASED_RPG_LIB_NAME} INTERFACE)
target_link_libraries(${TURN_BASED_RPG_LIB_NAME}
    INTERFACE ${MODEL_SUBPROGRAM_LIB_NAME}
    INTERFACE ${VIEW_SUBPROGRAM_LIB_NAME}
    INTERFACE ${CONTROLLER_SUBPROGRAM_LIB_NAME})
