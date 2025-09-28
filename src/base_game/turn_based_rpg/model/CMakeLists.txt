cmake_minimum_required(VERSION 3.20)

add_library(${MODEL_SUBPROGRAM_LIB_NAME} STATIC 
    "model_subprogram.cpp"
    "game_time.cpp")
target_link_libraries(${MODEL_SUBPROGRAM_LIB_NAME}
    PRIVATE ${CORE_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
