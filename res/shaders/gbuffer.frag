#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

out layout(location = 0) vec4 gPosition;
out layout(location = 1) vec3 gNormal;

layout(binding = 2) uniform sampler2D normalTextureSample;

uniform bool hasTexture;

void main() {
    gPosition = vec4(position, 1.0);

    vec3 norm;
    if (hasTexture) {
        norm = TBN * (texture(normalTextureSample, textureCoordinates).xyz * 2.0 - 1.0);
    } else {
        norm = normal;
    }

    gNormal = normalize(norm);
}