#include "sdl_subprogram.hpp"

using namespace core;
using namespace core::subprograms;

static bool init(WindowConfig& window_config, SubprogramLocator* locator)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Window* window = SDL_CreateWindow(window_config.window_title.c_str(),
    window_config.window_x, window_config.window_y,
    window_config.window_w, window_config.window_h, 
    SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    *locator->get_data() = SDLData{window, renderer};

    return true;
}

static bool deinit(SubprogramLocator* locator)
{
    SDLData* data = locator->get_data<SDLData>();

    SDL_DestroyRenderer(data->renderer);
    SDL_DestroyWindow(data->window);
    SDL_Quit();

    return true;
}

void core::subprograms::install_sdl_subprogram(SubprogramLocator& locator)
{
    SubprogramInfo info;

    AnyArgsFunction<bool> init_func = init;
    AnyArgsFunction<bool> deinit_func = deinit;

    WindowConfig window_config;
    window_config.window_title = "PSR2";
    window_config.window_x = SDL_WINDOWPOS_CENTERED;
    window_config.window_y = SDL_WINDOWPOS_CENTERED;
    window_config.window_w = 800;
    window_config.window_h = 600;

    info.set_name(SDL_SUBPROGRAM_NAME);
    info.set_func(SubprogramFuncNames::INIT, init_func, window_config);
    info.set_func(SubprogramFuncNames::DEINIT, deinit_func);
    locator.add(info);
}

template <>
SDLData* core::subprograms::get_parent<SDLData>(SubprogramLocator* locator)
{
    return locator->get_parent_data<SDLData>(SDL_SUBPROGRAM_NAME);
}