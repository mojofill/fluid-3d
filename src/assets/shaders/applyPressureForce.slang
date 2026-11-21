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

void updateVelocities(int3 coord) {
    float k = dt / density;

    // x
    if (isSolid(coord, solidMap) || isSolid(coord - int3(1,0,0), solidMap)) {
        vxWRITE[coord] = 0;
    }
    else {
        float pressureRight = getPressure(coord, pressureMapREAD);
        float pressureLeft = getPressure(coord - int3(1,0,0), pressureMapREAD);

        vxWRITE[coord] = vxREAD[coord] - k * (pressureRight - pressureLeft);
    }

    // y
    if (isSolid(coord, solidMap) || isSolid(coord - int3(0,1,0), solidMap)) {
        vyWRITE[coord] = 0;
    }
    else {
        float pressureFront = getPressure(coord, pressureMapREAD);
        float pressureBack = getPressure(coord - int3(0,1,0), pressureMapREAD);

        vyWRITE[coord] = vyREAD[coord] - k * (pressureFront - pressureBack);
    }

    // z
    if (isSolid(coord, solidMap) || isSolid(coord - int3(0,0,1), solidMap)) {
        vzWRITE[coord] = 0;
    }
    else {
        float pressureTop = getPressure(coord, pressureMapREAD);
        float pressureBottom = getPressure(coord - int3(0, 0, 1), pressureMapREAD);

        vzWRITE[coord] = vzREAD[coord] - k * (pressureTop - pressureBottom);
    }
}

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
    int3 coord = int3(dispatchThreadID);
    int3 dims;
    pressureMapREAD.GetDimensions(dims.x, dims.y, dims.z);
    if (any(coord >= dims)) return;

    densityImageWRITE[coord] = densityImageREAD[coord];
    vxWRITE[coord] = vxREAD[coord];
    vyWRITE[coord] = vyREAD[coord];
    vzWRITE[coord] = vzREAD[coord];
    pressureMapWRITE[coord] = pressureMapREAD[coord];

    if (coord.x == dims.x) vxWRITE[coord + int3(1, 0, 0)] = vxREAD[coord + int3(1, 0, 0)];
    if (coord.y == dims.y) vyWRITE[coord + int3(0, 1, 0)] = vyREAD[coord + int3(0, 1, 0)];
    if (coord.z == dims.z) vzWRITE[coord + int3(0, 0, 1)] = vzREAD[coord + int3(0, 0, 1)];

    updateVelocities(coord);
}