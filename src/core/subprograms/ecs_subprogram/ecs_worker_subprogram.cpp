#include "core/subprogram_locator.hpp"
#include "ecs_worker_subprogram.hpp"
#include "core/ecs_world_subprogram.hpp"
#include "core/worker_subprogram.hpp"

void ecs_handler(std::any* data, core::SubprogramLocator::ParentsData* parents_data)
{
    using namespace core::subprograms;

    ECSData* ecs_data = get_parent<ECSData>(parents_data);

    for (int i = 0; i < 5; i++) {
        ecs_data->world.progress();
    }
}

void core::subprograms::install_ecs_worker_subprogram(SubprogramLocator& locator)
{
    SubprogramInfo info;
    info.set_name(ECS_WORKER_SUBPROGRAM_NAME);
    info.add_dependency(ECS_WORLD_SUBPROGRAM_NAME);
    
    install_worker(info, ecs_handler, "ECS_worker_thread");

    locator.add(info);
}