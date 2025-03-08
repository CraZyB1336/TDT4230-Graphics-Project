#version 430 core

in layout(location = 0) vec2 textureCoordinates;

out vec4 color;

void main()
{
    color = vec4(1.0, 0.4, 0.3, 1.0);
}
