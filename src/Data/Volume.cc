#include "Volume.hh"

Volume::Volume(int depth, int length, int width, const std::vector<uint8_t> &data)
{
    raw.reserve(width*depth*length);
    for (auto z = 0; z < width; ++z)
    {
        auto zyx = z * length * depth;
        for (auto y = 0; y < length; ++y)
        {
            auto yx = y * depth;
            for (auto x = 0; x < depth; ++x)
            {
                cl_uchar bnw = data.at(x + yx + zyx);
                cl_uchar4 arr = {bnw, bnw, bnw, 0xFF};
                raw.push_back(arr);
            }
        }
    }
}

Volume::~Volume()
{
}

void Volume::sendToCl(const cl::Context &context)
{
    buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(raw[0])*raw.size(), raw.data());
}
