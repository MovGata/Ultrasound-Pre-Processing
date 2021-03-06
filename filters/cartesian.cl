kernel void toSpherical(
    uint inDepth, uint inLength, uint inWidth, global uint *input,
    uint outDepth, uint outLength, uint outWidth, global uint *output,
    float ratio, float angleDelta) // ratio is the ratio of empty space to depth
{
    uint x = get_global_id(0); // Depth
    uint y = get_global_id(1); // Length
    uint z = get_global_id(2); // Width


    float r = convert_float(inDepth) * ratio;

    float halfAngle = angleDelta / 2.0f;
    float voff =  r + convert_float(inDepth) - convert_float(outDepth);

    if (x == 0 && y == 0 && z == 0)
    {
        printf("%f\n", voff);
    }

    float3 centrepoint = (float3)(0.0f, convert_float(outLength) / 2, convert_float(outWidth) / 2);
    float3 pos = (float3)(convert_float(x)+voff, convert_float(y), convert_float(z));
    pos = pos - centrepoint;

    float lAngle = atan2(pos.y, pos.x);
    float wAngle = atan2(pos.z, pos.x);

    float R = length(pos);

    if (R < r || R > r + inDepth - 1 || fabs(lAngle) > halfAngle || fabs(wAngle) > halfAngle)
    {
        output[x + y * outDepth + z * outDepth * outLength] = 0; // Inside empty space
        return;
    }

    uint x_in = clamp(R - r, 0.0f, inDepth - 1.0f);
    uint y_in = clamp((lAngle / halfAngle / 2.0f + 0.5f) * (inLength - 1.0f), 0.0f, inLength - 1.0f);
    uint z_in = clamp((wAngle / halfAngle / 2.0f + 0.5f) * (inWidth - 1.0f), 0.0f, inWidth - 1.0f);

    output[x + y * outDepth + z * outDepth * outLength] = input[x_in + y_in * inDepth + z_in * inDepth * inLength];
}

kernel void toCartesian(
    uint inDepth, uint inLength, uint inWidth, global uint *input,
    uint outDepth, uint outLength, uint outWidth, global uint *output,
    float ratio, float angleDelta)
{
    uint x = get_global_id(0); // Depth
    uint y = get_global_id(1); // Length
    uint z = get_global_id(2); // Width

    float3 centrepoint = (float3)(0.0f, convert_float(outLength) / 2, convert_float(outWidth) / 2);
    float3 pos = (float3)(convert_float(x), convert_float(y), convert_float(z));
    pos = pos - centrepoint;

    float angleY = pos.y / convert_float(outLength / 2) * angleDelta / 2.0f;
    float angleZ = pos.z / convert_float(outLength / 2) * angleDelta / 2.0f;

    float r = convert_float(inDepth) * (ratio / (1.0f + ratio));

    pos.x *= convert_float(inDepth - r) / convert_float(outDepth);
    pos.y *= convert_float(inLength) / convert_float(outLength);
    pos.z *= convert_float(inWidth) / convert_float(outWidth);

    float d = r + length(pos);

    uint x_in = clamp(d * cos(angleY), 0.0f, inDepth - 1.0f);
    uint y_in = clamp(d * sin(angleY), 0.0f, inLength - 1.0f);
    uint z_in = clamp(d * sin(angleZ), 0.0f, inWidth - 1.0f);

    output[x + y * outDepth + z * outDepth * outLength] = input[x_in + y_in * inDepth + z_in * inDepth * inLength];
}