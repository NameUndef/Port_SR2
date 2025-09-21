#include <SDL.h>
#include "core/subprogram_locator.hpp"
#include "core/sdl_subprogram.hpp"
#include "core/ecs_world_subprogram.hpp"
#include "core/thread_pool_subprogram.hpp"
#include "core/ecs_worker_subprogram.hpp"
#include <iostream>
#include <list>
#include <functional>

int run_engine(int argc, char* argv[])
{
    using namespace core;
    using namespace core::subprograms;

    SubprogramLocator locator;

    install_sdl_subprogram(locator);

    locator.start(SDL_SUBPROGRAM_NAME);

    locator.stop(SDL_SUBPROGRAM_NAME);
    return 0;
}