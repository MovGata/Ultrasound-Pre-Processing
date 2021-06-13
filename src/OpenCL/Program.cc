#include "Program.hh"

#include <string>
#include <string_view>
#include <fstream>

#include "Source.hh"

Program::Program(cl::Context context, Source src) : program(context, src), name(std::string_view(src.name).substr(src.name.find_last_of('/') + 1, src.name.find_last_of('.')))
{
    try
    {
        program.build("-cl-kernel-arg-info");
        std::vector<cl::Kernel> kernelVec;
        program.createKernels(&kernelVec);
        for (auto &&k : kernelVec)
        {
            kernels.emplace(k.getInfo<CL_KERNEL_FUNCTION_NAME>(), std::move(k));
        }
    }
    catch(const cl::BuildError& e)
    {
        std::cerr << e.err() << ": " << e.what() << '\n';
        std::fstream fout;
        fout.open(std::string("./build/logs/") + name + std::string(".txt"), std::fstream::out | std::fstream::trunc);
        for (auto &str : e.getBuildLog())
        {
            fout << str.second << '\n';
        }
    }
}

Program::~Program()
{
}

Kernel &Program::at(std::string str)
{
    return kernels.at(str);
}

// cl::Program *Program::operator->()
// {
//     return &program;
// }
