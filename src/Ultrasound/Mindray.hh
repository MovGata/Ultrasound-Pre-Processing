/*
 * @file Mindray.hh
 * @author Harry Gougousidis (harry.gougousidis@outlook.com)
 * @version 0.1
 * @date 2021-05-10
 * @brief 
 */

#ifndef IO_ULTRASOUND_MINDRAY_HH
#define IO_ULTRASOUND_MINDRAY_HH

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <CL/cl2.hpp>

#include "../IO/InfoStore.hh"
#include "../Data/Volume.hh"
#include "../OpenCL/Concepts.hh"


namespace ultrasound
{
    class Mindray : public data::Volume
    {
    private:
        // std::vector<cl_uchar4> raw;

    public:
        Mindray();
        ~Mindray();

        const std::string in = "IN";
        const std::string out = "3D";

        using vmBinInfoStore = io::InfoStore<bool, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double>;
        using vmTxtInfoStore = io::InfoStore<uint32_t, double, std::string>;
        using cpInfoStore = io::InfoStore<uint8_t, int32_t, float, std::size_t>;

        vmBinInfoStore vmBinStore;
        vmTxtInfoStore vmTxtStore;
        cpInfoStore cpStore;

        bool load(const char *dir);

        void load(const cl::Context &context);

        template<concepts::VolumeType V>
        void input([[maybe_unused]] const V &v){}
        void execute();

        void sendToCl(const cl::Context &context);

    };

} // namespace ultrasound

#endif