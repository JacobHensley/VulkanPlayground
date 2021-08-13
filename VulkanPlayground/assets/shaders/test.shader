#Shader Vertex
#version 450

layout(location = 0) in vec3 a_Position;

vec3 colors[4] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0)
);

layout(set = 0, binding = 0) uniform UniformBufferObject1{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo1;

layout(set = 2, binding = 1) uniform UniformBufferObject2{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo2;

layout(set = 2, binding = 2) uniform UniformBufferObject3{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo3;

layout(set = 3, binding = 3) uniform UniformBufferObject4{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo4;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = vec4(a_Position, 1.0);
    fragColor = colors[gl_VertexIndex];
}

#Shader Fragment
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(fragColor, 1.0);
}