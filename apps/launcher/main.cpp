#include <SDL.h>
#include "core/subprogram_locator.hpp"
#include "core/sdl_subprogram.hpp"
#include "core/ecs_world_subprogram.hpp"
#include <iostream>
#include <list>
#include <functional>



int main(int argc, char* argv[])
{
    using namespace core;
    using namespace core::subprograms;

    SubprogramLocator locator;
    install_sdl_subprogram(locator);
    install_ecs_world_subprogram(locator);
    std::cout << "Start" << std::endl;
    locator.init("SDL");

    locator.remove("SDL");
    return 0;
}