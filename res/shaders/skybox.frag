#version 430 core

in layout(location = 2) vec3 position;

struct LightSource {
    vec3 position;
    vec3 color;
    float intensity;
    int type; // 0 = point, 1 = dir, 2 = spot (no support for spot)
};

uniform LightSource[1] lights;
uniform int screenHeight;

out vec4 color;

vec3 topColor = vec3(0.1, 0.4, 0.7);
vec3 bottomColor = vec3(0.8, 0.9, 1.0) + vec3(0.2, 0.1, 0.0);
vec3 sunColor = vec3(1.0, 0.9, 0.7);

void main()
{
    vec3 skyColor = mix(bottomColor, topColor, gl_FragCoord.y / 1080);

    vec3 viewDir = normalize(position);
    viewDir.z = viewDir.z * -1;
    vec3 sunDir = normalize(-lights[0].position);
    float sunDot = dot(viewDir, sunDir);

    float sunIntensity = smoothstep(0.995, 0.998, sunDot);
    vec3 sunCol = sunColor * lights[0].intensity * 1.5;

    vec3 finalColor = mix(skyColor, sunCol, sunIntensity);

    float sunGlow = smoothstep(0.97, .995, sunDot);
    finalColor = mix(finalColor, sunCol * 0.6, sunGlow * 0.5);

    color = vec4(finalColor, 1.0);
}
