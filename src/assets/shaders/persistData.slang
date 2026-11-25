import utils;

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
    int3 coord = int3(dispatchThreadID);
    int3 dims;
    densityImageWRITE.GetDimensions(dims.x, dims.y, dims.z);
    if (any(coord >= dims)) return;

    densityImageWRITE[coord] = densityImageREAD[coord];
    vxWRITE[coord] = vxREAD[coord];
    vyWRITE[coord] = vyREAD[coord];
    vzWRITE[coord] = vzREAD[coord];
    pressureMapWRITE[coord] = pressureMapREAD[coord];

    if (coord.x == dims.x - 1) vxWRITE[coord + int3(1, 0, 0)] = vxREAD[coord + int3(1, 0, 0)];
    if (coord.y == dims.y - 1) vyWRITE[coord + int3(0, 1, 0)] = vyREAD[coord + int3(0, 1, 0)];
    if (coord.z == dims.z - 1) vzWRITE[coord + int3(0, 0, 1)] = vzREAD[coord + int3(0, 0, 1)];

    float3 m = minXYZ.xyz;
    float3 M = maxXYZ.xyz;

    // persist velocity
    float h = M.z - m.z;
    float3 low = float3(m.x * 0.25, m.y * 0.25, m.z + 2);
    float3 high = float3(M.x * 0.25, M.y * 0.25, m.z + 0.5 * h);
    float3 p = coord - 0.5 * float3(dims);

    if (all(p > low) && all(p < high)) vzWRITE[coord] = 500;

    low = float3(m.x * 0.25, m.y * 0.25, m.z + 0.5 * h);
    high = float3(M.x * 0.25, M.y * 0.25, M.z - 2);

    if (all(p > low) && all(p < high)) vzWRITE[coord] = -500;

    low = float3(m.x * 0.95, m.y * 0.25, m.z + 0.45 * h);
    high = float3(M.x * 0.95, M.y * 0.25, m.z + 0.65 * h);

    if (all(p > low) && all(p < high)) vxWRITE[coord] = (p.x < (low + high).x * 0.5) ? -500 : -500;

    low = float3(m.x * 0.25, m.y * 0.95, m.z + 0.45 * h);
    high = float3(M.x * 0.25, M.y * 0.95, m.z + 0.65 * h);

    // if (all(p > low) && all(p < high)) vyWRITE[coord] = (p.y < (low + high).y * 0.5) ? -500 : 500;

    // persist density
    low = float3(m.x * 0.25, m.y * 0.25, m.z + 2);
    high = float3(M.x * 0.25, M.y * 0.25, m.z + 30);

    float3 c1 = float3(1, 1, 0);
    float3 c2 = float3(1, 1, 1);
    float3 c3 = float3(1, 0, 1);
    float3 c4 = float3(0, 1, 1);
    
    float d = 10;

    if (all(p > low) && all(p < high)) {
        densityImageWRITE[coord] = p.x > 0.5 * (low + high).x ? float4(c1, d) : float4(c2, d);
    }

    low = float3(m.x * 0.25, m.y * 0.25, M.z - 30);
    high = float3(M.x * 0.25, M.y * 0.25, M.z - 2);

    if (all(p > low) && all(p < high)) {
        densityImageWRITE[coord] = p.x < 0.5 * (low + high).x ? float4(c3, d) : float4(c4, d);
    }
}