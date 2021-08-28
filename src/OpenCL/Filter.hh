#ifndef OPENCL_FILTER
#define OPENCL_FILTER

#include <functional>
#include <memory>

#include "../Data/Volume.hh"

namespace opencl
{
    class Filter
    {
    protected:
        Filter() = default;
        ~Filter() = default;
        
    public:
        std::shared_ptr<data::Volume> volume;
        std::function<void(const std::weak_ptr<data::Volume> &)> input;
        std::function<void(void)> execute;
    };

} // namespace opencl

#endif