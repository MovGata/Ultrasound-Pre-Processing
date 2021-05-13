#ifndef IO_CONCEPTS_HH
#define IO_CONCEPTS_HH

#include <cstring>
#include <ostream>

namespace concepts
{

    template <typename T>
    concept PrintableDepthType = requires (T x, std::ostream &os) {x.print(os, 0);};
    
} // namespace concepts


#endif