#include "EventManager.hh"

class Volume;

namespace gui
{
    class Rectangle;
    class Window;
} // namespace gui

namespace events
{
    // template<typename F, typename... Args>
    // requires std::invocable<F, const SDL_Event &, Args &...>
    // void EventManager::addCallback(Uint32 e, F f, Args &... args)
    // {
        
    // }

    void EventManager::clearCallback(Uint32 e)
    {
        events.erase(e);
    }
    
    void EventManager::clearCallbacks()
    {
        events.clear();
    }

    bool EventManager::process(const SDL_Event &e)
    {
        auto itr = events.equal_range(e.type);
        if (itr.first == itr.second)
        {
            return false;
        }

        for (auto i = itr.first; i != itr.second; ++i)
        {
            i->second(e);
        }

        return true;
    }
} // namespace events
