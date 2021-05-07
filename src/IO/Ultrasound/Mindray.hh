#ifndef IO_ULTRASOUND_MINDRAY_HH
#define IO_ULTRASOUND_MINDRAY_HH

#include <cstddef>
#include <vector>

#include "../InfoStore.hh"

namespace io
{
    
class Mindray
{
private:
    std::vector<std::byte> data;
    InfoStore is;
    InfoStore vmTxtStore;
public:
    Mindray(/* args */);
    ~Mindray();

    bool load(const char *vm_txt, const char *vm_bin, const char *cp);

};

} // namespace io

#endif