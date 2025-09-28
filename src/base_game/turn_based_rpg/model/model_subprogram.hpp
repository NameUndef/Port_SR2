#ifndef MODEL_SUBPROGRAM_HPP_
#define MODEL_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include "core/ecs_world_subprogram.hpp"

namespace game::subprograms {

    struct ModelData {
        flecs::system system;
    };

    inline const char* MODEL_SUBPROGRAM_NAME = "model";

    void install_model_subprogram(core::SubprogramLocator& locator);

}

#endif  // MODEL_SUBPROGRAM_HPP_