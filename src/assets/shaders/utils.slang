module utils;

public const static float fov = 1.5707963268 * 0.9f;

public struct Camera {
    public float4 pos;
    public float4 forward;
    public float4 right;
    public float4 up;
};

public struct Ray {
    public float3 origin;
    public float3 dir;
};

public struct BoundingBox {
    public float3 minXYZ;
    public float3 maxXYZ;
};

// can probably improve this, but doesnt matter for now
public bool intersectBB(Ray ray, BoundingBox bb, out float tmin, out float tmax) {
    ray.dir = normalize(ray.dir);
    tmin = (bb.minXYZ.x - ray.origin.x) / ray.dir.x;
    tmax = (bb.maxXYZ.x - ray.origin.x) / ray.dir.x;

    if (tmin > tmax) {
        float temp = tmin;
        tmin = tmax;
        tmax = temp;
    }

    float tymin = (bb.minXYZ.y - ray.origin.y) / ray.dir.y;
    float tymax = (bb.maxXYZ.y - ray.origin.y) / ray.dir.y;

    if (tymin > tymax) {
        float temp = tymin;
        tymin = tymax;
        tymax = temp;
    }

    float tzmin = (bb.minXYZ.z - ray.origin.z) / ray.dir.z;
    float tzmax = (bb.maxXYZ.z - ray.origin.z) / ray.dir.z;

    if (tzmin > tzmax) {
        float temp = tzmin;
        tzmin = tzmax;
        tzmax = temp;
    }

    if (tmin > tymax || tymin > tmax) return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    if (tmin > tzmax || tzmin > tmax) return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    return true;
}

public Ray createCameraRay(in Camera camera, in int2 coord, in int2 imgSize) {
    float2 uv = float2(
        (coord.x + 0.5) / float(imgSize.x) * 2 - 1,
        1 - (coord.y + 0.5) / float(imgSize.y) * 2
    );

    float k = tan(fov * 0.5);
    uv.x *= k * imgSize.x / float(imgSize.y);
    uv.y *= k;

    float3 worldDir =
        camera.right.xyz * uv.x +
        camera.forward.xyz +
        camera.up.xyz * uv.y;

    worldDir = normalize(worldDir);

    return Ray(camera.pos.xyz, worldDir);
}

uint hash3D(int3 p) {
    uint h = uint(p.x) * 374761393u
           + uint(p.y) * 668265263u
           + uint(p.z) * 2147483647u;

    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;

    return h;
}

float hashToFloat(int3 p) {
    return float(hash3D(p) & 0x00FFFFFFu) / 16777216.0;
}

float3 gradient3D(int3 p) {
    // Uniformly distributed spherical gradient direction.
    float z = hashToFloat(p) * 2.0 - 1.0;
    float theta = hashToFloat(p + int3(157, 113, 269)) * 6.28318530718;

    float r = sqrt(max(0.0, 1.0 - z * z));

    return float3(
        r * cos(theta),
        r * sin(theta),
        z
    );
}

float fade(float t) {
    // Quintic interpolation curve.
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

public float perlin3D(float3 p) {
    int3 cell = int3(floor(p));
    float3 local = frac(p);

    float3 u = float3(
        fade(local.x),
        fade(local.y),
        fade(local.z)
    );

    float n000 = dot(gradient3D(cell + int3(0, 0, 0)), local - float3(0, 0, 0));
    float n100 = dot(gradient3D(cell + int3(1, 0, 0)), local - float3(1, 0, 0));
    float n010 = dot(gradient3D(cell + int3(0, 1, 0)), local - float3(0, 1, 0));
    float n110 = dot(gradient3D(cell + int3(1, 1, 0)), local - float3(1, 1, 0));

    float n001 = dot(gradient3D(cell + int3(0, 0, 1)), local - float3(0, 0, 1));
    float n101 = dot(gradient3D(cell + int3(1, 0, 1)), local - float3(1, 0, 1));
    float n011 = dot(gradient3D(cell + int3(0, 1, 1)), local - float3(0, 1, 1));
    float n111 = dot(gradient3D(cell + int3(1, 1, 1)), local - float3(1, 1, 1));

    float x00 = lerp(n000, n100, u.x);
    float x10 = lerp(n010, n110, u.x);
    float x01 = lerp(n001, n101, u.x);
    float x11 = lerp(n011, n111, u.x);

    float y0 = lerp(x00, x10, u.y);
    float y1 = lerp(x01, x11, u.y);

    return lerp(y0, y1, u.z);
}

public float fbm3D(float3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    float totalAmplitude = 0.0;

    const int OCTAVES = 10;

    for (int i = 0; i < OCTAVES; i++)
    {
        value += perlin3D(p * frequency) * amplitude;
        totalAmplitude += amplitude;

        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return value / totalAmplitude;
}

// Main sampling function.
// Input: integer coordinates in [0, 40].
// Output: approximately [0, 1].
public float noise3D(int3 position) {
    float3 p = float3(position);

    float baseNoise = fbm3D(p * 0.030);
    float detailNoise = fbm3D(p * 0.005);

    baseNoise = baseNoise * 0.5 + 0.5;
    detailNoise = detailNoise * 0.5 + 0.5;

    float density = baseNoise - (1.0 - detailNoise) * 0.15;

    return lerp(0.0, 1, smoothstep(0.3, 0.7, density));
}