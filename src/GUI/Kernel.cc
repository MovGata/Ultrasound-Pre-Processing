#include "Kernel.hh"
#include "Dropzone.hh"
#include "Renderer.hh"

namespace gui
{

    KernelBase::KernelBase(std::shared_ptr<Texture> &&tptr) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), outLine({0.0f, 0.0f, 1.0f, 1.0f}), title(0.0f, 0.0f, static_cast<float>(tptr->textureW), static_cast<float>(tptr->textureH), std::forward<std::shared_ptr<Texture>>(tptr))
    {
    }

    void KernelBase::updateLine(float ox, float oy)
    {
        outLine.y = outNode->y + outNode->h / 2.0f - outLine.h / 2.0f;
        outLine.x = x + w;
        outLine.angle = std::atan2(oy - outLine.y, ox - outLine.x);
        outLine.h = 3.0f;
        outLine.w = std::sqrt((oy - outLine.y) * (oy - outLine.y) + (ox - outLine.x) * (ox - outLine.x));
        outLine.update();
    }

    void KernelBase::draw()
    {
        Rectangle::draw();
        title.draw();
        renderButton->draw();
        inNode->draw();
        outNode->draw();
        options->draw();

        if (!outLine.hidden)
        {
            outLine.draw();
        }
    }

    Kernel::Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr) : KernelBase(std::forward<std::shared_ptr<Texture>>(tptr)), filter(std::forward<std::shared_ptr<opencl::Filter>>(f)), eventManager(std::make_shared<events::EventManager>())
    {
        texture->fill({0x5C, 0x5C, 0x5C, 0xFF});

        inNode = (Button<Rectangle>::build("IN"));
        outNode = (Button<Rectangle>::build("-->"));
        renderButton = (Button<Rectangle>::build("+"));

        w = std::max(w, title.w + renderButton->w + 2.0f);
        h = title.h * 3.0f;

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        x = static_cast<float>(mx) - w / 2.0f;
        y = static_cast<float>(my) - h / 2.0f;

        title.update(x + w / 2.0f - title.w / 2.0f - renderButton->w / 2.0f - 1.0f, y);
        renderButton->update(title.x + title.w + 2.0f, title.y);
        inNode->update(x + 2.0f, y + h / 2.0f);
        outNode->update(x + w - 2.0f - outNode->w, y + h / 2.0f);

        outLine.hidden = true;
        outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});

        options = TreeType::build("OPTIONS");
        options->update(x, y + h);

        auto test = Slider::build(0.0f, 0.0f, w, 5.0f);
        options->addLeaf(std::move(test));

        h = h + options->h;

        Rectangle::update();
    }

    void Kernel::execute()
    {
        if (inLink)
            arm();

        filter->execute();

        if (outLink)
            fire();
    }

    void Kernel::update(float dy)
    {
        y += dy;
        Rectangle::update();
        inNode->y += dy;
        outNode->y += dy;
        outLine.y += dy;
        title.y += dy;
        renderButton->y += dy;
        options->y += dy;

        inNode->update();
        outNode->update();
        outLine.update();
        title.update();
        renderButton->update();
        options->update();
    }

    std::shared_ptr<Renderer<Rectangle>> Kernel::buildRenderer(std::vector<std::shared_ptr<Renderer<Rectangle>>> &wr)
    {
        return Renderer<Rectangle>::build(wr, {0.0f, 0.0f, 1.0f, 1.0f, std::make_shared<gui::Texture>(512, 512)}, std::shared_ptr(filter->volume));
    }

} // namespace gui