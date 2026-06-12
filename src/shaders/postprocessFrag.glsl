#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform float time;
uniform vec2 iResolution;
uniform mat3 camRotation;
uniform float FOV;

float VIGNETTE_AMOUNT = 0.4;
float cld_brightness = 1.4;

// hashing
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// =-= CLOUDS =-=
#define cld_march_steps (50)
#define cld_coverage (.35)
#define cld_thick (90.)
#define cld_absorb_coeff (1.)
#define cld_wind_dir vec3(0, 0, time * .005)
#define cld_sun_dir normalize(vec3(0, 0, -1))

struct ray_t {
    vec3 origin;
    vec3 direction;
};

// Volumetric utilities
struct volume_sampler_t {
    vec3 origin; // start of ray
    vec3 pos; // current pos of acccumulation ray
    float height;
    float coeff_absorb;
    float T; // transmitance
    vec3 C; // color
    float alpha;
};

volume_sampler_t begin_volume(vec3 origin, float coeff_absorb) {
    return volume_sampler_t(
        origin, origin, 0.0,
        coeff_absorb, 1.0,
        vec3(0.0), 0.0
    );
}

volume_sampler_t integrate_volume(volume_sampler_t vol, vec3 V, vec3 L, float density, float dt) {
    float T_i = exp(-vol.coeff_absorb * density * dt);
    vol.T *= T_i;
    vol.C += vol.T * exp(vol.height) / 1.95 * density * dt;
    vol.alpha += (1.0 - T_i) * (1.0 - vol.alpha);
    return vol;
}

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 permute(vec4 x) {
    return mod289(((x * 34.0) + 1.0) * x);
}
vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    i = mod289(i);
    vec4 p = permute(permute(permute(
            i.z + vec4(0.0, i1.z, i2.z, 1.0))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    float n_ = 0.142857142857; // 1.0/7.0
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);
    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;
    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}

float fbm_clouds(vec3 pos, float lacunarity, float init_gain, float gain) {
    vec3 p = pos;
    float H = init_gain;
    float t = 0.0;
    for (int i = 0; i < 5; i++) {
        t += abs(snoise(p)) * H;
        p *= lacunarity;
        H *= gain;
    }
    return t;
}

float density_func(vec3 pos, float h) {
    vec3 p = pos * 0.001 + cld_wind_dir;
    float dens = fbm_clouds(p * 2.032, 2.6434, 0.5, 0.5);
    dens *= smoothstep(cld_coverage, cld_coverage + 0.035, dens);
    return dens;
}

vec4 render_clouds(ray_t eye) {
    const int steps = cld_march_steps;
    const float march_step = cld_thick / float(steps);

    float eyeY = eye.direction.y;
    if (abs(eyeY) < 0.0001) eyeY = 0.0001 * (eyeY < 0.0 ? -1.0 : 1.0);
    vec3 projection = eye.direction / eyeY;
    vec3 iter = projection * march_step;

    float cutoff = dot(eye.direction, vec3(0.0, 1.0, 0.0));

    volume_sampler_t cloud = begin_volume(
        eye.origin + projection * 100.0,
        cld_absorb_coeff
    );

    for (int i = 0; i < steps; i++) {
        cloud.height = (cloud.pos.y - cloud.origin.y) / cld_thick;
        float dens = density_func(cloud.pos, cloud.height);

        cloud = integrate_volume(cloud, eye.direction, cld_sun_dir, dens, march_step);

        cloud.pos += iter;
        cloud.pos += eye.direction;
    }

    return vec4(cloud.C, cloud.alpha * smoothstep(0.0, 0.2, cutoff));
}

vec3 render_sky(ray_t eye_ray) {
    vec3 sky = vec3(0.0, 0.0, 0.0);
    // Early-out if the ray points horizontally or downwards to prevent division-by-zero NaN glitches
    if (eye_ray.direction.y < 0.01) {
        return sky;
    }
    vec4 cld = render_clouds(eye_ray);
    return mix(sky, cld.rgb, cld.a);
}

ray_t get_primary_ray(vec3 cam_local_point, vec3 cam_origin) {
    // Manually extract orthonormal columns to be 100% immune to matrix layout transposition bugs
    vec3 camRight = camRotation[0];
    vec3 camUp    = camRotation[1];
    vec3 camBack  = camRotation[2]; // rot[2] is the back vector in camera space
    
    vec3 dir = normalize(camRight * cam_local_point.x + camUp * cam_local_point.y - camBack);
    return ray_t(cam_origin, dir);
}

// --- OPTICAL & CAMERA SENSOR EFFECT UTILITIES ---

// 1. Peripheral Lens Blur (5-tap fast blur scaled by radius)
vec3 sampleTextureBlurred(sampler2D tex, vec2 uv, float blurAmount) {
    vec3 col = texture(tex, uv).rgb;
    if (blurAmount > 0.001) {
        float step = blurAmount * 0.003;
        col += texture(tex, uv + vec2(-step, -step)).rgb;
        col += texture(tex, uv + vec2(step, -step)).rgb;
        col += texture(tex, uv + vec2(-step, step)).rgb;
        col += texture(tex, uv + vec2(step, step)).rgb;
        col *= 0.2;
    }
    return col;
}

// 2. Dynamic Auto-Gain Control (5-point sparse screen luminance lookup with depth-awareness)
float getSampleLuminance(vec2 uv) {
    float depth = texture(depthTexture, uv).r;
    if (depth >= 0.99999) {
        // Procedural sky and volumetric cloud average luminance estimate.
        // During daytime, this averages around 0.45, stabilizing the AGC from blowing out the clouds.
        return 0.45;
    }
    vec3 col = texture(screenTexture, uv).rgb;
    return dot(col, vec3(0.299, 0.587, 0.114));
}

float getSceneLuminance() {
    float lumC  = getSampleLuminance(vec2(0.5, 0.5));
    float lumTL = getSampleLuminance(vec2(0.25, 0.25));
    float lumTR = getSampleLuminance(vec2(0.75, 0.25));
    float lumBL = getSampleLuminance(vec2(0.25, 0.75));
    float lumBR = getSampleLuminance(vec2(0.75, 0.75));
    
    return (lumC + lumTL + lumTR + lumBL + lumBR) * 0.2;
}

// 3. Fixed-Pattern Noise (uncooled sensor row banding)
float getRowStriping(vec2 uv) {
    float yPixel = uv.y * iResolution.y;
    float band = sin(yPixel * 3.14159) * 0.008;
    float rowVal = random(vec2(floor(yPixel), 17.43));
    band += (rowVal - 0.5) * 0.035;
    return band;
}

void main()
{
    // Flat, sharp screen coordinates (NO barrel distortion, resolving edge artifacts completely!)
    vec2 toCenter = TexCoords - 0.5;
    float dist = length(toCenter);
    float peripheralBlur = smoothstep(0.4, 0.75, dist) * 0.35; // super slight blur at extreme corners
    
    // Sample texture with peripheral defocus blur (sharp in center, subtle softening in far corners)
    vec3 color = sampleTextureBlurred(screenTexture, TexCoords, peripheralBlur);
    float depth = texture(depthTexture, TexCoords).r;

    // Only draw volumetric clouds where depth is infinity (background/sky)
    if (depth >= 0.99999)
    {
        vec2 aspect_ratio = vec2(iResolution.x / iResolution.y, 1.0);
        vec3 eye = vec3(0.0, 0.0, 0.0);
        
        vec3 point_cam = vec3((2.0 * TexCoords - 1.0) * aspect_ratio * FOV, -1.0);
        ray_t ray = get_primary_ray(point_cam, eye);
        
        vec3 skyColor = render_sky(ray) * cld_brightness;
        color += skyColor;
    }

    // --- CAMERA SIMULATION ---

    // 1. Dynamic Auto-Gain Control (AGC)
    // Damped curve that stabilizes around 1.0 under standard lighting and clamps at 2.0 max
    float avgLuminance = getSceneLuminance();
    float gain = 0.55 / (avgLuminance + 0.2);
    gain = clamp(gain, 0.4, 2.0);

    // 2. Grayscale conversion commented out to keep vibrant full colors!
    // float thermal = dot(color.rgb, vec3(0.299, 0.587, 0.114)) * gain;
    
    // Brightness scaling & camera response contrast applied to full RGB channels
    vec3 baseColor = color.rgb * gain;
    baseColor = pow(clamp(baseColor, 0.0, 1.0), vec3(1.25)) * 0.95;

    // 3. Layer bloom, dynamic noise, fixed-pattern row bands, and scanlines
    float luma = dot(baseColor, vec3(0.299, 0.587, 0.114));
    float bleed = (luma > 0.85) ? 0.05 : 0.0;
    float noise = random(TexCoords + vec2(sin(time), cos(time)) * 8.5) * 0.05 * (1.0 + 0.6 * gain);
    float fpn = getRowStriping(TexCoords);
    float scanline = sin(TexCoords.y * iResolution.y * 2.0) * 0.035;

    // 4. Combine all artifacts and apply physical vignette
    vec3 composite = (baseColor + vec3(bleed + noise + fpn - scanline)) * clamp(1.0 - dist * dist * 0.85, 0.0, 1.0);
    FragColor = vec4(clamp(composite, 0.0, 1.0), 1.0);
}