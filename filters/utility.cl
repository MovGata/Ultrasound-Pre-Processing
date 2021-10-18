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
        float dSlice = slices[i] * convert_float(depth);
        if (fabs(convert_float(x) - dSlice) < 3.0f)
        {
            output[x + y * depth + z * depth * length] = input[x + y * depth + z * depth * length];
            return;
        }
    }

    for (uint i = 0; i < lNum; ++i)
    {
        float lSlice = slices[dNum + i] * convert_float(length);
        if (fabs(convert_float(y) - lSlice) < 3.0f)
        {
            output[x + y * depth + z * depth * length] = input[x + y * depth + z * depth * length];
            return;
        }
    }

    for (uint i = 0; i < wNum; ++i)
    {
        float wSlice = slices[dNum + lNum + i] * convert_float(width);
        if (fabs(convert_float(z) - wSlice) < 3.0f)
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
        // output[offset].w = 0xFF;
    }
    else if (input[offset].w > 0x00)
    {
        output[offset] = (uchar4)(0x00, 0x00, 0x00, 0x00);
    }
    else
    {
        output[offset] = (uchar4)(0x00, 0x00, 0x00, 0x00);
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
    output[offset].w = clamp(0xFF - output[offset].w, 0x01, 0xFF);
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

kernel void logTwo(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * length * depth;

    output[offset] = input[offset];
    output[offset].w = convert_uchar(native_log2(1.0f + convert_float(input[offset].w) / 255.0f) * 255.0f);
}

kernel void square(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * length * depth;

    output[offset] = input[offset];
    output[offset].w = convert_uchar(native_sqrt(convert_float(input[offset].w) / 255.0f) * 255.0f);
}

kernel void shrink(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint lOff = y * depth;
    uint wOff = z * depth * length;
    uint offset = x + lOff + wOff;

    uint dlim = depth - 1;
    uint llim = (length - 1) * depth;
    uint wlim = (width - 1) * (length * depth);

    output[offset] = input[offset];

    for (int i = 1; i <= 3; ++i)
    {
        if (
            input[clamp(offset - i, (uint)(0), dlim)].w == 0x00 ||
            input[clamp(offset + i, (uint)(0), dlim)].w == 0x00 ||
            input[clamp(offset - depth * i, (uint)(0), llim)].w == 0x00 ||
            input[clamp(offset + depth * i, (uint)(0), llim)].w == 0x00 ||
            input[clamp(offset - depth * length * i, (uint)(0), wlim)].w == 0x00 ||
            input[clamp(offset + depth * length * i, (uint)(0), wlim)].w == 0x00)
        {
            output[offset].w = 0x00;
            break;
        }
    }
}

kernel void fade(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * depth * length;

    output[offset] = input[offset];

    if (output[offset].x == output[offset].y && output[offset].x == output[offset].z) // Not Doppler data
    {
        output[offset].w = input[offset].w / 2;
    }
}

kernel void colourise(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output,
    float red, float green, float blue)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint offset = x + y * depth + z * depth * length;

    output[offset] = input[offset];

    output[offset].x = convert_uchar(mix(red, convert_float(input[offset].x) / 255.0f, convert_float(input[offset].w) / 255.0f) * 255.0f);
    output[offset].y = convert_uchar(mix(green, convert_float(input[offset].y) / 255.0f, convert_float(input[offset].w) / 255.0f) * 255.0f);
    output[offset].z = convert_uchar(mix(blue, convert_float(input[offset].z) / 255.0f, convert_float(input[offset].w) / 255.0f) * 255.0f);
    // output[offset].y = (1.0f - convert_float(input[offset].w)/255.0f)*green;
    // output[offset].z = (1.0f - convert_float(input[offset].w)/255.0f)*blue;
}

kernel void medianNoise(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint lOff = y * depth;
    uint wOff = z * depth * length;
    uint offset = x + lOff + wOff;

    uint size = depth * length * width;
    uint start = clamp(x - 1, (uint)0, depth) + clamp(y - 1, (uint)0, length) * depth + clamp(z - 1, (uint)0, width) * depth * length;

    uchar pixels[27];

    for (uint i = 0; i < 3; ++i)
    {
        for (uint j = 0; j < 3; ++j)
        {
            for (uint k = 0; k < 3; ++k)
            {
                pixels[k + j * 3 + i * 9] = input[min(start + k + j * depth + i * depth * length, size - 1)].w;
            }
        }
    }

    for (uint i = 0; i < 26; ++i)
    {
        for (uint j = 0; j < 26 - i; ++j)
        {
            uchar t = max(pixels[j], pixels[j + 1]);
            pixels[j] = min(pixels[j], pixels[j + 1]);
            pixels[j + 1] = t;
        }
    }

    output[offset] = pixels[13];
}

kernel void gaussian(
    uint depth, uint length, uint width, global uchar4 *input, global uchar4 *output)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint z = get_global_id(2);

    uint lOff = y * depth;
    uint wOff = z * depth * length;
    uint offset = x + lOff + wOff;

    const float weights[9] = {0.0162162162, 0.0540540541, 0.1216216216, 0.1945945946, 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162};

    float out = 3.0f * convert_float(input[offset].w) / 255.0f * weights[0];

    uint size = depth * length * width;

    uint start = clamp(x - 5, (uint)0, depth - 1) + y * depth + z * depth * length;
    float outX = out;
    for (uint i = 1; i < 9; ++i)
    {
        outX += convert_float(input[min(start + i, size - 1)].w) / 255.0f * weights[i];
    }

    start = x + clamp(y - 5, (uint)0, length - 1) * depth + z * depth * length;
    float outY = out;
    for (uint i = 1; i < 9; ++i)
    {
        outY += convert_float(input[min(start + i * depth, size - 1)].w) / 255.0f * weights[i];
    }

    start = x + y * depth + clamp(z - 5, (uint)0, width - 1) * depth * length;
    float outZ = out;
    for (uint i = 1; i < 9; ++i)
    {
        outZ += convert_float(input[min(start + i * depth * length, size - 1)].w) / 255.0f * weights[i];
    }

    output[offset] = input[offset];
    output[offset].w = convert_uchar(clamp((outZ * outY * outX), 0.0f, 1.0f) * 255.0f);
}