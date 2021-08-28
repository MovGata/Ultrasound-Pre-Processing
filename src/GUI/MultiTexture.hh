#ifndef GUI_MULTITEXTURE_HH
#define GUI_MULTITEXTURE_HH

#include <array>
#include <memory>

#include <GL/glew.h>
#include <GL/gl.h>

#include "../Events/Concepts.hh"
#include "Texture.hh"

namespace gui
{

    template <concepts::DrawableType Drawable, std::size_t Size>
    class MultiTexture : public Drawable
    {
    private:
        std::size_t index = 0;
        std::array<std::shared_ptr<GLuint>, Size> textures;

    public:
        using Drawable::Drawable;

        void setTexture(std::size_t i, std::shared_ptr<GLuint> &&t)
        {
            textures[i] = (std::forward<std::shared_ptr<GLuint>>(t));
        }

        void nextTexture()
        {
            Drawable::texture = textures.at(index++);
            index = index % textures.size();
        }
    };

    template <concepts::DrawableType Drawable>
    class MultiTexture<Drawable, 2> : public Drawable
    {
    private:
        std::size_t index = 0;
        std::array<std::shared_ptr<Texture>, 2> textures;
        bool toggle = false;

    public:
        using Drawable::Drawable;
        using Drawable::operator=;

        MultiTexture(const Drawable &m) : Drawable(m){};
        MultiTexture(Drawable &&m) : Drawable(std::forward<Drawable>(m)){};
        
        MultiTexture &operator=(const Drawable &m){return Drawable::operator=(m);};
        MultiTexture &operator=(Drawable &&m) {return Drawable::operator=(std::move(m));};

        void setTexture(std::size_t i, std::shared_ptr<Texture> &&t)
        {
            textures.at(i) = (std::forward<std::shared_ptr<Texture>>(t));
        }

        void nextTexture()
        {
            toggle = !toggle;
            index = static_cast<std::size_t>(toggle);
            Drawable::texture = textures.at(index);
        }
    };

}

#endif