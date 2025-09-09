#include "ecs_worker_subprogram.hpp"
#include "core/ecs_world_subprogram.hpp"

void core::subprograms::install_ecs_worker_subprogram(SubprogramLocator& locator)
{
    SubprogramInfo info;
    info.set_name(ECS_WORKER_SUBPROGRAM_NAME);
    info.add_dependency(ECS_WORLD_SUBPROGRAM_NAME);
    
    

    locator.add(info);
}