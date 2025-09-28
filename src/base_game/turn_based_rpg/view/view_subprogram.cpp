#include "view_subprogram.hpp"

using namespace core;
using namespace core::subprograms;

static bool init(SubprogramLocator* locator)
{
    return true;
}

void game::subprograms::install_view_subprogram(SubprogramLocator &locator)
{
    SubprogramInfo info;
    info.set_name(VIEW_SUBPROGRAM_NAME);
    info.add_dependency(SDL_SUBPROGRAM_NAME);

    info.set_func(SubprogramFuncNames::INIT, make_any_args_func<SubprogramLocator*>(init));

    locator.add(info);
}
