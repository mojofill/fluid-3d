import utils;

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

    if (coord.x == dims.x - 1) vxWRITE[coord + int3(1, 0, 0)] = vxREAD[coord + int3(1, 0, 0)];
    if (coord.y == dims.y - 1) vyWRITE[coord + int3(0, 1, 0)] = vyREAD[coord + int3(0, 1, 0)];
    if (coord.z == dims.z - 1) vzWRITE[coord + int3(0, 0, 1)] = vzREAD[coord + int3(0, 0, 1)];

    updateVelocities(coord);
}