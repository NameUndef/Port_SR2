#ifndef INCLUDE_ECS_WORKER_SUBPROGRAM_HPP_
#define INCLUDE_ECS_WORKER_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"

namespace core::subprograms {

    inline const char* ECS_WORKER_SUBPROGRAM_NAME = "ECS_worker";

    void install_ecs_worker_subprogram(SubprogramLocator& locator);

}

#endif