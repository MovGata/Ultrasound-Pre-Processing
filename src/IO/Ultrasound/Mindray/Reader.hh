#ifndef MINDRAY_READER_HH
#define MINDRAY_READER_HH

#include <memory>

#include <SDL2/SDL_rwops.h>

#include "../../SDL2/RWOpsStream.hh"

namespace mindray
{
    class Reader
    {
    private:
        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> vmOps;
        std::unique_ptr<SDL_RWops, decltype(&SDL_RWclose)> cpOps;
        IO::RWOpsStream vmRWStream;
        IO::RWOpsStream cpRWStream;

    public:
        Reader(const char *vm, const char *cp);
        ~Reader();
    };
    
} // namespace mindray

#endif