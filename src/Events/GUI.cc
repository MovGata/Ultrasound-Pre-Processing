#include "GUI.hh"

namespace events
{
    const Uint32 GUI_SHOW = SDL_RegisterEvents(7);
    const Uint32 GUI_DROP = GUI_SHOW + 1;
    const Uint32 GUI_MOVE = GUI_SHOW + 2;
    const Uint32 GUI_VOLUME = GUI_SHOW + 3;
    const Uint32 GUI_DRAW = GUI_SHOW + 4;
    const Uint32 GUI_TOGGLE = GUI_SHOW + 5;
    const Uint32 GUI_REDRAW = GUI_SHOW + 6;
} // namespace events
