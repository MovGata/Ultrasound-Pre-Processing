// #define stepLim 500
// #define td 0.01f

int rayHitBBox(float4 rayOrg, float4 rayDir, float4 bbMin, float4 bbMax, float *nPlane, float *fPlane)
{
    // Ray intersections with BBox.
    float4 invRay = (float4)(1.0f, 1.0f, 1.0f, 1.0f) / rayDir;
    float4 posInts = invRay * (bbMax - rayOrg);
    float4 negInts = invRay * (bbMin - rayOrg);

    float4 maxInts = max(posInts, negInts);
    float4 minInts = min(posInts, negInts);

    // Only two intersections will occur.
    float maxInt = min(min(maxInts.x, maxInts.y), min(maxInts.x, maxInts.z));
    float minInt = max(max(minInts.x, minInts.y), max(minInts.x, minInts.z));

    *nPlane = minInt;
    *fPlane = maxInt;

    return maxInt > minInt;
}

kernel void render(
    uint w_out, uint l_out, global uint *output,
    uint depth, uint length, uint width, global uchar4 *data,
    constant float *invMVTransposed)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    float u = (x / (float)w_out) * 2.0f - 1.0f;
    float v = (y / (float)l_out) * 2.0f - 1.0f;

    float4 bbMin = (float4)(-1.0f, -1.0f, -1.0f, 1.0f);
    float4 bbMax = (float4)(1.0f, 1.0f, 1.0f, 1.0f);

    // Eye ray in MV space.
    float4 eyerayOrg = (float4)(invMVTransposed[3], invMVTransposed[7], invMVTransposed[11], 1.0f);
    float4 eyerayDir;

    float4 acc = normalize(((float4)(u, v, -2.0f, 0.0f)));
    eyerayDir.x = dot(acc, ((float4)(invMVTransposed[0], invMVTransposed[1], invMVTransposed[2], invMVTransposed[3])));
    eyerayDir.y = dot(acc, ((float4)(invMVTransposed[4], invMVTransposed[5], invMVTransposed[6], invMVTransposed[7])));
    eyerayDir.z = dot(acc, ((float4)(invMVTransposed[8], invMVTransposed[9], invMVTransposed[10], invMVTransposed[11])));
    eyerayDir.w = 0.0f;

    // Find intersection with BBox
    float nPlane, fPlane;
    if (!rayHitBBox(eyerayOrg, eyerayDir, bbMin, bbMax, &nPlane, &fPlane))
    {
        // Output black pixel
        output[x + y * w_out] = 0;
        return;
    }

    // Clamp nplane minimum to 0
    nPlane *= sign(nPlane);

    acc = (float4)(1.0f, 1.0f, 1.0f, 0.0f);
    output[(y * w_out) + x] = 0;

    float t = fPlane;
    float4 scale = (float4)(depth, length, width, 1.0f);

    // Raymarch back to front
    float n = 1.0f;
    uint stepLim = convert_uint(native_sqrt(convert_float(depth * depth + length * length + width * width + 1)))/4;
    float td = (fPlane - nPlane) / stepLim;
    for (uint i = 0; i < stepLim; ++i)
    {
        float4 pos = eyerayOrg + eyerayDir * t;
        t -= td;

        pos = (pos * 0.5f + 0.5f) * scale;
        uint4 iPos = clamp(convert_uint4_sat(pos), 0, convert_uint4_sat(scale - 1.0f));

        uchar4 sample = data[iPos.x + iPos.y * depth + iPos.z * length * depth];

        if ((sample.x | sample.y | sample.z | sample.w) == 0)
        {
            continue;
        }

        float4 sampleF = native_divide(convert_float4(sample), 255.0f);

        // acc = mix(acc, sampleF, 1.0f / (i + 1));
        

        // if (sample.x == sample.y && sample.x == sample.z)
        // {
        //     acc.w = mix(acc.w, sampleF.w, 1.0f / n);
        //     n += 1.0f;
        // }
        // else
        // {
            sampleF.xyz *= sampleF.w;
            acc.xyz = acc.xyz*(1.0f - sampleF.w) + sampleF.xyz;
            acc.w = acc.w*(1.0f - sampleF.w) + sampleF.w;
        // }

        
        // n += 1.0f;

        // acc = mix(acc, sampleF, sampleF.w);

        // acc = mix(acc, sampleF, sampleF.w); // Interesting solid approach

        if (t < nPlane)
        {
            break;
        }
    }

    // Write to output buffer
    uchar4 ut = convert_uchar4_sat(acc * 255.0f);
    output[(y * w_out) + x] = ((uint)(ut.x) << 24) | ((uint)(ut.y) << 16) | (((uint)(ut.z)) << 8) | (uint)(ut.w);
}
