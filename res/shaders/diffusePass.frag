#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

layout(binding = 1) uniform sampler2D textureSample;
layout(binding = 2) uniform sampler2D normalTextureSample;
layout(binding = 3) uniform sampler2D roughnessTextureSample;
// layout(binding = 4) uniform sampler2D ssaoTexture;

struct LightSource {
    vec3 position;
    vec3 color;
    float intensity;
    int type; // 0 = point, 1 = dir, 2 = spot (no support for spot)
};

uniform LightSource[1] lights;
uniform bool hasTexture;

uniform vec3 albedo;
uniform float roughnessFactor;

vec3 diffuse = vec3(0.0);

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

// Attenuation 
float la = 0.01;
float lb = 0.001;
float lc = 0.002;


void calculateLights(vec3 norm, float roughness) {

    // float aoFactor = texture(ssaoTexture, gl_FragCoord.xy / vec2(textureSize(ssaoTexture, 0))).r;

    for (int i = 0; i < 1; i++) {
        vec3 toLight = lights[i].type == 0 ? lights[i].position - position : -lights[i].position;
        vec3 nToLight = normalize(toLight);

        float lightDistance = length(toLight);
        // float L = 1 / (la + (lightDistance * lb) + (lightDistance * lightDistance * lc));
        
        // Diffuse
        float diffuseIntensity = max(dot(nToLight, norm), 0.0);
        diffuse += diffuseIntensity * lights[i].intensity * lights[i].color * (1.0 - roughness);
    }
}

void main()
{
    vec3 norm = TBN * (texture(normalTextureSample, textureCoordinates).xyz * 2.0 - 1.0);
    float roughnessTexture = texture(roughnessTextureSample, textureCoordinates).r;

    float roughness = roughnessFactor;

    if (roughnessFactor >= 1.0)
    {
        roughness = 0.95;
    } else if (roughnessFactor <= 0) {
        roughness = 0.05;
    }

    // float aoFactor = texture(ssaoTexture, gl_FragCoord.xy / vec2(textureSize(ssaoTexture, 0))).r;
    // vec3 ambient = albedo * aoFactor;

    if (hasTexture) {
        vec4 texture = texture(textureSample, textureCoordinates);

        calculateLights(norm, roughnessTexture);
        vec3 lightColor = albedo + diffuse + dither(textureCoordinates);
        color = vec4(lightColor, 1.0) * texture;
    } else {
        calculateLights(normal, roughness);
        vec3 lightColor = albedo + diffuse + dither(textureCoordinates);
        color = vec4(lightColor, 1.0);
    }

    // color = vec4(vec3(gl_FragCoord.z), 1.0);
}