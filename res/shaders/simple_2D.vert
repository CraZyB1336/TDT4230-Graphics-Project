#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 2) vec2 textureCoordinates;

uniform mat4 OrthoVP;
uniform mat4 M;

out layout(location = 0) vec2 textureCoordinates_out;

void main()
{
    textureCoordinates_out = textureCoordinates;

    gl_Position = OrthoVP * M * vec4(position, 1.0);
}