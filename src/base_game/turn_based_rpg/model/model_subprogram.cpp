#include "model_subprogram.hpp"
#include "core/ecs_world_subprogram.hpp"
#include "game_time.hpp"

using namespace core;
using namespace core::subprograms;
using namespace game;
using namespace game::ecs;

static bool init(SubprogramLocator* locator)
{
    ECSData* ecs = get_parent<ECSData>(locator);

    // register components
    ecs->world.set<GameTime>(GameTime{});

    // register entities

    // register systems
    ecs->world.system<GameTime>()
        .kind(flecs::OnUpdate)
        .each(update_game_time);

    return true;
}

static bool deinit(SubprogramLocator* locator)
{
    //ECSData* ecs = get_parent<ECSData>(locator);

    return true;
}

void game::subprograms::install_model_subprogram(core::SubprogramLocator& locator)
{
    SubprogramInfo info;
    info.set_name(MODEL_SUBPROGRAM_NAME);
    info.add_dependency(ECS_WORLD_SUBPROGRAM_NAME);

    info.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>(init));
    info.set_func(SubprogramFuncNames::DEINIT, make_any_args_func<SubprogramLocator*>(deinit));

    locator.add(info);
}
