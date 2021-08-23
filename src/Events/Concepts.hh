#ifndef EVENTS_CONCEPTS_HH
#define EVENTS_CONCEPTS_HH

#include <concepts>
#include <memory>
#include <type_traits>

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <SDL_events.h>

#include "EventManager.hh"
#include "../GUI/Texture.hh"

namespace concepts
{

    template<typename T>
    constexpr auto decay(T t)
    {
        return std::decay_t<decltype(t)>();
    }

    template <typename T>
    concept ProcessorType = requires(T t, SDL_Event e)
    {
        {decay(t.eventManager)} -> std::same_as<std::shared_ptr<events::EventManager>>;
    };

    template <typename T>
    concept DrawableType = requires(T &t)
    {
        {decay(t.vertices[0])} -> std::same_as<float>;
        {decay(t.texcoords[0])} -> std::same_as<float>;
        {decay(t.modelview)} -> std::same_as<glm::mat4>;
        {decay(t.modelview)} -> std::same_as<glm::mat4>;
        {decay(t.vBuffer)} -> std::same_as<GLuint>;
        {decay(t.tBuffer)} -> std::same_as<GLuint>;
        {decay(t.vArray)} -> std::same_as<GLuint>;
        {decay(t.texture)} -> std::same_as<std::shared_ptr<gui::Texture>>;
        t.draw();
    };

    template <typename T>
    concept PositionableType = DrawableType<T> && requires(T t)
    {
        {decay(t.x)} -> std::same_as<float>;
        {decay(t.y)} -> std::same_as<float>;
        {decay(t.w)} -> std::same_as<float>;
        {decay(t.h)} -> std::same_as<float>;
    };

    // template <typename T>
    // concept TransformableType = DrawableType<T> && requires(T t)
    // {
    //     {decay(t.transformations)} -> std::same_as<glm::mat4>;
    // };

    template <typename T>
    concept HidableType = DrawableType<T> && requires(T t)
    {
        {decay(t.hidden)} -> std::same_as<bool>;
    };

    template <typename T>
    concept SharableType = requires(T t)
    {
        t.T::std::enable_shared_from_this::shared_from_this();
    };

    template <typename T>
    concept WeakableType = SharableType<T> && requires(T t)
    {
        t.T::std::enable_shared_from_this::weak_from_this();
    };

    template <typename T>
    concept LazyType = requires(T t)
    {
        {decay(t.modified)} -> std::same_as<bool>;
    };

    template <typename T, typename U>
    concept LinkableType = requires(T t, U u)
    {
        t.template link<U>(u);
        u.template link<T>(t);
    };

    template<typename T>
    concept ExternalType = DrawableType<T> && requires(T t, GLuint i)
    {
        t.update(i);
    };

    template <typename T>
    concept ScalableType = requires(T t)
    {
        {decay(t.scale)} -> std::same_as<glm::vec3>;
    };

    template <typename T>
    concept RotatableType = requires(T t)
    {
        {decay(t.rotation)} -> std::same_as<glm::vec3>;
    };

    template <typename T>
    concept TranslatableType = requires(T t)
    {
        {decay(t.translation)} -> std::same_as<glm::vec3>;
    };

    template<typename T>
    concept TransformableType = ScalableType<T> && RotatableType<T> && TranslatableType<T>;

    template <typename T>
    concept TogglableType = requires(T t)
    {
        {decay(t.toggle)} -> std::same_as<bool>;
    };

} // namespace concepts

#endif