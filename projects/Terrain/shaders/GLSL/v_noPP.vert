#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(vertex)

layout(set = 0, binding = 0) uniform ubobject {
	vec4 variable;
} ubo;

layout (location = 0) in vec3 inPos;				// NDC position. Since it's in NDCs, no MVP transformation is required-
layout (location = 1) in vec2 inUVs;

layout(location = 0) out vec2 outUVs;				// UVs

void main()
{
	//gl_Position.x = gl_Position.x * ubo.aspRatio.x;
	gl_Position = vec4(inPos, 1.0f);
    outUVs = inUVs;
}