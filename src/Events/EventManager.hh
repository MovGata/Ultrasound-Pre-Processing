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
    private:
        std::unordered_multimap<Uint32, std::function<void(const SDL_Event &e)>> events;

    public:
        EventManager() = default;
        ~EventManager() = default;

        EventManager(const EventManager &) = default;
        EventManager(EventManager &&e) = default;
        
        bool process(const SDL_Event &);

        template <typename F, typename... Args>
        requires std::invocable<F, const SDL_Event &, Args &&...>
        void addCallback(Uint32 e, F f, Args &&...args)
        {
            events.emplace(e, std::bind(f, std::placeholders::_1, std::forward<Args>(args)...));
        }

        void clearCallback(Uint32);
        void clearCallbacks();

        EventManager &operator=(const EventManager &) = default;
        EventManager &operator=(EventManager &&) = default;
    };

} // namespace events

#endif