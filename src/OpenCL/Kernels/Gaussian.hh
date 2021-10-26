#ifndef OPENCL_KERNELS_GAUSSIAN_HH
#define OPENCL_KERNELS_GAUSSIAN_HH

#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Device.hh"
#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"

namespace opencl
{
    class Gaussian : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel2D;
        std::shared_ptr<opencl::Kernel> kernel3D;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        Gaussian(const Device &d);
        ~Gaussian() = default;

        void input(const std::weak_ptr<data::Volume> &wv);
        void execute();
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif