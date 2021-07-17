#define stepLim 500
#define td 0.01f

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

    // float maxEdge = convert_float(max(max(depth, length), max(depth, width));)

    // float fDepth = convert_float(depth);
    // float fLength = convert_float(length);
    // float fWidth = convert_float(width);

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

    acc = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float t = fPlane;
    float4 scale = (float4)(depth - 1, length - 1, width - 1, 1.0f);

    // Raymarch back to front
    for (uint i = 0; i < stepLim; ++i)
    {
        float4 pos = eyerayOrg + eyerayDir * t;
        pos = clamp((pos * 0.5f + 0.5f) * scale, 0.0f, scale);
        uint4 iPos = clamp(convert_uint4_sat(pos), 0, convert_uint4_sat(scale));

        uchar4 sample = data[iPos.x + iPos.y * depth + iPos.z * length * depth];
        float4 sampleF = clamp(native_divide(convert_float4(sample), 255), 0.0f, 1.0f);

        acc = mix(acc, sampleF, 1.0f / (i + 1));
        // acc.xyz = acc.xyz + (1 - acc.w)*sampleF.xyz;
        // acc.w = acc.w + (1 - acc.w)*sampleF.w;

        t -= td;
        if (t < nPlane)
        {
            break;
        }
    }

    // Write to output buffer
    acc *= (float4)(1.0f-u*v, 1.0f-(v+u)/2.0f, 1.0f-(-u*v), 1.0f);
    uchar4 ut = convert_uchar4_sat(acc * 255.0f);
    output[(y * w_out) + x] = ((uint)(ut.x) << 24) | ((uint)(ut.y) << 16) | (((uint)(ut.z)) << 8) | (uint)(ut.w);
}
