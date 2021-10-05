#ifndef IO_TYPES_NIFTI1_HH
#define IO_TYPES_NIFTI1_HH

#include <array>
#include <memory>
#include <string>

#include <CL/cl2.hpp>

#include "../../OpenCL/Filter.hh"
#include "../../Concepts.hh"
#include "../../Data/Volume.hh"
#include "../../GUI/Tree.hh"

namespace io
{
    class Nifti1 : public opencl::Filter
    {
    private:
        cl::CommandQueue cQueue;
        std::weak_ptr<data::Volume> inVolume;

    public:
        Nifti1(const cl::CommandQueue &cq);
        ~Nifti1() = default;

        const std::string in = "3D";
        const std::string out = "OUT";

        bool save(const char *dir);

        void input(const std::weak_ptr<data::Volume> &wv);
        void execute();
        std::shared_ptr<gui::Tree> getOptions();
    };

} // namespace ultrasound

#endif