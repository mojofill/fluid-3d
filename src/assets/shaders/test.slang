import utils;

[[vk::binding(0, 0)]]
cbuffer UniformBuffer {
    Camera camera;
};

[[vk::binding(1, 0)]]
[[vk::image_format("rgba32f")]]
RWTexture2D<float4> outputImage;

[[vk::binding(2, 0)]]
[[vk::image_format("r32f")]]
Sampler3D<float> densityImageREAD;

[[vk::binding(3, 0)]]
[[vk::image_format("r32f")]]
WTexture3D<float> densityImageWRITE;

// do some 3d perlin noise
// alright my next step is to turn this into 3d smoke. that is, give it some velocity and pressure

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
    int3 coord = int3(dispatchThreadID);
    int3 dimensions;
    densityImageWRITE.GetDimensions(dimensions.x, dimensions.y, dimensions.z);
    if (any(coord >= dimensions)) return;
    densityImageWRITE.Store(coord, noise3D(coord));
}
