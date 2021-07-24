kernel void slice(
    uint depth, uint length, uint width, global uint *input, global uint *output,
    uint dNum, uint lNum, uint wNum, constant float *slices)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    output[x + y * depth + z * depth * length] = 0;

    for (uint i = 0; i < dNum; ++i)
    {
        uint dSlice = convert_uint(slices[i] * convert_float(depth));
        if (x == dSlice)
        {
            output[x + y * depth + z * depth * length] = input[x + y * depth + z * depth * length];
            return;
        }
    }

    for (uint i = 0; i < lNum; ++i)
    {
        uint lSlice = convert_uint(slices[dNum + i] * convert_float(length));
        if (y == lSlice)
        {
            output[x + y * depth + z * depth * length] = input[x + y * depth + z * depth * length];
            return;
        }
    }

    for (uint i = 0; i < wNum; ++i)
    {
        uint wSlice = convert_uint(slices[dNum + lNum + i] * convert_float(width));
        if (z == wSlice)
        {
            output[x + y * depth + z * depth * length] = input[x + y * depth + z * depth * length];
            return;
        }
    }
}

kernel void clamping(
    uint depth, uint length, uint width, global uint *input, global uint *output,
    float dl, float du, float ll, float lu, float wl, float wu)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * depth * length;

    output[offset] = 0;

    if (
        x > dl * depth && x < du * depth &&
        y > ll * length && y < lu * length &&
        z > wl * width && z < wu * width)
    {
        output[offset] = input[offset];
    }
}

kernel void threshold(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output, uchar val)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * length * depth;
    if (input[offset].w > val)
    {
        output[offset] = input[offset];
    }
    else
    {
        output[offset] = (uchar4)(0, 0, 0, 0);
    }
}

kernel void invert(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * length * depth;
    output[offset] = input[offset];
    output[offset].x = 0xFF - output[offset].x;
    output[offset].y = 0xFF - output[offset].y;
    output[offset].z = 0xFF - output[offset].z;
    output[offset].w = 0xFF - output[offset].w;
}

kernel void contrast(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output, uchar minim, uchar maxim)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * length * depth;

    float mdiff = convert_float(maxim - minim);
    float vdiff = convert_float(input[offset].w - minim);

    output[offset] = input[offset];
    output[offset].w = convert_uchar(clamp(vdiff / mdiff * 255.0f, 0.0f, 255.0f));
}