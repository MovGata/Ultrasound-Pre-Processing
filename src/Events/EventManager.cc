#include "EventManager.hh"
#include "GUI.hh"

class Volume;

namespace events
{

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
