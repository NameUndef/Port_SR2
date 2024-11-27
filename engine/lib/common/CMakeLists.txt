set(COMMON_LIB_SRC "")
list(APPEND COMMON_LIB_SRC 
    "id.cpp"
    "debug_helper.cpp")

add_library(${COMMON_LIB_NAME} STATIC ${COMMON_LIB_SRC})
add_library(${COMMON_DUMMY_LIB_NAME} OBJECT "dummy.cpp")