#ifndef INCLUDE_VIEW_SUBPROGRAM_HPP_
#define INCLUDE_VIEW_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include "core/sdl_subprogram.hpp"

namespace game::subprograms {

    inline const char* VIEW_SUBPROGRAM_NAME = "view";

    void install_view_subprogram(core::SubprogramLocator& locator);
}

#endif  // INCLUDE_VIEW_SUBPROGRAM_HPP_