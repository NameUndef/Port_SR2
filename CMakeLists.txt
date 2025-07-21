## ---------- HEAD ---------- ##

cmake_minimum_required (VERSION 3.20)
project ("PSR2")

## ---------- EXTERNAL DEPENDENCIES ---------- ##

## SDL2

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)
find_package(SDL2_ttf REQUIRED)    

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

set(ENGINE_LIB_NAME "engine")
    set(COMMON_LIB_NAME "common")
        set(COMMON_DUMMY_LIB_NAME "dummy")
    set(GRAPH_LIB_NAME "graph")
    set(CORE_LIB_NAME "core")
        set(SUBPROGRAM_LOCATOR_LIB_NAME "subprogram_locator")

set(TEST_ENGINE_LIB_NAME "test_engine")    
    set(TEST_INNER_LIB_NAME "test_inner")
    set(TEST_CORE_LIB_NAME "test_core")

set(TEST_APP_NAME "run_test_engine")

## ---------- SUBDIRS ---------- ##

add_subdirectory("engine")
add_subdirectory("apps/test_app")
