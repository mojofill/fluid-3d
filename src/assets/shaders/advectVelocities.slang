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

void semiLagrangianAdvectVelocities(in int3 coord) {
    vxWRITE.Store(coord, vxREAD[coord]);
    vyWRITE.Store(coord, vyREAD[coord]);
    vzWRITE.Store(coord, vzREAD[coord]);

    // gotta do manual trilinear interpolation of all 6 faces
    // x velocities
    if (!isSolid(coord - int3(1, 0, 0), solidMap) && !isSolid(coord, solidMap)) {
        // find center of left face
        float3 leftFace = float3(coord) - float3(0.5, 0, 0);
        float3 vel = velocityAt(leftFace, linearSampler, vxREAD, vyREAD, vzREAD);
        float3 prevPos = leftFace - vel * dt;
        vxWRITE.Store(coord, sampleVx(prevPos, linearSampler, vxREAD));
    }

    // y velocities
    if (!isSolid(coord - int3(0, 1, 0), solidMap) && !isSolid(coord, solidMap)) {
        // find center of back face
        float3 backFace = float3(coord) - float3(0, 0.5, 0);
        float3 vel = velocityAt(backFace, linearSampler, vxREAD, vyREAD, vzREAD);
        float3 prevPos = backFace - vel * dt;
        vyWRITE.Store(coord, sampleVy(prevPos, linearSampler, vyREAD));
    }

    // z velocities
    if (!isSolid(coord - int3(0, 0, 1), solidMap) && !isSolid(coord, solidMap)) {
        // find center of bottom face
        float3 bottomFace = float3(coord) - float3(0, 0, 0.5);
        float3 vel = velocityAt(bottomFace, linearSampler, vxREAD, vyREAD, vzREAD);
        float3 prevPos = bottomFace - vel * dt;
        vzWRITE.Store(coord, sampleVz(prevPos, linearSampler, vzREAD));
    }
}

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

    // do semi lagrangian advection
    semiLagrangianAdvectVelocities(coord);
}
