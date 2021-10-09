#include "Binary.hh"

#include <filesystem>
#include <iostream>

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
        volume->raw.resize(1);
        if (sptr)
        {
            volume->raw[0] = sptr->loadFromCl(cQueue);
        }
    }

    void Binary::execute()
    {
        if (!Filter::toggle)
            return;

        SDL_RWops *outFile;

        std::shared_ptr<data::Volume> sptr = inVolume.lock();
        if (sptr->rFrame == 0)
        {            
            outFile = SDL_RWFromFile("./out.bin", "wb");
        }
        else
        {
            outFile = SDL_RWFromFile("./out.bin", "ab");
        }

        SDL_RWwrite(outFile, volume->raw[0].data(), volume->raw[0].size(), 1);
        SDL_RWclose(outFile);

        if (sptr->rFrame == sptr->frames - 1)
        {
            std::string astr = "File saved to: \n\n";
            astr += std::filesystem::absolute(std::filesystem::current_path()).string() + "\\out.bin";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Binary Save Complete", astr.c_str(), nullptr);
        }

    }

    std::shared_ptr<gui::Tree> Binary::getOptions()
    {
        std::shared_ptr<gui::Tree> options = gui::Tree::build("OPTIONS");
        return options;
    }
} // namespace io
