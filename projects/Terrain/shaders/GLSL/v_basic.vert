#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(vertex)

#include "..\..\..\projects\Terrain\shaders\GLSL\vertexTools.vert"

layout(set = 0, binding = 0) uniform ubobject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMatrix;			// mat3
	vec4 camPos_t;				// camPos (vec3) + time (float)
	LightPD light[NUMLIGHTS];	// n * (2 * vec4)
} ubo[1];						// [i]: array of descriptors

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUVs;
//normal: layout(location = 3) in vec2 inTan;

layout(location = 0) out vec3 outPos;						// world space vertex position
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUVs;
layout(location = 3) flat out vec3 outCamPos;
//normal: layout(location = 4) out TB outTB;				// Tangents & Bitangents
layout(location = 4) flat out LightPD outLight[NUMLIGHTS];	// light positions & directions

int i = gl_InstanceIndex;

void main()
{
	vec3 pos = inPos;
	//displace: pos.x += 0.2;
	//waving: pos += vec3(1,0,0) * sin(<speed> * (ubo[i].camPos_t.w + ubo[i].model[0][0])) * (<amplitude> * inPos.z);	// move axis (0,0,1)
	
	gl_Position = ubo[i].proj * ubo[i].view * ubo[i].model * vec4(pos, 1.0);
	outPos = (ubo[i].model * vec4(pos, 1.0)).xyz;
	outNormal = mat3(ubo[i].normalMatrix) * inNormal;
	//verticalNormals: outNormal = mat3(ubo[i].normalMatrix) * vec3(0,0,1);
	outUVs = inUVs;
	outCamPos = ubo[i].camPos_t.xyz;
	
	for(int i = 0; i < NUMLIGHTS; i++) 
	{
		outLight[i].position.xyz  = ubo[i].light[i].position.xyz;						// for point & spot light
		outLight[i].direction.xyz = normalize(ubo[i].light[i].direction.xyz);			// for directional & spot light
	}
	
	//backfaceNormals: if(dot(outNormal, normalize(ubo[i].camPos_t.xyz - outPos)) < 0) outNormal *= -1;
	
	//normal: outTB = getTB(inNormal, inTan);
}