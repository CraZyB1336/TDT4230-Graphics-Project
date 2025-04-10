#version 430 core

layout(binding = 1) uniform sampler2D textureSample;

out vec4 color;

void main()
{
    color = texture(textureSample);
}
