#Shader Vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

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
layout(location = 1) out vec2 v_TexCoord;

void main() 
{
    gl_Position = vec4(a_Position, 1.0);
//    fragColor = colors[gl_VertexIndex];
    fragColor = u_ColorBuffer.Color;
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
    outColor = vec4(fragColor, 1.0);
    outColor.rg = v_TexCoord;
    outColor.b = 0.0;

    vec4 texColor = texture(u_Texture, v_TexCoord);

    outColor = texColor;
}