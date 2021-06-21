#include "EventManager.hh"

namespace events
{
    void EventManager::addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun)
    {
        events.insert_or_assign(e, fun);
    }

    void EventManager::clearCallback(Uint32 e)
    {
        events.erase(e);
    }

    void EventManager::process(const SDL_Event &e)
    {
        auto itr = events.find(e.type);
        if (itr != events.end())
        {
            itr->second(e);
        }
    }
} // namespace events
