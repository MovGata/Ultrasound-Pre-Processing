#include "EventManager.hh"

class Volume;

namespace gui
{
    class Rectangle;
    class Window;
} // namespace gui

namespace events
{
    template <typename T>
    void EventManager<T>::addCallback(Uint32 e, std::function<void(T *, const SDL_Event &)> fun)
    {
        events.insert_or_assign(e, fun);
    }

    template <typename T>
    void EventManager<T>::clearCallback(Uint32 e)
    {
        events.erase(e);
    }

    template <typename T>
    void EventManager<T>::process(T *t, const SDL_Event &e)
    {
        auto itr = events.find(e.type);
        if (itr != events.end())
        {
            itr->second(t, e);
        }
    }

    template class EventManager<Volume>;
    template class EventManager<gui::Rectangle>;
    template class EventManager<gui::Window>;
} // namespace events
