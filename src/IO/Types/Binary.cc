#include "Binary.hh"

#include <SDL2/SDL_rwops.h>

#include "../SDL2/RWOpsStream.hh"

namespace io
{

    Binary::Binary(const cl::CommandQueue &cq) : cQueue(cq)
    {
        Filter::input = std::bind(input, this, std::placeholders::_1);
        Filter::execute = std::bind(execute, this);
        Filter::getOptions = std::bind(getOptions, this);
    }

    void Binary::input(const std::weak_ptr<data::Volume> &wv)
    {
        inVolume = wv;
        std::shared_ptr<data::Volume> sptr = wv.lock();
        if (sptr)
        {
            sptr->loadFromCl(cQueue);
        }
    }

    void Binary::execute()
    {
        SDL_RWops *outFile = SDL_RWFromFile("./out.bin", "wb");
        std::shared_ptr<data::Volume> sptr = inVolume.lock();
        
        SDL_RWwrite(outFile, sptr->raw.data(), sptr->raw.size(), 1);
        SDL_RWclose(outFile);
    }

    std::shared_ptr<gui::Tree> Binary::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace io
