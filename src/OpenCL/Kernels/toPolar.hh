#ifndef OPENCL_TOPOLAR_HH
#define OPENCL_TOPOLAR_HH

#include <cmath>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../Filter.hh"
#include "../Kernel.hh"
#include "../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"

namespace opencl
{
    class ToPolar : public Filter
    {
    private:
        std::shared_ptr<opencl::Kernel> kernel;
        cl_uint inlength;
        cl_uint inwidth;
        cl_uint indepth;
        cl::Buffer inBuffer;

    public:
        cl::Context context;
        cl::CommandQueue queue;

        const std::string in = "3D";
        const std::string out = "3D";

        ToPolar(const cl::Context &c, const cl::CommandQueue &q, const std::shared_ptr<opencl::Kernel> &ptr);
        ~ToPolar() = default;

        void input(const std::weak_ptr<data::Volume> &wv);
        void execute();
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace opencl

#endif