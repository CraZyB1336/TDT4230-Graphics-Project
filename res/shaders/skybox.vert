#version 430 core

in layout(location = 0) vec3 position;

// Model transformation matrix
uniform mat4 VP;

out layout(location = 2) vec3 position_out;

void main()
{
    position_out = position;

    gl_Position = VP * vec4(position, 1.0);
}
