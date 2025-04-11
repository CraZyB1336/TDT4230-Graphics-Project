#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

layout(binding = 2) uniform sampler2D normalTextureSample;

struct LightSource {
    vec3 position; // Is directional light
    vec3 color;
    float intensity;
    int type; // 0 = point, 1 = dir, 2 = spot (no support for spot)
};

uniform LightSource[1] lights;

uniform bool hasTexture;
uniform vec3 cameraPosition;
uniform float baseThickness;
uniform int materialType;

out float color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    vec3 norm;
    if (hasTexture) {
        norm = TBN * (texture(normalTextureSample, textureCoordinates).xyz * 2.0 - 1.0);
    } else {
        norm = normal;
    }

    vec3 lightDirection = normalize(lights[0].position);

    vec3 viewDir = normalize(cameraPosition - position);

    float viewAngle = 1.0 - max(dot(norm, viewDir), 0.0);
    float viewBasedThickness = pow(viewAngle, 3.3);

    float lightAngle = max(dot(norm, -lightDirection), 0.0);
    float lightBasedThickness = pow(1.0 - lightAngle, 4.0);

    float thickness = baseThickness;

    if (materialType == 0) { // skin
        // More viewbased
        thickness *= mix(viewBasedThickness, lightBasedThickness, 0.3);

        // float noisePattern = sin(textureCoordinates.x * 20.0) * sin(textureCoordinates.y * 20.0) * 0.5 + 0.5;
        // thickness = mix(thickness, thickness * noisePattern, 0.15);

        thickness = smoothstep(0.001, 0.3, thickness);
    } else { // leaves
        // More lightbased
        thickness *= mix(viewBasedThickness, lightBasedThickness, 0.7);

        float veins = 0.0;

        float mainVein = abs(textureCoordinates.x - 0.5) < 0.05 ? 1.0 : 0.0;
        float secondaryVeins = 0.0;
        for (float i = 0.2; i <= 0.8; i += 0.2) {
            secondaryVeins += abs(textureCoordinates.y - i) < 0.03 ? 1.0 : 0.0;
        }

        veins = max(mainVein, secondaryVeins * 0.7);

        thickness = mix(thickness * 0.7, thickness * 1.3, smoothstep(0.06, 0.1, veins));

        // Sharper
        thickness = pow(thickness, 1.5);
    }

    // Non linear curve
    thickness += dither(textureCoordinates);

    thickness = clamp(thickness, 0.0, 1.0);

    color = thickness;
}
