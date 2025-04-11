#version 430 core

in layout(location = 0) vec2 textureCoordinates;
layout(binding = 0) uniform sampler2D textureSample;

out vec4 color;

void main()
{
    vec4 text =  texture(textureSample, textureCoordinates);
    vec3 textCol = text.rgb;

    if (textCol.r > 0.5 ) textCol = vec3(0.0);


    color = vec4(textCol, text.a);
}
