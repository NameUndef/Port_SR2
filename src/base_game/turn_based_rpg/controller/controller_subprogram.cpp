#include "controller_subprogram.hpp"

using namespace core;
using namespace core::subprograms;

static bool init(SubprogramLocator* locator)
{
    return true;
}

void game::subprograms::install_controller_subprogram(SubprogramLocator &locator)
{
    SubprogramInfo info;
    info.set_name(CONTROLLER_SUBPROGRAM_NAME);

    info.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>(init));

    locator.add(info);
}