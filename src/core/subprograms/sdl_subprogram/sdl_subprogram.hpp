#ifndef INCLUDE_SDL_SUBPROGRAM_HPP_
#define INCLUDE_SDL_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include <SDL.h>
#include <iostream>
#include <any>

namespace core::subprograms {

inline const char* SDL_SUBPROGRAM_NAME = "SDL";

struct WindowConfig {
    std::string window_title;
    int window_x; 
    int window_y;
    int window_w;
    int window_h;
};

struct SDLData {
    SDL_Window* window;
    SDL_Renderer* renderer;
};

    template <>
    SDLData* get_parent<SDLData>(SubprogramLocator* locator);

    template <>
    SDLData* get_parent<SDLData>(SubprogramLocator::ParentsData* parents_data);

void install_sdl_subprogram(SubprogramLocator& locator);

}

#endif  // INCLUDE_SDL_SUBPROGRAM_HPP_