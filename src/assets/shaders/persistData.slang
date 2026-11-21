import utils;

[[vk::binding(0, 0)]]
cbuffer UniformBuffer {
    Camera camera;
    float4 minXYZ;
    float4 maxXYZ;
    uint32_t frameCount;
    float g;
    float dt;
    float weightSOR;
    float density;
};

[[vk::binding(1, 0)]]
[[vk::image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[[vk::binding(2, 0)]]
[[vk::image_format("r32f")]]
Texture3D<float> densityImageREAD;

[[vk::binding(3, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> densityImageWRITE;

[[vk::binding(4, 0)]]
[[vk::image_format("r32f")]]
Texture3D<float> vxREAD;

[[vk::binding(5, 0)]]
[[vk::image_format("r32f")]]
Texture3D<float> vyREAD;

[[vk::binding(6, 0)]]
[[vk::image_format("r32f")]]
Texture3D<float> vzREAD;

[[vk::binding(7, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> vxWRITE;

[[vk::binding(8, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> vyWRITE;

[[vk::binding(9, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> vzWRITE;

[[vk::binding(10, 0)]]
SamplerState linearSampler;

[[vk::binding(11, 0)]]
[[vk::image_format("r8ui")]]
RWTexture3D<uint> solidMap;

[[vk::binding(12, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> pressureMapREAD;

[[vk::binding(13, 0)]]
[[vk::image_format("r32f")]]
RWTexture3D<float> pressureMapWRITE;

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

    if (coord.x == dims.x) vxWRITE[coord + int3(1, 0, 0)] = vxREAD[coord + int3(1, 0, 0)];
    if (coord.y == dims.y) vyWRITE[coord + int3(0, 1, 0)] = vyREAD[coord + int3(0, 1, 0)];
    if (coord.z == dims.z) vzWRITE[coord + int3(0, 0, 1)] = vzREAD[coord + int3(0, 0, 1)];

    float3 m = minXYZ.xyz;
    float3 M = maxXYZ.xyz;

    // persist velocity
    float h = M.z - m.z;
    float3 low = float3(m.x * 0.15, m.y * 0.15, m.z + 2);
    float3 high = float3(M.x * 0.15, M.y * 0.15, m.z + 0.5 * h);
    float3 p = coord - 0.5 * float3(dims);

    if (all(p > low) && all(p < high)) vzWRITE[coord] = 500;

    low = float3(m.x * 0.05, m.y * 0.05, m.z + 0.75 * h);
    high = float3(M.x * 0.05, M.y * 0.05, M.z - 2);

    if (all(p > low) && all(p < high)) vzWRITE[coord] = -500;

    // persist density
    low = float3(m.x * 0.25, m.y * 0.25, m.z + 2);
    high = float3(M.x * 0.25, M.y * 0.25, m.z + 30);

    if (all(p > low) && all(p < high)) densityImageWRITE[coord] = 1;
}