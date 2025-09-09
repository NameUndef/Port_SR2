#include "ecs_world_subprogram.hpp"

using namespace core;
using namespace core::subprograms;

using ECSDataPtr = std::shared_ptr<ECSData>;
static bool init(SubprogramLocator* locator)
{
    ECSDataPtr data_ptr = std::make_shared<ECSData>();
    *locator->get_data() = std::move(data_ptr);

    return true;
}

static bool deinit(SubprogramLocator* locator)
{
    locator->get_data()->reset();
    return true;
}

void core::subprograms::install_ecs_world_subprogram(SubprogramLocator& locator)
{
    SubprogramInfo info;
    info.set_name(ECS_WORLD_SUBPROGRAM_NAME);
    
    info.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>(init));
    info.set_func(SubprogramFuncNames::DEINIT, make_any_args_func<SubprogramLocator*>(deinit));

    locator.add(info);
}

template <>
ECSData* core::subprograms::get_parent<ECSData>(SubprogramLocator* locator)
{
    return locator->get_parent_data_from_sptr<ECSData>(ECS_WORLD_SUBPROGRAM_NAME);
}