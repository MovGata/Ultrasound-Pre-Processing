#ifndef IO_BOOL_HH
#define IO_BOOL_HH

namespace io
{

    typedef struct Bool
    {
        bool b;

        inline operator bool() const;
        inline friend auto operator<<(std::ostream &os, const Bool &b) -> std::ostream &;
    } Bool;

    Bool::operator bool() const
    {
        return b;
    }

    auto operator<<(std::ostream &os, const Bool &b) -> std::ostream &
    {
        return os << b.b;
    }

} // namespace io

#endif