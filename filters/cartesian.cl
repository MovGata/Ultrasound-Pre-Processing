kernel void toSpherical(
    uint inDepth, uint inLength, uint inWidth, global uint *input,
    uint outDepth, uint outLength, uint outWidth, global uint *output,
    float ratio, float angleDelta) // ratio is the ratio of empty space to depth
{
    uint x = get_global_id(0); // Depth
    uint y = get_global_id(1); // Length
    uint z = get_global_id(2); // Width

    float3 centrepoint = (float3)(0.0f, convert_float(outLength) / 2, convert_float(outWidth) / 2);
    float3 pos = (float3)(convert_float(z), convert_float(y), convert_float(x));
    pos = pos - centrepoint;

    float3 r = (float3)(convert_float(outDepth), convert_float(outLength), convert_float(outWidth)) * ratio;

    float lAngle = 180.0f * acospi(dot(normalize(pos.yz), (float2)(0.0f, 1.0f)));
    float wAngle = 180.0f * acospi(dot(normalize(pos.xz), (float2)(0.0f, 1.0f)));

    if (fast_distance(pos, r) < 0 || lAngle > angleDelta * inLength / 2.0f || wAngle > angleDelta * inWidth / 2.0f)
    {
        output[x + y * outDepth + z * outDepth * outLength] = 0; // Inside empty space
        return;
    }

    float3 pos_sqrd = pos * pos;

    float R = native_sqrt(pos_sqrd.x + pos_sqrd.y + pos_sqrd.z);
    float theta = 180 * acospi(pos.z / R);
    float azim = 180 * atan2pi(pos.y, pos.x);

    uint x_in = clamp((R - r.x) * (convert_float(inDepth) - r.x) / convert_float(outDepth), 0.0f, inDepth-1.0f);
    uint y_in = clamp(copysign(lAngle / angleDelta, pos.y) + angleDelta * inLength / 2.0f, 0.0f, inLength-1.0f);
    uint z_in = clamp(copysign(wAngle / angleDelta, pos.y) + angleDelta * inWidth / 2.0f, 0.0f, inWidth-1.0f);

    output[x + y * outDepth + z * outDepth * outLength] = input[x_in + y_in * inDepth + z_in * inDepth * inLength];
}