cmake_minimum_required(VERSION 3.20)

add_library(${THREAD_POOL_SUBPROGRAM_LIB_NAME} STATIC 
    "thread_pool.cpp"
    "thread_pool_subprogram.cpp")
target_link_libraries(${THREAD_POOL_SUBPROGRAM_LIB_NAME} 
    PRIVATE ${COMMON_LIB_NAME}
    PRIVATE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME})
