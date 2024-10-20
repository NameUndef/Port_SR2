
cmake_minimum_required (VERSION 3.20)
project ("ProjectQaragon")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)
find_package(SDL2_ttf REQUIRED)    

if(NOT SDL2_FOUND)
    message("Error: SDL not found")
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

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/lib/imgui-1.91.4")
file(GLOB IMGUI_CPP_FILES "${IMGUI_DIR}/*.cpp")
set(IMGUI_CPP_FILES_LIST "${IMGUI_CPP_FILES}")
LIST(APPEND IMGUI_CPP_FILES_LIST "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp")
include_directories("${IMGUI_DIR}" "${IMGUI_DIR}/backends")

find_package(flecs REQUIRED)
install(IMPORTED_RUNTIME_ARTIFACTS flecs::flecs)

find_package(nlohmann_json REQUIRED)

add_library(lua STATIC IMPORTED)
set_target_properties(lua PROPERTIES 
IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/lua/lib/liblua.a 
INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_PREFIX_PATH}/lua/include 
VERSION 5.4.4)

include_directories("include")
add_subdirectory("src")
