#ifndef INCLUDE_GAME_LOOP_SUBPROGRAM_HPP_
#define INCLUDE_GAME_LOOP_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include "core/sdl_subprogram.hpp"

namespace core::subprograms {

    inline const char* GAME_LOOP_SUBPROGRAM_NAME = "game_loop";

    struct GameLoopData
    {
        bool running;
        SDLData* window_data;
    };

    
    template <>
    GameLoopData* get_parent<GameLoopData>(SubprogramLocator* locator);
    
    template <>
    GameLoopData* get_parent<GameLoopData>(SubprogramLocator::ParentsData* parents_data);
    
    void game_loop(GameLoopData* data, SubprogramLocator* locator);
    void install_game_loop_subprogram(SubprogramLocator& locator);
}

#endif  // INCLUDE_GAME_LOOP_SUBPROGRAM_HPP_