import utils;

struct PushConstants {
    uint ping;
};
[[vk::push_constant]]
uniform PushConstants push;

float pressureSolveCell(int3 cellPos) {
    // divergence = du/dx + du/dy + du/dz
    // divergence within a single cell
    // thus use the faces
    // when sampling velocity fields, use no trilinear interpolation
    // ie use image[coord] instead of .SampleLevel
    // only calculate cells that have 1 <= ijk < dim
    // du/dx = (vx(i+1,j,k) - vx(i,j,k)) / 1 = vx(i+1,j,k) - vx(i,j,k)
    // du/dy = vy(i,j+1,k) - vy(i,j,k)
    // du/dz = vz(i,j,k+1) - vz(i,j,k)
    
    int flowRight  = isSolid(cellPos + int3( 1,  0,  0), solidMap) ? 0 : 1;
    int flowFront  = isSolid(cellPos + int3( 0,  1,  0), solidMap) ? 0 : 1;
    int flowTop    = isSolid(cellPos + int3( 0,  0,  1), solidMap) ? 0 : 1;
    int flowLeft   = isSolid(cellPos + int3(-1,  0,  0), solidMap) ? 0 : 1;
    int flowBack   = isSolid(cellPos + int3( 0, -1,  0), solidMap) ? 0 : 1;
    int flowBottom = isSolid(cellPos + int3( 0,  0, -1), solidMap) ? 0 : 1;

    int fluidEdgeCount = flowRight + flowFront + flowTop + flowLeft + flowBack + flowBottom;

    if (isSolid(cellPos, solidMap) || fluidEdgeCount == 0) {
        return 0; // no pressure in solid cells
    }

    float pressureRight  = getPressure(cellPos + int3(1, 0, 0), pressureMapREAD);
    float pressureTop    = getPressure(cellPos + int3(0, 0, 1), pressureMapREAD);
    float pressureFront  = getPressure(cellPos + int3(0, 1, 0), pressureMapREAD);
    float pressureBack   = getPressure(cellPos + int3(0, -1, 0), pressureMapREAD);
    float pressureLeft   = getPressure(cellPos + int3(-1, 0, 0), pressureMapREAD);
    float pressureBottom = getPressure(cellPos + int3(0, 0, -1), pressureMapREAD);

    float velocityLeft   = vxREAD[cellPos + int3(0, 0, 0)];
    float velocityBack   = vyREAD[cellPos + int3(0, 0, 0)];
    float velocityBottom = vzREAD[cellPos + int3(0, 0, 0)];
    float velocityRight  = vxREAD[cellPos + int3(1, 0, 0)];
    float velocityFront  = vyREAD[cellPos + int3(0, 1, 0)];
    float velocityTop    = vzREAD[cellPos + int3(0, 0, 1)];

    float pressureSum = 
        flowRight  * pressureRight +
        flowTop    * pressureTop +
        flowFront  * pressureFront +
        flowBack   * pressureBack +
        flowLeft   * pressureLeft +
        flowBottom * pressureBottom;

    float divergence = velocityRight - velocityLeft + velocityTop - velocityBottom + velocityFront - velocityBack;

    return float((pressureSum - density * divergence / dt) / float(fluidEdgeCount));
}

// gauss seidel red black checkerboard pattern
void solvePressure(int3 coord) {
    // if ping = 0, then only do even/even coords
    // if ping = 1, then only do odd/odd coords
    float pressure = pressureMapREAD[coord];

    if ((coord.x + coord.y + coord.z) % 2 == push.ping) {
        float pressureNew = pressureSolveCell(coord);
        pressure = pressure + (pressureNew - pressure) * weightSOR;
    }

    pressureMapWRITE.Store(coord, pressure);
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

    if (isSolid(coord, solidMap)) return;

    solvePressure(coord);
}