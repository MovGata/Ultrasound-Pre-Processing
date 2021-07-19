#ifndef OPENCL_CONCEPTS_HH
#define OPENCL_CONCEPTS_HH

#include <cstring>
#include <ostream>

#include <CL/cl2.hpp>

#include "../Concepts.hh"

namespace concepts
{

    template <typename T>
    concept OpenCLScalarType = SubType<T,
                                       cl_bool,
                                       cl_char,
                                       cl_uchar,
                                       cl_short,
                                       cl_ushort,
                                       cl_int,
                                       cl_uint,
                                       cl_long,
                                       cl_ulong,
                                       cl_half,
                                       cl_float,
                                       cl_double>;
    template <typename T>
    concept OpenCLVectorType = SubType<T,
                                       cl_char2,   cl_char3,   cl_char4,   cl_char8,   cl_char16,
                                       cl_uchar2,  cl_uchar3,  cl_uchar4,  cl_uchar8,  cl_uchar16,
                                       cl_short2,  cl_short3,  cl_short4,  cl_short8,  cl_short16,
                                       cl_ushort2, cl_ushort3, cl_ushort4, cl_ushort8, cl_ushort16,
                                       cl_int2,    cl_int3,    cl_int4,    cl_int8,    cl_int16,
                                       cl_uint2,   cl_uint3,   cl_uint4,   cl_uint8,   cl_uint16,
                                       cl_long2,   cl_long3,   cl_long4,   cl_long8,   cl_long16,
                                       cl_ulong2,  cl_ulong3,  cl_ulong4,  cl_ulong8,  cl_ulong16,
                                       cl_float2,  cl_float3,  cl_float4,  cl_float8,  cl_float16,
                                       cl_double2, cl_double3, cl_double4, cl_double8, cl_double16,
                                       cl_half2,   cl_half3,   cl_half4,   cl_half8,   cl_half16>;

    template<typename T>
    concept OpenCLType = OpenCLScalarType<T> || OpenCLVectorType<T> || std::is_pointer_v<T> || std::is_same_v<T, cl::Buffer>;


    template<typename T>
    concept VolumeType = requires(T t)
    {
        {decay(t.depth)} -> std::same_as<cl_uint>;
        {decay(t.length)} -> std::same_as<cl_uint>;
        {decay(t.width)} -> std::same_as<cl_uint>;
        {decay(t.buffer)} -> std::same_as<cl::Buffer>;
        {decay(t.ratio)} -> std::same_as<cl_float>;
        {decay(t.delta)} -> std::same_as<cl_float>;
    };

} // namespace concepts

#endif