#ifndef INCLUDE_CONTROLLER_SUBPROGRAM_HPP_
#define INCLUDE_CONTROLLER_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include "base_game/turn_based_rpg/model_subprogram.hpp"
#include "base_game/turn_based_rpg/view_subprogram.hpp"

namespace game::subprograms {

inline const char* CONTROLLER_SUBPROGRAM_NAME = "controller";

void install_controller_subprogram(core::SubprogramLocator& locator);

}

#endif  // INCLUDE_CONTROLLER_SUBPROGRAM_HPP_