import utils;

// do some 3d perlin noise
// alright my next step is to turn this into 3d smoke. that is, give it some velocity and pressure

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

    // initialize density
    // float3 p = coord - 0.5 * float3(dims);
    // if (length(p) < 80) densityImageWRITE.Store(coord, 1);
    // else densityImageWRITE.Store(coord, 0);
    densityImageWRITE.Store(coord, 0);

    // initialize velocity
    int width = dims.x;
    int _length = dims.y;
    int height = dims.z;

    uint seed = hashSeed(uint3(dispatchThreadID.xy, uint(frameCount)));

    float k = 100;

    if (coord.x == 0) vxWRITE.Store(coord, 0);
    else vxWRITE.Store(coord, k * (2 * randf(seed) - 1));
    if (coord.x == width - 1) vxWRITE.Store(coord + int3(1,0,0), 0);

    if (coord.y == 0) vyWRITE.Store(coord, 0);
    else vyWRITE.Store(coord, k * (2 * randf(seed) - 1));
    if (coord.y == _length - 1) vyWRITE.Store(coord + int3(0,1,0), 0);

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
