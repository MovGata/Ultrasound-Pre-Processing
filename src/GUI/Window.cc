// #include "Window.hh"

// #include <cmath>
// #include <iostream>
// #include <numbers>

// #include <gl/gl.h>
// #include <gl/glext.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>


// #include "../Events/GUI.hh"

// namespace gui
// {
//     template <concepts::DrawableType... Drawables>
//     Window<Drawables...>::Window(unsigned int width, unsigned int height, Uint32 flags) : Window(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags) {}

//     template <concepts::DrawableType... Drawables>
//     Window<Drawables...>::Window(unsigned int xPos, unsigned int yPos, unsigned int width, unsigned int height, Uint32 flags) : window(nullptr, SDL_DestroyWindow), projection(1.0f)
//     {
//         window.reset(SDL_CreateWindow("Ultrasound Pre-Processor", xPos, yPos, width, height, flags));
//         if (window == nullptr)
//         {
//             std::cout << "SDL2 window initialisation failed, error: " << SDL_GetError() << std::endl;
//             return;
//         }

//         // addCallback(SDL_WINDOWEVENT, Window::windowEvent);
//         // addCallback(SDL_MOUSEWHEEL, Window::scrollEvent);
//         // addCallback(SDL_MOUSEBUTTONUP, Window::dragStopEvent);
//         // addCallback(SDL_MOUSEBUTTONDOWN, Window::dragStartEvent);
//         // addCallback(SDL_MOUSEMOTION, Window::dragEvent);
//         // addCallback(Rectangle::dropEventData, Window::userDrop);
//         // addCallback(Rectangle::volumeEventData, Window::userDrop);
//         // addCallback(Rectangle::moveEventData, Window::userDrop);
//         // addCallback(Rectangle::showEventData, Window::userShow);

//         glContext = SDL_GL_CreateContext(window.get());
//         if (glContext == nullptr)
//         {
//             std::cout << "OpenGL context initialisation failed, error: " << SDL_GetError() << std::endl;
//             window.release();
//             return;
//         }

//         modelview = glm::mat4(1.0f);

//         // arrow = Rectangle(0.5f, 0.0f, 0.5f, 0.005f);
//         // arrow.setBG({0xFF, 0xFF, 0xFF, 0xFF});
//         // arrow.allocTexture(1, 1);
//         // arrow.update(modelview);

//         clean();
//         redraw();
//     }

//     template <concepts::DrawableType... Drawables>
//     Window<Drawables...>::~Window()
//     {
//         SDL_GL_DeleteContext(glContext);
//     }

//     // void Window::process(const SDL_Event &e)
//     // {
//     //     em.process(*this, e);
//     // }

//     template <concepts::DrawableType... Drawables>
//     auto Window<Drawables...>::getPosition() -> std::pair<int, int>
//     {
//         std::pair<int, int> p;
//         SDL_GetWindowPosition(window.get(), &p.first, &p.second);
//         return p;
//     }

//     template <concepts::DrawableType... Drawables>
//     auto Window<Drawables...>::getSize() -> std::pair<int, int>
//     {
//         std::pair<int, int> p;
//         SDL_GetWindowSize(window.get(), &p.first, &p.second);
//         return p;
//     }

//     template <concepts::DrawableType... Drawables>
//     auto Window<Drawables...>::getID() -> Uint32
//     {
//         return SDL_GetWindowID(window.get());
//     }

//     template <concepts::DrawableType... Drawables>
//     void Window<Drawables...>::clean()
//     {
//         glClear(GL_COLOR_BUFFER_BIT);
//     }

//     template <concepts::DrawableType... Drawables>
//     void Window<Drawables...>::update()
//     {
//     }

//     template <concepts::DrawableType... Drawables>
//     void Window<Drawables...>::draw()
//     {
//         if (minimised)
//         {
//             return;
//         }

//         for (auto &&drawable : drawables)
//         {
//             std::visit(
//                 [](auto &&d)
//                 { d.draw(); },
//                 drawable);
//         }
        
//         // clean();

//         // for (const auto &rec : subRectangles)
//         // {
//         //     // rec->draw();
//         //     events::visible(events::draw<Rectangle>, (*rec));
//         // }

//         // for (const auto &rec : subRectangles)
//         // {
//         //     rec->drawChildren();
//         // }

//         // for (const auto &rec : subRectangles)
//         // {
//         //     rec->drawLinks();
//         // }

//         // drawArrow();

//         SDL_GL_SwapWindow(window.get());
//     }

//     // template<concepts::DrawableType... Drawables>
//     // void Window<Drawables...>::drawArrow()
//     // {
//     //     if (minimised)
//     //     {
//     //         return;
//     //     }

//     //     if (Rectangle::mouseArrow.has_value())
//     //     {
//     //         arrow.transformations = *mouseArrow;
//     //         events::draw(Rectangle::arrow);
//     //     }
//     // }

//     template <concepts::DrawableType... Drawables>
//     void Window<Drawables...>::redraw()
//     {
//         if (minimised)
//         {
//             return;
//         }

//         auto pair = getSize();

//         glViewport(0, 0, pair.first, pair.second);

//         if (pair.first > pair.second)
//         {
//             float aspect = static_cast<float>(pair.first) / static_cast<float>(pair.second);
//             x = -1.0f;
//             y = -1.0f / aspect;
//             w = 2.0f;
//             h = 2.0f * std::abs(y);
//         }
//         else
//         {
//             float aspect = static_cast<float>(pair.second) / static_cast<float>(pair.first);
//             x = -1.0f / aspect;
//             y = -1.0f;
//             w = 2.0f * std::abs(x);
//             h = 2.0f;
//         }

//         projection = glm::ortho(x, x + w, y, y + h, 0.0f, 1.0f);

//         glUniformMatrix4fv(projectionUni, 1, GL_FALSE, reinterpret_cast<const GLfloat *>(&projection[0]));

//         for (auto &rec : subRectangles)
//         {
//             rec->transformations = glm::scale(glm::mat4(1.0f), {std::abs(x), std::abs(y), 1.0f});
//         }
//     }

//     // template<concepts::DrawableType... Drawables>
//     // void Window<Drawables...>::setActive()
//     // {
//     //     SDL_GL_MakeCurrent(window.get(), glContext);
//     // }

//     template <concepts::DrawableType... Drawables>
//     void Window<Drawables...>::windowEvent(const SDL_Event &e)
//     {
//         switch (e.window.event)
//         {
//         case SDL_WINDOWEVENT_MINIMIZED:
//             minimised = true;
//             break;
//         case SDL_WINDOWEVENT_MAXIMIZED:
//             break;
//         case SDL_WINDOWEVENT_RESTORED:
//             minimised = false;
//             break;
//         case SDL_WINDOWEVENT_SIZE_CHANGED:
//             redraw();
//             break;
//         case SDL_WINDOWEVENT_MOVED:
//             break;
//         default:
//             break;
//         }
//     }

//     // void Window::dragStartEvent(const SDL_Event &e)
//     // {
//     //     std::pair<int, int> size;
//     //     SDL_GetWindowSize(SDL_GetWindowFromID(e.button.windowID), &size.first, &size.second);

//     //     int mx = e.button.x, my = e.button.y;
//     //     glm::vec4 mp = {0.0f, 0.0f, 0.0f, 1.0f};

//     //     mp.x = std::lerp(x, x + w, static_cast<float>(mx) / static_cast<float>(size.first));
//     //     mp.y = std::lerp(y, y + h, 1.0f - static_cast<float>(my) / static_cast<float>(size.second));

//     //     mp = modelview * mp;

//     //     for (auto &r : subRectangles)
//     //     {
//     //         if (r->contains(mp.x, mp.y) && r->hidden)
//     //         {
//     //             r->process(e);
//     //             break;
//     //         }
//     //     }
//     // }

//     // void Window::dragStopEvent(const SDL_Event &e)
//     // {
//     //     // auto size = getSize();

//     //     // int mx = e.button.x, my = e.button.y;

//     //     // float rx = std::lerp(x, x+w, static_cast<float>(mx)/static_cast<float>(size.first));
//     //     // float ry = std::lerp(y, y+h, 1.0f - static_cast<float>(my)/static_cast<float>(size.second));
//     //     // gui::Rectangle *rec = static_cast<Rectangle *>(dragObject->user.data1);

//     //     if (dragObject.has_value())
//     //     {
//     //         // if (dragObject->user.type == gui::Rectangle::dropEventData)
//     //         // {
//     //         //     for (auto &r : subRectangles)
//     //         //     {
//     //         //         if (r->containsTF(*rec))
//     //         //         {
//     //         //             r->process(dragObject.value());
//     //         //             break;
//     //         //         }
//     //         //     }
//     //         // }
//     //         // else if (dragObject->user.type == gui::Rectangle::moveEventData)
//     //         // {
//     //         //     for (auto &r : subRectangles)
//     //         //     {
//     //         //         // gui::Rectangle *rec = static_cast<Rectangle *>(dragObject->user.data1);
//     //         //         if (r->contains(*rec))
//     //         //         {
//     //         //             r->process(dragObject.value());
//     //         //             break;
//     //         //         }
//     //         //     }
//     //         // }

//     //         static_cast<Rectangle *>(dragObject->user.data1)->process(e);
//     //         dragObject.reset();
//     //     }
//     // }

//     // void Window::dragEvent(const SDL_Event &e)
//     // {
//     //     if (dragObject.has_value())
//     //     {
//     //         static_cast<gui::Rectangle *>(dragObject->user.data1)->process(e);
//     //     }
//     // }

//     // void Window::userDrop(const SDL_Event &e)
//     // {
//     //     dragObject.emplace(e);
//     // }

//     // void Window::userShow(const SDL_Event &e)
//     // {
//     //     for (auto &r : subRectangles)
//     //     {
//     //         r->process(e);
//     //     }
//     // }

// } // namespace gui
