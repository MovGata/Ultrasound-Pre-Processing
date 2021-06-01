#include "Source.hh"

#include <fstream>
#include <string>
#include <vector>

Source::Source(const char *url)
{
    std::ifstream f(url);
    std::string s(std::istreambuf_iterator<char>(f), (std::istreambuf_iterator<char>()));
    src = cl::Program::Sources(1, {s.c_str(), s.length()});
}

Source::Source(std::initializer_list<char *> urls)
{
    for (const char *url : urls)
    {
        std::ifstream f(url);
        str.emplace_back(std::istreambuf_iterator<char>(f), (std::istreambuf_iterator<char>()));
        src.emplace_back(str.back().c_str(), str.back().length());
    }
}

Source::~Source()
{
}


Source::operator cl::Program::Sources()
{
    return src;
}