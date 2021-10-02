#ifndef OPENCL_KERNELS_SLICE_HH
#define OPENCL_KERNELS_SLICE_HH

#include <array>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"
#include "../../GUI/Slider.hh"

namespace opencl
{
    class Slice : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

        cl::Buffer slices;

        std::array<std::shared_ptr<gui::Slider>, 3> slcSliders;
        std::array<float, 3> slc = {0.5f, 0.5f, 0.5f};

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Slice(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr)
        ~Slice() = default;

        void input(const std::weak_ptr<data::Volume> &wv);
        void execute();
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif