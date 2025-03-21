#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

layout(binding = 0, rgba32f) uniform image2D diffuseTextureSample;
layout(binding = 1) uniform sampler2D textureSample;
layout(binding = 2) uniform sampler2D normalTextureSample;
layout(binding = 3) uniform sampler2D roughnessTextureSample;

struct LightSource {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform LightSource[1] lights;
uniform vec3 cameraPosition;
uniform bool hasTexture;
uniform bool isSubsurface;

vec3 hardColor = vec3(0.06);
vec3 diffuse = vec3(0.0);
vec3 specular = vec3(0.0);

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

// Attenuation 
float la = 0.01;
float lb = 0.001;
float lc = 0.002;

// vec3 reject(vec3 from, vec3 onto) {
//     return from - onto*dot(from, onto)/dot(onto, onto);
// }

void calculateDiffusion(vec3 norm) {
    for (int i = 0; i < 1; i++) {
        vec3 toLight = lights[i].position - position;
        vec3 nToLight = normalize(toLight);

        float lightDistance = length(toLight);
        // float L = 1 / (la + (lightDistance * lb) + (lightDistance * lightDistance * lc));
        
        // Diffuse
        float diffuseIntensity = max(dot(nToLight, norm), 0.0);
        diffuse += diffuseIntensity * lights[i].intensity * lights[i].color;
    }
}

void calculateSpecular(vec3 norm, vec3 roughness) {
    vec3 toCam = normalize(cameraPosition - position);
    
    for (int i = 0; i < 1; i++) {
        vec3 toLight = lights[i].position - position;
        vec3 nToLight = normalize(toLight);

        float lightDistance = length(toLight);
        // float L = 1 / (la + (lightDistance * lb) + (lightDistance * lightDistance * lc));

        float rough = 5 / (roughness.r * roughness.r);

        vec3 reflectedL = reflect(-nToLight, norm);
        float specularIntensity = pow(max(dot(reflectedL, toCam), 0.0), rough);
        specular += specularIntensity * lights[i].intensity * lights[i].color;
    }
}

void main()
{
    vec3 norm = TBN * (texture(normalTextureSample, textureCoordinates).xyz * 2.0 - 1.0);
    vec3 roughnessTextureSample = texture(roughnessTextureSample, textureCoordinates).xyz;

    if (isSubsurface) {
        ivec2 pxCoord = ivec2(gl_FragCoord.xy);
        vec4 diffuseTexture = imageLoad(diffuseTextureSample, pxCoord);

        if (hasTexture) {
            calculateSpecular(norm, roughnessTextureSample);
        } else {
            calculateSpecular(normal, vec3(1.0));
        }

        vec3 lightColor = diffuseTexture.rgb + specular + dither(textureCoordinates);
        color = vec4(lightColor, 1.0);
    } else {
        vec4 brickTexture = texture(textureSample, textureCoordinates);

        if (hasTexture) {
            calculateDiffusion(norm);
            calculateSpecular(norm, roughnessTextureSample);
            vec3 lightColor = hardColor + diffuse + specular + dither(textureCoordinates);
            color = vec4(lightColor, 1.0) * brickTexture;
        } else {
            calculateDiffusion(normal);
            calculateSpecular(normal, vec3(1.0));
            vec3 lightColor = hardColor + diffuse + specular + dither(textureCoordinates);
            color = vec4(lightColor, 1.0);
        }
    }
}