#Shader Vertex
#version 450

layout(location = 0) in vec3 a_Position;

vec3 colors[4] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0)
);

layout(set = 0, binding = 1) uniform ColorBuffer
{
    vec3 Color;
} u_ColorBuffer;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = vec4(a_Position, 1.0);
//    fragColor = colors[gl_VertexIndex];
    fragColor = u_ColorBuffer.Color;
}

#Shader Fragment
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(fragColor, 1.0);
}