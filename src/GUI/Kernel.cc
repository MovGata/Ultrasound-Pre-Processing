#include "Kernel.hh"
#include "Dropzone.hh"
#include "Renderer.hh"

namespace gui
{

    std::vector<std::weak_ptr<Kernel>> Kernel::xKernels;

    void Kernel::executeKernels(cl_uint i)
    {
        for (auto &wptr : xKernels)
        {
            auto sptr = wptr.lock();
            sptr->filter->volume->rFrame = i;
            sptr->execute(sptr->filter->volume);
        }

    }

    KernelBase::KernelBase(std::shared_ptr<Texture> &&tptr) : Rectangle(0.0f, 0.0f, 40.0f, 40.0f), outLine({0.0f, 0.0f, 0.0f, 3.0f}), title(0.0f, 0.0f, static_cast<float>(tptr->textureW), static_cast<float>(tptr->textureH), std::forward<std::shared_ptr<Texture>>(tptr))
    {
    }

    void KernelBase::updateLine(float ox, float oy)
    {
        float newX = x + w;
        float newY = outNode->y + outNode->h / 2.0f - outLine.h / 2.0f;
        outLine.angle = std::atan2(oy - newY, ox - newX);
        outLine.update(
            newX - outLine.x,
            newY - outLine.y,
            std::sqrt((oy - outLine.y) * (oy - outLine.y) + (ox - outLine.x) * (ox - outLine.x)) - outLine.w,
            0.0f);
    }

    void KernelBase::draw()
    {
        Rectangle::upload();
        title.upload();
        renderButton->draw();
        inNode->draw();
        outNode->draw();
        options->draw();

        if (!outLine.hidden)
        {
            outLine.upload();
        }
    }

    Kernel::Kernel(std::shared_ptr<opencl::Filter> &&f, std::shared_ptr<Texture> &&tptr) : KernelBase(std::forward<std::shared_ptr<Texture>>(tptr)), filter(std::forward<std::shared_ptr<opencl::Filter>>(f))
    {
        texture->fill({0x5C, 0x5C, 0x5C, 0xFF});

        inNode = (Button::build("IN"));
        outNode = (Button::build("-->"));
        renderButton = (Button::build("+"));

        w = std::max(w, title.w + renderButton->w + 2.0f);
        h = title.h * 3.0f;

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        x = static_cast<float>(mx) - w / 2.0f;
        y = static_cast<float>(my) - h / 2.0f;

        title.update(x + w / 2.0f - title.w / 2.0f - renderButton->w / 2.0f - 1.0f - title.x,
                     y - title.y);
        renderButton->resize(
            title.x + title.w + 2.0f - renderButton->x,
            title.y - renderButton->y, 0.0f, 0.0f);
        inNode->resize(
            x + 2.0f - inNode->x,
            y + h / 2.0f - inNode->y, 0.0f, 0.0f);
        outNode->resize(
            x + w - 2.0f - outNode->w - outNode->x,
            y + h / 2.0f - outNode->y, 0.0f, 0.0f);

        outLine.hidden = true;
        outLine.texture->fill({0xD3, 0xD3, 0xD3, 0xFF});

        // options = Tree::build("OPTIONS");
        // auto test = Slider::build(0.0f, 0.0f, w, 5.0f);
        // options->addLeaf(std::move(test));
        options = filter->getOptions();


        options->resize(x - options->x,
                        y + h - options->y, 0.0f, 0.0f);

        h = h + options->h;

        Rectangle::update();

        arm = std::bind(filter->input, std::placeholders::_1);
    }

    void Kernel::execute(std::shared_ptr<data::Volume> &sp)
    {
        active = true;

        if (sp)
            arm(sp);

        filter->execute();

        if (outLink)
            fire(filter->volume);
    }

    void Kernel::update(float dx, float dy, float dw, float dh)
    {
        Rectangle::update(dx, dy, dw, dh);
        inNode->resize(dx, dy, dw, dh);
        outNode->resize(dx, dy, dw, dh);
        // outLine.update(dx, dy, dw, dh);
        title.update(dx, dy, dw, dh);
        renderButton->resize(dx, dy, dw, dh);
        options->resize(dx, dy, dw, dh);
        
        if (outLink)
            updateLine(outLink->inNode->x, outLink->inNode->y + outLink->inNode->h / 2.0f);

        if (inLink)
            inLink->updateLine(inNode->x + inNode->w, inNode->y + inNode->h / 2.0f);
    }

    std::shared_ptr<Renderer> Kernel::buildRenderer(std::vector<std::shared_ptr<Renderer>> &wr)
    {
        return Renderer::build(wr, {0.0f, 0.0f, 1.0f, 1.0f, std::make_shared<gui::Texture>(512, 512)}, std::shared_ptr(filter->volume), shared_from_this());
    }

} // namespace gui