#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent;
in layout(location = 4) vec3 bitangent;

// Model transformation matrix
uniform mat4 M;
uniform mat4 V;
uniform mat4 VP;
// Normal matrix
uniform mat3 NM;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 position_out;
out layout(location = 3) mat3 TBN;

void main()
{
    vec3 normalM = normalize(NM * normal_in);
    vec3 tangentM = normalize(NM * tangent);
    vec3 bitangentM = normalize(NM * bitangent);

    // Have to invert the bitangent.
    TBN = mat3(
        normalize(tangentM), 
        normalize(bitangentM), 
        normalize(normalM)
    );

    normal_out = normalM;
    textureCoordinates_out = textureCoordinates_in;



    vec4 MP = M * vec4(position, 1.0);
    vec4 viewPos = V * MP;
    position_out = viewPos.xyz;

    gl_Position = VP * MP;
}
