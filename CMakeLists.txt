## ---------- HEAD ---------- ##

cmake_minimum_required (VERSION 3.20)
project ("PSR2")

## ---------- EXTERNAL DEPENDENCIES ---------- ##

## SDL2

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image REQUIRED CONFIG)
find_package(SDL2_mixer REQUIRED CONFIG)
find_package(SDL2_net REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)    

if(NOT SDL2_FOUND)
    message("Error: SDL2 not found")
endif()

install(IMPORTED_RUNTIME_ARTIFACTS 
SDL2::SDL2 
SDL2_image::SDL2_image 
SDL2_mixer::SDL2_mixer 
SDL2_net::SDL2_net 
SDL2_ttf::SDL2_ttf)

set(SDL_GPU_LIBRARY "-L${CMAKE_PREFIX_PATH}/SDL_gpu/lib/libSDL2_gpu_s.a")
set(SDL_GPU_INCLUDE_DIR "${CMAKE_PREFIX_PATH}/SDL_gpu/include")
include_directories(${SDL_GPU_INCLUDE_DIR})

## IMGUI

set(IMGUI_DIR "${CMAKE_PREFIX_PATH}/imgui-1.91.4")
file(GLOB IMGUI_CPP_FILES "${IMGUI_DIR}/*.cpp")
set(IMGUI_CPP_FILES_LIST "${IMGUI_CPP_FILES}")
LIST(APPEND IMGUI_CPP_FILES_LIST "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp")
include_directories("${IMGUI_DIR}" "${IMGUI_DIR}/backends")

## FLECS

find_package(flecs REQUIRED)
install(IMPORTED_RUNTIME_ARTIFACTS flecs::flecs)

## NLOHMANN JSON

find_package(nlohmann_json REQUIRED)

## LUA

add_library(lua STATIC IMPORTED)
set_target_properties(lua PROPERTIES 
IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lua/lib/liblua.a 
INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_PREFIX_PATH}/lua/include 
VERSION 5.4.4)

## CATCH2

find_package(Catch2 REQUIRED)
if(NOT Catch2_FOUND)
    message("Error: Catch2 not found")
endif()
include_directories(${Catch2_INCLUDE_DIRS})

## ---------- LINK LIBRARIES ---------- ##

# list(APPEND YOUR_LIB_SRC ${IMGUI_CPP_FILES_LIST})

# for SDL2_GPU header compile
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal -Wno-error=float-equal -Wswitch-default -Wno-error=switch-default -Wno-error=zero-as-null-pointer-constant -Wzero-as-null-pointer-constant")

# target_link_libraries(YOUR_LIB PUBLIC
# SDL2::SDL2main 
# SDL2::SDL2 
# SDL2_image::SDL2_image 
# SDL2_mixer::SDL2_mixer 
# SDL2_net::SDL2_net 
# SDL2_ttf::SDL2_ttf
# nlohmann_json::nlohmann_json
# flecs::flecs
# lua
# ${SDL_GPU_LIBRARY}
# Catch2::Catch2)

## ---------- NAMES ---------- ##

set(HEADERS_ENGINE_LIB_NAME "headers_engine")

set(ENGINE_BUNDLE_LIB_NAME "engine_bundle")
    set(COMMON_LIB_NAME "common")
        set(COMMON_DUMMY_LIB_NAME "dummy")
    set(GRAPH_LIB_NAME "graph")
    set(CORE_LIB_NAME "core")
        set(SUBPROGRAM_LOCATOR_LIB_NAME "subprogram_locator")
        set(THREAD_POOL_SUBPROGRAM_LIB_NAME "thread_pool_subprogram")
        set(WORKER_SUBPROGRAM_LIB_NAME "worker_subprogram")
        set(SDL_SUBPROGRAM_LIB_NAME "sdl_subprogram")
        set(ECS_SUBPROGRAM_LIB_NAME "ecs_subprogram")
        set(GAME_LOOP_SUBPROGRAM_LIB_NAME "game_loop_subprogram")
    set(BASE_GAME_LIB_NAME "base_game")
        set(TURN_BASED_RPG_LIB_NAME "turn_based_rpg")
            set(MODEL_SUBPROGRAM_LIB_NAME "model_subprogram")
            set(VIEW_SUBPROGRAM_LIB_NAME "view_subprogram")
            set(CONTROLLER_SUBPROGRAM_LIB_NAME "controller_subprogram")

set(TEST_ENGINE_BUNDLE_LIB_NAME "test_engine_bundle")    
    set(TEST_INNER_LIB_NAME "test_inner")
    set(TEST_CORE_LIB_NAME "test_core")

set(ENGINE_LIB_NAME "engine")
set(TEST_ENGINE_LIB_NAME "test_engine")

set(LAUNCHER_APP_NAME "launcher")
set(TEST_APP_NAME "run_test_engine")

## ---------- MAIN BIN TARGETS ---------- ##

# Engine 

add_library(${HEADERS_ENGINE_LIB_NAME} INTERFACE)
target_include_directories(${HEADERS_ENGINE_LIB_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory("lib/common")
add_subdirectory("lib/graph")
add_subdirectory("src/core")
add_subdirectory("src/base_game")
add_subdirectory("test")

add_library(${ENGINE_BUNDLE_LIB_NAME} INTERFACE)
target_link_libraries(${ENGINE_BUNDLE_LIB_NAME}
    INTERFACE ${COMMON_LIB_NAME}
    INTERFACE ${GRAPH_LIB_NAME} 
    INTERFACE ${CORE_LIB_NAME}
    INTERFACE ${BASE_GAME_LIB_NAME}
    INTERFACE ${HEADERS_ENGINE_LIB_NAME})

add_library(${ENGINE_LIB_NAME} SHARED "src/runners/engine/engine.cpp")
target_link_libraries(${ENGINE_LIB_NAME}
    PRIVATE ${ENGINE_BUNDLE_LIB_NAME})
target_include_directories(${ENGINE_LIB_NAME} PUBLIC "src/runners/engine")

add_library(${TEST_ENGINE_LIB_NAME} SHARED 
    "src/runners/test_engine/test_engine.cpp")
target_link_libraries(${TEST_ENGINE_LIB_NAME}
    PRIVATE ${ENGINE_BUNDLE_LIB_NAME}
    PRIVATE ${TEST_ENGINE_BUNDLE_LIB_NAME}
    PRIVATE Catch2::Catch2)
target_include_directories(${TEST_ENGINE_LIB_NAME} PUBLIC "src/runners/test_engine")

install(TARGETS ${ENGINE_LIB_NAME} ${TEST_ENGINE_LIB_NAME})

# Launcher

add_executable(${LAUNCHER_APP_NAME} "src/apps/launcher/main.cpp")
target_link_libraries(${LAUNCHER_APP_NAME} 
    PRIVATE ${ENGINE_LIB_NAME})
install(TARGETS ${LAUNCHER_APP_NAME})

# Test app

add_executable(${TEST_APP_NAME} "src/apps/test_app/main.cpp")
target_link_libraries(${TEST_APP_NAME} 
    PRIVATE ${TEST_ENGINE_LIB_NAME})
install(TARGETS ${TEST_APP_NAME})

include(CTest)
include(Catch)
catch_discover_tests(${TEST_APP_NAME})
