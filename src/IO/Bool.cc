#include "Bool.hh"

namespace io
{
    Bool::operator bool() const
    {
        return b;
    }

    auto operator<<(std::ostream &os, const Bool &b) -> std::ostream &
    {
        return os << b.b;
    }

} // namespace io