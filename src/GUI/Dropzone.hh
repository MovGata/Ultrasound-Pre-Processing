#ifndef GUI_DROPZONE_HH
#define GUI_DROPZONE_HH

#include <unordered_map>
#include <memory>
#include <vector>

#include "Button.hh"
#include "Rectangle.hh"
#include "Texture.hh"
#include "Kernel.hh"

#include "../Events/Concepts.hh"
#include "../Events/EventManager.hh"

#include "../OpenCL/Concepts.hh"

namespace gui
{

    class Dropzone : public Rectangle, public std::enable_shared_from_this<Dropzone>
    {
    private:
        Dropzone(Rectangle &&d);

    public:
        ~Dropzone() = default;
        std::vector<std::shared_ptr<Kernel>> kernels;

        static std::shared_ptr<Dropzone> build(float wWidth, float wHeight);

        void draw() const;
        void erase(std::weak_ptr<Kernel> wptr);
    };

}

#endif