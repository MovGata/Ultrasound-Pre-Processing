#include "RWOpsStream.hh"
#include <cstring>
#include <iostream>
#include <fstream>

namespace io
{

RWOpsStream::RWOpsStream(SDL_RWops *p) : sbuf_(p), rbuffer_()
{
    setg(0, 0, 0);
    setp(wbuffer_.data(), wbuffer_.data() + blockSize);
}

RWOpsStream::~RWOpsStream()
{
    sync();
}

std::streambuf::int_type RWOpsStream::underflow()
{
    size_t read = SDL_RWread(sbuf_, rbuffer_.data(), 1, blockSize);// sbuf_->sgetn(rbuffer_.data(), blockSize);
    if (!read)
        return traits_type::eof();
    setg(rbuffer_.data(), rbuffer_.data(), rbuffer_.data() + read);
    ++buffersRead;
    return traits_type::to_int_type(*gptr());
}

std::streambuf::int_type RWOpsStream::overflow(std::streambuf::int_type value)
{
    size_t dist = pptr() - pbase();
    if (dist)
    {
        size_t written = SDL_RWwrite(sbuf_, wbuffer_.data(), 1, dist);
        if (written != dist)
            return traits_type::eof();
    }
    setp(wbuffer_.data(), wbuffer_.data() + blockSize);
    if (!traits_type::eq_int_type(value, traits_type::eof()))
        sputc(traits_type::to_char_type(value));
    return traits_type::not_eof(value);
}

int RWOpsStream::sync()
{
    std::streambuf::int_type result = this->overflow(traits_type::eof());
    size_t dist = gptr() - eback(); // Characters that have been read
    setg(0, 0, 0); // Force underflow next time which updates and fixes.

    // Move fp back to reread unused buffered data.
    if (SDL_RWtell(sbuf_) != 0)
        SDL_RWseek(sbuf_, dist + (buffersRead-1)*blockSize, RW_SEEK_SET);

    return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
}

} // namespace io