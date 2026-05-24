#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform float time;
uniform vec2 iResolution;
uniform mat3 camRotation;
uniform float FOV;

float VIGNETTE_AMOUNT = 0.5;
float cld_brightness = 0.35; // Gloomy dark overcast by default

// hashing
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// =-= CLOUDS =-=
#define cld_march_steps (50)
#define cld_coverage (.42)
#define cld_thick (90.)
#define cld_absorb_coeff (1.3)
#define cld_wind_dir vec3(0.01 * time, 0, time * .015)
#define cld_sun_dir normalize(vec3(0.1, 0.8, -0.2))

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
    float T; // transmittance
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
    // Darker, stormier cloud base color
    vec3 cloudBase = vec3(0.15, 0.18, 0.22);
    vol.C += vol.T * cloudBase * exp(vol.height) / 1.95 * density * dt;
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
    // Dark slate gray sky background
    vec3 sky = vec3(0.04, 0.06, 0.1);
    if (eye_ray.direction.y < 0.01) {
        return sky;
    }
    vec4 cld = render_clouds(eye_ray);
    return mix(sky, cld.rgb, cld.a);
}

ray_t get_primary_ray(vec3 cam_local_point, vec3 cam_origin) {
    vec3 camRight = camRotation[0];
    vec3 camUp    = camRotation[1];
    vec3 camBack  = camRotation[2];
    
    vec3 dir = normalize(camRight * cam_local_point.x + camUp * cam_local_point.y - camBack);
    return ray_t(cam_origin, dir);
}

// 1. Dynamic Lightning double-flicker model
float getLightningIntensity(float t) {
    float period = 13.0; // Strike every 13s
    float cycle = floor(t / period);
    float offset = fract(sin(cycle * 37.59) * 43758.545) * 4.0; // random drift 0-4s
    float strikeTime = 3.0 + offset;
    float dt = mod(t, period) - strikeTime;
    
    if (dt < 0.0 || dt > 0.8) return 0.0;
    
    float intensity = 0.0;
    if (dt < 0.15) {
        // First fast flicker
        intensity = mix(0.0, 1.0, dt / 0.05) * step(0.0, dt) * step(dt, 0.05) +
                    mix(1.0, 0.15, (dt - 0.05) / 0.10) * step(0.05, dt) * step(dt, 0.15);
    } else if (dt < 0.8) {
        // Second main surge and slow exponential decay
        float dt2 = dt - 0.15;
        if (dt2 < 0.05) {
            intensity = mix(0.15, 2.5, dt2 / 0.05);
        } else {
            intensity = 2.5 * exp(-7.0 * (dt2 - 0.05));
        }
    }
    return intensity;
}

// 2. High-performance diagonal rain noise cell generator
float rainStreak(vec2 uv, float scale, float speed, float t) {
    vec2 rv = uv;
    rv.y -= t * speed;
    rv.x += t * speed * 0.12; // Slanted falling rain
    rv.x *= scale;
    rv.y *= scale * 0.06; // Highly stretched cells vertically
    
    vec2 ip = floor(rv);
    vec2 fp = fract(rv);
    
    float h = fract(sin(ip.x * 12.9898 + ip.y * 37.29) * 43758.5453);
    
    if (h > 0.88) { // 12% density
        float w = smoothstep(0.14, 0.0, abs(fp.x - 0.5));
        float len = smoothstep(0.0, 0.15, fp.y) * smoothstep(1.0, 0.3, fp.y);
        return w * len * (0.2 + 0.8 * fract(h * 10.0));
    }
    return 0.0;
}

float getRainOverlay(vec2 uv, float t, float yaw, float pitch, float lightning) {
    // 3D Parallax rain layers shifting by camera rotation!
    vec2 uvF = uv + vec2(yaw * 0.25, pitch * 0.25);
    float r1 = rainStreak(uvF, 50.0, 16.0, t) * 0.35; // Foreground
    
    vec2 uvM = uv + vec2(yaw * 0.12, pitch * 0.12);
    float r2 = rainStreak(uvM, 100.0, 24.0, t) * 0.22; // Midground
    
    vec2 uvB = uv + vec2(yaw * 0.04, pitch * 0.04);
    float r3 = rainStreak(uvB, 200.0, 32.0, t) * 0.10; // Background
    
    float totalRain = r1 + r2 + r3;
    
    // Rain droplets illuminate dynamically during lightning flashes
    return totalRain * (1.0 + lightning * 4.0);
}

void main()
{
    vec2 aspect_ratio = vec2(iResolution.x / iResolution.y, 1.0);
    vec2 toCenter = TexCoords - 0.5;
    float dist = length(toCenter);
    
    // Compute lightning intensity
    float lightning = getLightningIntensity(time);
    
    // Sample scene buffer
    vec3 color = texture(screenTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;

    // Apply volumetric sky/clouds
    if (depth >= 0.99999)
    {
        vec3 eye = vec3(0.0, 0.0, 0.0);
        vec3 point_cam = vec3((2.0 * TexCoords - 1.0) * aspect_ratio * FOV, -1.0);
        ray_t ray = get_primary_ray(point_cam, eye);
        
        vec3 skyColor = render_sky(ray) * cld_brightness;
        
        // Clouds flash cool blue-white during lightning strike
        vec3 skyFlash = vec3(0.70, 0.85, 1.0) * lightning * 2.2;
        skyColor += (skyColor + 0.1) * skyFlash;
        
        color += skyColor;
    }
    else
    {
        // 3D Scene elements (boat, ocean) get realistically lit by lightning flash!
        vec3 sceneFlash = vec3(0.75, 0.85, 1.0) * lightning * 1.6;
        color += color * sceneFlash;
    }

    // Apply camera rotation projection for rain parallax sliding
    vec3 camForward = -camRotation[2];
    float yaw = atan(camForward.x, camForward.z);
    float pitch = asin(clamp(camForward.y, -0.99, 0.99));
    
    // Compute rain overlay
    float rain = getRainOverlay(TexCoords * aspect_ratio, time, yaw, pitch, lightning);
    vec3 rainColor = vec3(0.75, 0.82, 0.9) * rain;
    color = mix(color, rainColor, rain * 0.85);

    // Apply dark physical storm vignette
    color *= clamp(1.0 - dist * dist * VIGNETTE_AMOUNT, 0.0, 1.0);
    
    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
