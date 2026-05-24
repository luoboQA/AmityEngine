// from shadertoy: https://www.shadertoy.com/view/MdXyzX
#version 460 core

out vec4 FragColor;

uniform mat3 u_CamRotation;
uniform float u_FOV;
in vec2 TexCoord;

// System Uniforms
uniform sampler2D depthTexture;
uniform vec3 u_CameraPos;
uniform float u_Time;
uniform vec2 u_Resolution;

// Water Parameters
uniform float u_WaterHeight; // Y plane height of water (e.g. 2.0f)
uniform float u_WaterDepth;  // Maximum depth of water waves (e.g. 1.0f)
uniform float u_WaterSpeed;  // Wave speed multiplier
uniform float u_WaterSpread; // Wave spatial density/spread multiplier

// Wave Physics Parameters
#define DRAG_MULT 0.38
#define ITERATIONS_RAYMARCH 12
#define ITERATIONS_NORMAL 36

// Calculates wave value and its derivative
vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters
float getwaves(vec2 position, int iterations) {
    position *= u_WaterSpread; // Apply customizable wave spread scale
    float wavePhaseShift = length(position) * 0.1; 
    float iter = 0.0; 
    float frequency = 1.0; 
    float timeMultiplier = 2.0; 
    float weight = 1.0;
    float sumOfValues = 0.0; 
    float sumOfWeights = 0.0; 
    for(int i = 0; i < iterations; i++) {
        vec2 p = vec2(sin(iter), cos(iter));
        // Apply customizable speed to the wave phase shift
        vec2 res = wavedx(position, p, frequency, u_Time * u_WaterSpeed * timeMultiplier + wavePhaseShift);

        // drag and derivative morph
        position += p * res.y * weight * DRAG_MULT;

        sumOfValues += res.x * weight;
        sumOfWeights += weight;

        weight = mix(weight, 0.0, 0.2);
        frequency *= 1.18;
        timeMultiplier *= 1.07;
        iter += 1232.399963;
    }
    return sumOfValues / sumOfWeights;
}

// Raymarches along the camera ray within the water column
float raymarchwater(vec3 camera, vec3 rayDir, float t_min, float t_max, float depth) {
    bool isAboveWater = (camera.y >= u_WaterHeight);
    float t = t_min;
    float stepSize = (t_max - t_min) / 64.0;
    
    for(int i = 0; i < 64; i++) {
        vec3 pos = camera + rayDir * t;
        
        // wave height is calculated in world space by shifting by u_WaterHeight
        float waveH = u_WaterHeight + getwaves(pos.xz, ITERATIONS_RAYMARCH) * depth - depth;
        
        // Hit check:
        // - Above water: we look down, hit when ray goes BELOW the wave surface (pos.y < waveH)
        // - Below water: we look up, hit when ray goes ABOVE the wave surface (pos.y > waveH)
        bool hit = isAboveWater ? (pos.y < waveH) : (pos.y > waveH);
        if (hit) {
            return t;
        }
        t += stepSize;
    }
    return t_max; // Return boundary exit if no wave hit registered
}

// Calculate normal at point using numerical derivation
vec3 calculateNormal(vec2 pos, float e, float depth) {
    vec2 ex = vec2(e, 0.0);
    float H = getwaves(pos.xy, ITERATIONS_NORMAL) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(
        cross(
            a - vec3(pos.x - e, getwaves(pos.xy - ex.xy, ITERATIONS_NORMAL) * depth, pos.y), 
            a - vec3(pos.x, getwaves(pos.xy + ex.yx, ITERATIONS_NORMAL) * depth, pos.y + e)
        )
    );
}

// Very basic but fast atmospheric approximation for sky reflection
vec3 cheap_atmosphere(vec3 raydir, vec3 sundir) {
    float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
    float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
    float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
    float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
    float mymie = sundt * special_trick * 0.2;
    vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
    vec3 bluesky = vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
    vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
    bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
    return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0));
} 

vec3 getSunDirection() {
    return normalize(vec3(-0.0773, 0.5 + sin(u_Time * 0.2 + 2.6) * 0.45, 0.5773));
}

vec3 getAtmosphere(vec3 dir) {
   return cheap_atmosphere(dir, getSunDirection()) * 0.5;
}

float getSunSpec(vec3 dir) { 
    return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
}

// ACES Tone mapping for realistic cinematic color response
vec3 aces_tonemap(vec3 color) {  
    mat3 m1 = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
    );
    mat3 m2 = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
    );
    vec3 v = m1 * color;  
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));  
}

void main()
{
    // 1. Reconstruct world-space camera ray from screen space TexCoord
    vec2 aspect_ratio = vec2(u_Resolution.x / u_Resolution.y, 1.0);
    vec3 point_cam = vec3((2.0 * TexCoord - 1.0) * aspect_ratio * u_FOV, -1.0);
    
    vec3 camRight = u_CamRotation[0];
    vec3 camUp    = u_CamRotation[1];
    vec3 camBack  = u_CamRotation[2];
    vec3 ray = normalize(camRight * point_cam.x + camUp * point_cam.y - camBack);
    
    // 1b. Determine if camera is above or below water level to support underwater rendering
    bool isAboveWater = (u_CameraPos.y >= u_WaterHeight);
    if (isAboveWater && ray.y >= -0.0001) {
        discard; // Looking up/horizon while above water -> discard
    }
    if (!isAboveWater && ray.y <= 0.0001) {
        discard; // Looking down/horizon while below water -> discard
    }

    // 2. Define Raycasting / Raymarching bounds within the active water column
    float t_top = (u_WaterHeight - u_CameraPos.y) / ray.y;
    float t_bottom = (u_WaterHeight - u_WaterDepth - u_CameraPos.y) / ray.y;

    float t_min = 0.0;
    float t_max = 0.0;

    if (isAboveWater) {
        t_min = t_top;
        t_max = t_bottom;
    } else {
        t_min = max(0.0, t_bottom);
        t_max = t_top;
    }

    // Epsilon threshold to prevent near-plane numerical glitches at the camera lens
    if (isAboveWater && t_min < 0.2) {
        discard;
    }

    // 3. Standard OpenGL Depth Buffer Occlusion Check
    // Sample the depth buffer at this screen pixel to know if an opaque object is in front of the water
    vec2 screenUV = gl_FragCoord.xy / u_Resolution;
    float depthVal = texture(depthTexture, screenUV).r;

    // Linearize screen depth to world space distance from camera
    float near = 0.1;
    float far = 5000.0;
    float z_ndc = depthVal * 2.0 - 1.0; 
    float terrainDistance = (2.0 * near * far) / (far + near - z_ndc * (far - near));

    // If the water entry point is already further than the physical terrain, early out discard!
    if (t_min > terrainDistance) {
        discard;
    }

    // 4. Raymarch through wave height displacement field along the camera ray
    float waveDist = raymarchwater(u_CameraPos, ray, t_min, t_max, u_WaterDepth);
    vec3 waterHitPos = u_CameraPos + ray * waveDist;

    // Depth Occlusion Resolution (after wave displacement)
    if (waveDist > terrainDistance) {
        discard;
    }

    // 6. Calculate Normal at the Wave Intersection Point
    vec3 N = calculateNormal(waterHitPos.xz, 0.01, u_WaterDepth);

    // Smooth out normals at large distances to eliminate high-frequency aliasing/noise
    N = mix(N, vec3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(waveDist * 0.01) * 1.1));

    // 7. Calculate Fresnel reflectivity coefficient
    float fresnel = (0.04 + (1.0 - 0.04) * (pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));

    // 8. Bounced ray reflection calculations
    vec3 R = normalize(reflect(ray, N));
    R.y = abs(R.y); // Constrain reflections above horizon
    
    vec3 reflection = getAtmosphere(R) + getSunSpec(R);
    
    // Deep water scattering / Subsurface absorption
    vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * 0.1 * (0.2 + (waterHitPos.y - (u_WaterHeight - u_WaterDepth)) / u_WaterDepth);

    // 9. Combine reflection & scattering and apply tonemapping
    vec3 finalColor = fresnel * reflection + scattering;
    
    // Shoreline blending (Depth fog): smoothly fade water out as it approaches terrain geometry
    float depthDiff = terrainDistance - waveDist;
    float shorelineAlpha = clamp(depthDiff / 1.5, 0.0, 1.0); // 1.5 units shoreline transition

    // Dynamic depth-based transparency: shallow water is crystal clear, deep water becomes opaque blue
    float depthFactor = clamp(depthDiff / 6.0, 0.0, 1.0);    // Transition to opaque over 6.0 units
    float waterAlpha = mix(0.35, 0.88, depthFactor) * shorelineAlpha; // 35% to 88% transparency bounds

    FragColor = vec4(aces_tonemap(finalColor * 2.0), waterAlpha);
}
