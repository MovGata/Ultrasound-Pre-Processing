#include "Mindray.hh"

#include <istream>
#include <iostream>
#include <string_view>

#include <SDL2/SDL_rwops.h>

#include "../SDL2/RWOpsStream.hh"

namespace io
{

    Mindray::Mindray(/* args */)
    {
    }

    Mindray::~Mindray()
    {
    }

    bool Mindray::load(const char *vmTxt, const char *vmBin, const char *cp)
    {

        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmTxtOps(nullptr, SDL_RWclose);
        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmBinOps(nullptr, SDL_RWclose);
        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> cpOps(nullptr, SDL_RWclose);
        io::RWOpsStream vmTxtRWStream;
        io::RWOpsStream vmBinRWStream;
        io::RWOpsStream cpRWStream;

        vmTxtOps.reset(SDL_RWFromFile(vmTxt, "r"));
        if (vmTxtOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray virtual machine file '" << vmTxt << "', error: " << SDL_GetError();
            return false;
        }
        vmTxtRWStream = RWOpsStream(vmTxtOps.get());

        std::istream vmTxtIs(&vmTxtRWStream);
        std::string s;

        int i = 0;
        std::cout << "VM TXT STRINGS" << std::endl;
        std::vector<std::reference_wrapper<InfoStore>> isDepth;
        isDepth.push_back(is);

        while (vmTxtIs >> s)
        {
            std::cout << i++ << ": " << s << std::endl;
            std::string_view sv(s);

            if (sv.starts_with("DATA_TREE_BEGIN"))
            {
                isDepth.push_back(vmTxtStore.load(std::string(sv.substr(17)), InfoStore()));
            }
            else if (sv.starts_with("DATA_TREE_END"))
            {
                isDepth.pop_back();
            }
            else
            {
                // Process data.
            }
            
        }

        vmBinOps.reset(SDL_RWFromFile(vmBin, "r"));
        if (vmBinOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray virtual machine file '" << vmBin << "', error: " << SDL_GetError();
            return false;
        }
        vmBinRWStream = RWOpsStream(vmBinOps.get());

        std::istream vmBinIs(&vmBinRWStream);

        i = 0;
        std::cout << "VM BIN STRINGS" << std::endl;
        while (vmBinIs >> s)
        {
            std::cout << i++ << ": " << s << std::endl;
        }

        cpOps.reset(SDL_RWFromFile(cp, "r"));
        if (cpOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray cine partition file '" << cp << "', error: " << SDL_GetError();
            return false;
        }
        cpRWStream = RWOpsStream(cpOps.get());

        return true;
    }

} // namespace io