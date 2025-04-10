#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 position;
in layout(location = 3) mat3 TBN;

layout(binding = 2) uniform sampler2D normalTextureSample;

uniform bool hasTexture;
uniform vec3 cameraPosition;

out vec4 color;

void main()
{
    vec3 norm;
    if (hasTexture) {
        norm = TBN * (texture(normalTextureSample, textureCoordinates).xyz * 2.0 - 1.0);
    } else {
        norm = normal;
    }

    vec3 viewDir = normalize(cameraPosition - position);
    float fallOff = pow(1.0 - max(dot(normalize(norm), viewDir), 0.0), 4.0);
    color = vec4(vec3(fallOff), 1.0);
}