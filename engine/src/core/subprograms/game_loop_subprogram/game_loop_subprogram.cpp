#include "game_loop_subprogram.hpp"
#include "core/subprogram_locator.hpp"
#include "core/sdl_subprogram.hpp"
#include "any_args_func.hpp"

using namespace core;
using namespace core::subprograms;

static bool init(GameLoopData** data_ret, SubprogramLocator* locator)
{
    GameLoopData data;
    data.running = false;
    SDLData* window_data = get_parent<SDLData>(locator);
    data.window_data = window_data;
    *locator->get_data() = data;
    *data_ret = locator->get_data<GameLoopData>();
    return true;
}

static bool start(SubprogramLocator* locator)
{
    GameLoopData* data = locator->get_data<GameLoopData>();
    data->running = true;
    return true;
}

static bool stop(SubprogramLocator* locator)
{
    GameLoopData* data = locator->get_data<GameLoopData>();
    data->running = false;
    return true;
}

void core::subprograms::game_loop(GameLoopData* data, SubprogramLocator* locator)
{
    SDL_Event event;
    SDL_Renderer* renderer = data->window_data->renderer;

    while (data->running)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                locator->stop("game_loop");
                return;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, 100, 100, 200, 200);
        SDL_RenderPresent(renderer);
    }
}

void core::subprograms::install_game_loop_subprogram(SubprogramLocator &locator)
{
    SubprogramInfo subprogram;
    subprogram.set_name(GAME_LOOP_SUBPROGRAM_NAME);

    subprogram.add_dependency(SDL_SUBPROGRAM_NAME);

    AnyArgsFunction<bool> init_func = &init;
    AnyArgsFunction<bool> start_func = &start;
    AnyArgsFunction<bool> stop_func = &stop;

    subprogram.set_func(SubprogramFuncNames::INIT, init_func);
    subprogram.set_func(SubprogramFuncNames::START, start_func);
    subprogram.set_func(SubprogramFuncNames::STOP, stop_func);
    
    locator.add(subprogram);
}


template <>
GameLoopData* core::subprograms::get_parent<GameLoopData>(SubprogramLocator* locator)
{
    return locator->get_parent_data<GameLoopData>(GAME_LOOP_SUBPROGRAM_NAME);
}

template <>
GameLoopData* core::subprograms::get_parent<GameLoopData>(SubprogramLocator::ParentsData* parents_data)
{
    return SubprogramLocator::get_parent_data<GameLoopData>(parents_data, GAME_LOOP_SUBPROGRAM_NAME);
}