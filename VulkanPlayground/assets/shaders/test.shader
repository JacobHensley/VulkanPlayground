#Shader Vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec2 a_TexCoord;

layout(set = 0, binding = 1) uniform CameraBuffer
{
    mat4 View;
    mat4 Proj;
} u_CameraBuffer;

layout(push_constant) uniform constants
{
    mat4 transform;
} PushConstants;

layout(location = 1) out vec2 v_TexCoord;

void main() 
{
    gl_Position = u_CameraBuffer.Proj * u_CameraBuffer.View * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
}

#Shader Fragment
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D u_Texture;

void main() 
{
    vec4 texColor = texture(u_Texture, v_TexCoord);

    outColor = texColor;
    outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}