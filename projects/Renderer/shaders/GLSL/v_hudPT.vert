#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(vertex)

//layout(set = 0, binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    gl_Position = vec4(inPos, 1.0f);
    fragTexCoord = inTexCoord;
}
