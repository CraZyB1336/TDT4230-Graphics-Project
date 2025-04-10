#version 430 core

in layout(location = 2) vec3 position;

struct LightSource {
    vec3 position;
    vec3 color;
    float intensity;
    int type; // 0 = point, 1 = dir, 2 = spot (no support for spot)
};

uniform LightSource[1] lights;

out vec4 color;

void main()
{
    color = vec4(0.74, 1, 0.98, 1.0);
}
