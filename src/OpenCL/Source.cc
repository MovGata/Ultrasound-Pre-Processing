#include "Source.hh"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace opencl
{
    


Source::Source(const std::string &url) : name(url)
{
    std::ifstream f(url);
    std::string s(std::istreambuf_iterator<char>(f), (std::istreambuf_iterator<char>()));
    src = cl::Program::Sources(1, {s.c_str(), s.length()});
}

// Source::Source(std::initializer_list<char *> urls)
// {
//     for (const char *url : urls)
//     {
//         std::ifstream f(url);
//         std::string s(std::istreambuf_iterator<char>(f), (std::istreambuf_iterator<char>()));
//         src.emplace_back(s.c_str(), s.length());
//     }
// }

Source::~Source()
{
}

Source::operator cl::Program::Sources()
{
    return src;
}

} // namespace opencl