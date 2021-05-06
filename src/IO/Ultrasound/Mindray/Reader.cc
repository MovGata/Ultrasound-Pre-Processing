#include <iostream>

#include <../../SDL2/SDL_rwops.h>

#include "Reader.hh"

namespace mindray
{
    
    Reader::Reader(const char *vm, const char *cp) : vmOps(nullptr, &SDL_RWclose), cpOps(nullptr, &SDL_RWclose)
    {
        vmOps.reset(SDL_RWFromFile(vm, "r"));
        if (vmOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray virtual machine file '" << vm << "', error: " << SDL_GetError();
            return;
        }
        vmRWStream = IO::RWOpsStream(vmOps.get());
        
        vmOps.reset(SDL_RWFromFile(cp, "r"));
        if (cpOps == nullptr)
        {
            std::cout << "SDL2 failed to open Mindray cine partition file '" << cp << "', error: " << SDL_GetError();
            return;
        }
        cpRWStream = IO::RWOpsStream(cpOps.get());
    }
    
    Reader::~Reader()
    {
    }
    

} // namespace IO
