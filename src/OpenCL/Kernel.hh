#ifndef OPENCL_KERNEL_HH
#define OPENCL_KERNEL_HH

#include <cctype>
#include <iostream>
#include <string>

#include "Concepts.hh"

#include <CL\cl2.hpp>

namespace opencl
{

    class Kernel
    {
    private:
        cl::Kernel kernel;

    public:
        Kernel(cl::Kernel kernel);
        ~Kernel();

        operator cl::Kernel();

        cl::NDRange global;

        std::string getArg(unsigned int pos);
        bool isInput(unsigned int pos);
        bool isOutput(unsigned int pos);

        cl_uint numArgs();

        template <concepts::OpenCLType T>
        void setArg(unsigned int pos, T t, std::size_t size = 0)
        {
            if constexpr (concepts::OpenCLScalarType<T>)
            {
                std::string name = kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos);
                if (!std::isalpha(name.back()))
                {
                    std::cout << kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos) << " is not a scalar type." << std::endl;
                    return;
                }
                kernel.setArg(pos, t);
            }
            else if constexpr (concepts::OpenCLVectorType<T>)
            {
                std::string name = kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos);
                if (!std::isdigit(name.back()))
                {
                    std::cout << kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos) << " is not a vector type." << std::endl;
                    return;
                }
                kernel.setArg(pos, t);
            }
            else if constexpr (std::is_pointer_v<T>)
            {
                std::string name = kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos);
                if (name.back() != '*')
                {
                    std::cout << kernel.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>(pos) << " is not a pointer type." << std::endl;
                    return;
                }
                else if (size == 0)
                {
                    std::cout << "Pointer size < 0." << std::endl;
                    return;
                }
                kernel.setArg(pos, size, t);
            }
            else if constexpr (std::is_same_v<T, cl::Buffer>)
            {
                kernel.setArg(pos, t);
            }
        }

        void execute(cl::CommandQueue &cQueue);
    };

} // namespace opencl

#endif