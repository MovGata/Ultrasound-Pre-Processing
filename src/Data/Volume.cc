#include "Volume.hh"

Volume::Volume(int width, int length, int depth, const std::vector<uint8_t> &data)
{
    raw.reserve(width*depth*length);
    for (auto z = 0; z < width; ++z)
    {
        auto zyx = z * length * depth;
        for (auto y = 0; y < depth; ++y)
        {
            auto yx = y * length;
            for (auto x = 0; x < length; ++x)
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
    buffer = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, raw.size(), raw.data());
}
