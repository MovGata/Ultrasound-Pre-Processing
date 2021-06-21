#ifndef EVENTS_EVENTMANAGER_HH
#define EVENTS_EVENTMANAGER_HH

#include <unordered_map>
#include <functional>

#include <SDL2/SDL.h>

namespace events
{

    /*
 * @brief Base class for controlling events. Protected constructor prohibits instantiation.
 * 
 */
    class EventManager
    {
    protected:
        std::unordered_map<Uint32, std::function<void(const SDL_Event &)>> events;

        EventManager(/* args */) = default;
        ~EventManager() = default;

    public:
        void process(const SDL_Event &e);
        void addCallback(Uint32 e, std::function<void(const SDL_Event &)> fun);
        void clearCallback(Uint32 e);
    };

} // namespace events

#endif