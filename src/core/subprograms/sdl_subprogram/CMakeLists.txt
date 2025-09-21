cmake_minimum_required(VERSION 3.20)

add_library(${SDL_SUBPROGRAM_LIB_NAME} STATIC "sdl_subprogram.cpp")
target_link_libraries(${SDL_SUBPROGRAM_LIB_NAME} 
    PRIVATE ${SUBPROGRAM_LOCATOR_LIB_NAME}
    PRIVATE ${HEADERS_ENGINE_LIB_NAME}
    PUBLIC SDL2::SDL2-static
    PUBLIC SDL2_image::SDL2_image-static
    PUBLIC SDL2_mixer::SDL2_mixer-static
    PUBLIC SDL2_net::SDL2_net-static
    PUBLIC SDL2_ttf::SDL2_ttf-static)
