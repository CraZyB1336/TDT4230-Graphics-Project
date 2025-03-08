#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

struct LightSource {
    vec3 position;
    vec3 color;
    float intensity;
};

// uniform LightSource[3] lights;
uniform vec3 cameraPosition;

vec3 hardColor = vec3(0.29);
vec3 diffuse = vec3(0.0);
vec3 specular = vec3(0.0);

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

// Attenuation 
float la = 0.003;
float lb = 0.004;
float lc = 0.003;

// vec3 reject(vec3 from, vec3 onto) {
//     return from - onto*dot(from, onto)/dot(onto, onto);
// }

void main()
{
    if (gl_FragCoord.x < 10.0) {
        color = vec4(0.2, 0.4, 0.5, 1.0);
    } else {
        color = vec4(0.6, 0.2, 0.5, 1.0);
    }
}