import utils;

static const float lightStepSize = 0.5;
static const float regularStepSize = 0.005;
static const int maxNumRegularSteps = 200;
static const int FASTmaxNumRegularSteps = 10;

// for now, can treat extinction as one across all three color channels. in future i want to separate extinction coefficients per channel
static const float3 extinctionCoefficient = 0.025;

// static const float3 backgroundColor = float3(0.5, 0.8, 0.9);
// static const float3 backgroundColor = float3(1);
static const float3 backgroundColor = float3(0);
static const float3 lightColor = float3(1);
// static const float3 cloudAlbedo = float3(0.75, 0.25, 0.10);
static const float3 cloudAlbedo = float3(1);

float3 marchToLight(float3 pos) {
    float3 sunPos = float3(0, 0, 200);
    sunPos = camera.pos.xyz;
    Ray ray = Ray(pos, normalize(sunPos - pos));
    float tmin, tmax;
    BoundingBox bb = { minXYZ.xyz, maxXYZ.xyz };
    if (!intersectBB(ray, bb, tmin, tmax)) {
        return 1;
    }

    float t = max(0, tmin);
    pos += t * ray.dir;
    float stepSize = lightStepSize;

    int3 dimensions;
    densityImageWRITE.GetDimensions(dimensions.x, dimensions.y, dimensions.z);
    float3 transmittance = 1;
    while (t < tmax) {
        float ds = min(lightStepSize, tmax - t);
        float3 density = densityImageREAD.SampleLevel(linearSampler, (pos - minXYZ.xyz) / (maxXYZ.xyz - minXYZ.xyz), 0).xyz;
        float3 stepExtinction = density * extinctionCoefficient * ds;
        float3 stepTransmittance = exp(-stepExtinction);
        transmittance *= stepTransmittance;
        if (any(transmittance < 0.01)) {
            break;
        }
        pos += ds * ray.dir;
        t += ds;
    }
    return transmittance;
}

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    int2 coord = int2(dispatchThreadID.xy);
    int2 imgSize;
    outputImage.GetDimensions(imgSize.x, imgSize.y);
    int3 dimensions;
    densityImageWRITE.GetDimensions(dimensions.x, dimensions.y, dimensions.z);

    if (any(coord >= imgSize)) return;

    Ray ray = createCameraRay(camera, coord, imgSize);
    BoundingBox bb = { minXYZ.xyz, maxXYZ.xyz };

    float4 color = float4(backgroundColor, 1);
    float tmin, tmax;
    if (intersectBB(ray, bb, tmin, tmax) && tmax > 0) {
        tmin = max(tmin, 0);
        // ok. remember your beer lambert law
        ray.origin += (tmin + 1e-4) * ray.dir;
        float3 transmittance = 1;
        float3 scatteredLight = float3(0);

        float stepSize = regularStepSize;
        float rayLength = tmax - tmin;
        int numSteps = max(1, int(ceil(rayLength / stepSize)));
        numSteps = min(numSteps, fast ? FASTmaxNumRegularSteps : maxNumRegularSteps);
        stepSize = rayLength / float(numSteps);

        float3 currPos = ray.origin;

        for (int i = 0; i < numSteps; i++) {
            // get percent of light that reaches this sample point from the sun
            // for now set the camera origin as the sun, like a flashlight
            float3 lightTransmittance = marchToLight(currPos);
            float3 incomingLight = lightTransmittance * lightColor; // lightColor = float3(1). replace with diff color for diff light color

            // find how percent of light that scatters toward the camera
            float4 smokeData = densityImageREAD.SampleLevel(linearSampler, (currPos - minXYZ.xyz) / (maxXYZ.xyz - minXYZ.xyz), 0);
            float density = smokeData.w;
            float3 smokeColor = smokeData.xyz;
            float3 stepExtinction = density * extinctionCoefficient * stepSize;
            float3 stepTransmittance = exp(-stepExtinction);

            float3 stepScatter = incomingLight * smokeColor * (1.0 - stepTransmittance); // cloud base color/albedo = float3(1), change to diff color for diff cloud tint. step transmittance = how much light was kept. 1 - step transmittance = how much light was scattered (could be absorbed no? ig it doesnt matter for an approximation). only scattered light contributes light to the camera

            scatteredLight += transmittance * stepScatter;

            transmittance *= stepTransmittance; // keep track of how much is being kept as ray moves further into medium

            if (any(transmittance < 0.01)) {
                break;
            }

            currPos += ray.dir * stepSize;
        }

        color = float4(transmittance * backgroundColor + scatteredLight, 1); // background color = float3(1)
    }

    outputImage[coord] = color;
}