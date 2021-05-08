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
public:
    Mindray(/* args */);
    ~Mindray();
    
    std::vector<std::byte> data;
    InfoStore is;
    InfoStore vmTxtStore;

    bool load(const char *vm_txt, const char *vm_bin, const char *cp);

};

} // namespace io

#endif