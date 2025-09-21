#ifndef INCLUDE_ECS_WORLD_SUBPROGRAM_HPP_
#define INCLUDE_ECS_WORLD_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include <flecs.h>
#include <mutex>
#include <memory>
#include <condition_variable>

namespace core::subprograms {

inline const char* ECS_WORLD_SUBPROGRAM_NAME = "ECS_world";

struct ECSData {
    flecs::world world;
};

template <>
ECSData* get_parent<ECSData>(SubprogramLocator* locator);

template <>
ECSData* get_parent<ECSData>(SubprogramLocator::ParentsData* parents_data);

void install_ecs_world_subprogram(SubprogramLocator& locator);

}

#endif  // INCLUDE_ECS_WORLD_SUBPROGRAM_HPP_