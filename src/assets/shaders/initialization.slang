import utils;

[[vk::binding(0, 0)]]
cbuffer UniformBuffer {
    Camera camera;
    float4 minXYZ;
    float4 maxXYZ;
    uint32_t frameCount;
};

[[vk::binding(1, 0)]]
[[vk::image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[[vk::binding(2, 0)]]
[[vk::image_format("r32f")]]
Texture3D<float> densityImageREAD;

[[vk::binding(3, 0)]]
[[vk::image_format("r32f")]]
WTexture3D<float> densityImageWRITE;

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

// do some 3d perlin noise
// alright my next step is to turn this into 3d smoke. that is, give it some velocity and pressure

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
    int3 coord = int3(dispatchThreadID);
    int3 dims;
    densityImageWRITE.GetDimensions(dims.x, dims.y, dims.z);
    if (any(coord >= dims)) return;

    // initialize density
    // float3 p = coord - 0.5 * float3(dims);
    // if (length(p) < 80) densityImageWRITE.Store(coord, 1);
    // else densityImageWRITE.Store(coord, 0);
    // densityImageWRITE.Store(coord, noise3D(int3(coord)));

    // initialize velocity
    int width = dims.x;
    int length = dims.y;
    int height = dims.z;

    uint seed = hashSeed(uint3(dispatchThreadID.xy, uint(frameCount)));

    float k = 100;

    if (coord.x == 0) vxWRITE.Store(coord, 0);
    else vxWRITE.Store(coord, k * (2 * randf(seed) - 1));
    if (coord.x == width - 1) vxWRITE.Store(coord + int3(1,0,0), 0);

    if (coord.y == 0) vyWRITE.Store(coord, 0);
    else vyWRITE.Store(coord, k * (2 * randf(seed) - 1));
    if (coord.y == length - 1) vyWRITE.Store(coord + int3(0,1,0), 0);

    if (coord.z == 0) vzWRITE.Store(coord, 0);
    else vzWRITE.Store(coord, k * (2 * randf(seed) - 1));
    if (coord.z == height - 1) vzWRITE.Store(coord + int3(0,0,1), 0);

    // initialize solid map
    if (any(coord == int3(0)) || any(coord == dims - int3(1))) {
        solidMap.Store(coord, 1);
    }
    else {
        solidMap.Store(coord, 0);
    }

    pressureMapWRITE.Store(coord, 0);
}
