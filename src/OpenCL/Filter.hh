#ifndef OPENCL_FILTER
#define OPENCL_FILTER

#include <functional>
#include <memory>

#include "../GUI/Tree.hh"
#include "../Data/Volume.hh"

namespace opencl
{
    class Filter
    {
    protected:
        Filter() = default;
        ~Filter() = default;

    public:
        bool toggle = true;
        std::shared_ptr<data::Volume> volume;
        std::function<void(const std::weak_ptr<data::Volume> &)> input;
        std::function<void(void)> execute;
        std::function<std::shared_ptr<gui::Tree>(void)> getOptions;
        std::function<bool(const char *)> load = [](const char *)
        { return false; };
    };

} // namespace opencl

#endif