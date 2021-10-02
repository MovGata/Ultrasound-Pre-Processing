#ifndef IO_BOOL_HH
#define IO_BOOL_HH

#include <ostream>

namespace io
{

    typedef struct Bool
    {
        bool b;

        operator bool() const;
        friend auto operator<<(std::ostream &os, const Bool &b) -> std::ostream &;
    } Bool;

    auto operator<<(std::ostream &os, const Bool &b) -> std::ostream &;

} // namespace io

#endif