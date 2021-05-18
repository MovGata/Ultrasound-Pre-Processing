#ifndef IO_RWOpsStream
#define IO_RWOpsStream

#include <array>
#include <ios>
#include <streambuf>

#include <SDL2/SDL_rwops.h>

namespace io
{

class RWOpsStream : public std::streambuf
{
private:
    static constexpr int blockSize = 1024;
    size_t buffersRead = 0;

    SDL_RWops *sbuf_;
    std::array<char, blockSize> rbuffer_;
    std::array<char, blockSize> wbuffer_;

public:
    RWOpsStream() = default;
    RWOpsStream(SDL_RWops *p);
    ~RWOpsStream();

    virtual auto seekpos(pos_type pos, std::ios::openmode which = std::ios_base::in | std::ios_base::out) -> pos_type;
    virtual auto seekoff(pos_type pos, std::ios_base::seekdir dir, std::ios::openmode which = std::ios_base::in | std::ios_base::out) -> pos_type;
    virtual std::streambuf::int_type underflow();
    virtual std::streambuf::int_type overflow(int_type value = traits_type::eof());
    virtual int sync();
};

} // namespace io

#endif