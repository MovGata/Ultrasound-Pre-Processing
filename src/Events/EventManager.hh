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
    template<typename T>
    class EventManager
    {
    private:
        std::unordered_map<Uint32, std::function<void(T*, const SDL_Event &)>> events;

    protected:
        EventManager() = default;
        ~EventManager() = default;

        bool process(T*, const SDL_Event &e);

    public:
        EventManager(const EventManager &e) = default;
        EventManager(EventManager &&e) = default;

        void addCallback(Uint32 e, std::function<void(T*, const SDL_Event &)> fun);
        void clearCallback(Uint32 e);
        void clearCallbacks();

    EventManager &operator=(const EventManager &) = default;
    EventManager &operator=(EventManager &&) = default;
    
    };

} // namespace events

#endif