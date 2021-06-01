kernel void test(
    uint w_out, uint l_out, global uint *output,
    uint width, uint depth, uint length, global uchar4 *data,
    constant float *invMVTransposed)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    output[x + y * w_out] = (uint)0xFF | (((uint)255 - clamp((y + x) >> 2, (uint)0, (uint)255)) << 16) | (clamp(y, (uint)0, (uint)255) << 8) | (clamp(x, (uint)0, (uint)255) << 24);
}