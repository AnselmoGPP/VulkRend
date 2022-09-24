#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Light
{
    int lightType;		// int   0: no light   1: directional   2: point   3: spot
	
    vec4 position;		// vec3
    vec4 direction;		// vec3

    vec4 ambient;		// vec3
    vec4 diffuse;		// vec3
    vec4 specular;		// vec3

    vec4 degree;		// vec3	(constant, linear, quadratic)
    vec4 cutOff;		// vec2 (cuttOff, outerCutOff)
};

// https://www.reddit.com/r/vulkan/comments/7te7ac/question_uniforms_in_glsl_under_vulkan_semantics/
layout(set = 0, binding = 1) uniform ubobject
{
	vec4  time;
} ubo;

layout(set = 0, binding  = 2) uniform sampler2D texSampler[1];		// sampler1D, sampler2D, sampler3D

layout(location = 0) in vec3 inFragPos;
layout(location = 1) flat in vec3 inCamPos;
layout(location = 2) in vec3 lightDir;
layout(location = 3) flat in Light inLight;

layout(location = 0) out vec4 outColor;								// layout(location=0) specifies the index of the framebuffer (usually, there's only one).

vec3 getFragColor			(vec3 albedo, vec3 normal, vec3 specularity, float roughness);
vec3 directionalLightColor	(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness);
vec3 PointLightColor		(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness);
vec3 SpotLightColor			(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness);
vec3 toRGB					(vec3 vec);		// Transforms non-linear sRGB color to linear RGB. Usually, input is non-linear sRGB, but it's automatically converted to linear RGB in the shader, and output later in sRGB.
vec3 toSRGB					(vec3 vec);		// Transforms linear RGB color to non-linear sRGB

void main()
{
	//outColor = vec4(getFragColor(vec3(0, 0, 0), vec3(0, 0, 1), vec3(0.3, 0.3, 0.3), 2.f), 0.7f);

	outColor = vec4(
				getFragColor(
					vec3(1./255., 137./255., 249./255.),
					normalize(
						toSRGB(texture(texSampler[0], vec2(               inFragPos.x,  ubo.time[0] + inFragPos.y)/40).rgb) * 2.f - 1.f +  
						toSRGB(texture(texSampler[0], vec2(               inFragPos.x, -ubo.time[0] + inFragPos.y)/60).rgb) * 2.f - 1.f +
						toSRGB(texture(texSampler[0], vec2( ubo.time[0] + inFragPos.x,                inFragPos.y)/50).rgb) * 2.f - 1.f +
						toSRGB(texture(texSampler[0], vec2(-ubo.time[0] + inFragPos.x,                inFragPos.y)/70).rgb) * 2.f - 1.f ).rgb,	
					vec3(0.3, 0.3, 0.3),
					2.f ), 
				0.7f);
}


// Apply the lighting type you want to a fragment
vec3 getFragColor(vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
	if(inLight.lightType == 1)
		return directionalLightColor(inLight, albedo, normal, specularity, roughness);
	else if(inLight.lightType == 2)
		return PointLightColor(inLight, albedo, normal, specularity, roughness);
	else if(inLight.lightType == 3)
		return SpotLightColor(inLight, albedo, normal, specularity, roughness);
	else
		return albedo;
}


vec3 directionalLightColor(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    // ----- Ambient lighting -----
    vec3 ambient = light.ambient.xyz * albedo;
	
	if(dot(lightDir, normal) > 0) return ambient;			// If light comes from below the tangent plane

    // ----- Diffuse lighting -----
    float diff   = max(dot(normal, -lightDir), 0.f);
    vec3 diffuse = light.diffuse.xyz * albedo * diff;

    // ----- Specular lighting -----
	vec3 viewDir      = normalize(inCamPos - inFragPos);
	//vec3 reflectDir = normalize(reflect(lightDir, normal));
	//float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 halfwayDir   = normalize(-lightDir + viewDir);
	float spec        = pow(max(dot(normal, halfwayDir), 0.0), roughness * 4);
	vec3 specular     = light.specular.xyz * specularity * spec;
	
    // ----- Result -----
	return vec3(ambient + diffuse + specular);
}


vec3 PointLightColor(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    float distance    = length(light.position.xyz - inFragPos);
    float attenuation = 1.0 / (light.degree[0] + light.degree[1] * distance + light.degree[2] * distance * distance);
	vec3 lightDir 	  = normalize(inFragPos - light.position.xyz);

	if(dot(lightDir, normal) > 0) return vec3(0,0,0);			// If light comes from below the tangent plane

    // ----- Ambient lighting -----
    vec3 ambient = light.ambient.xyz * albedo * attenuation;

    // ----- Diffuse lighting -----
    float diff   = max(dot(normal, -lightDir), 0.f);
    vec3 diffuse = light.diffuse.xyz * albedo * diff * attenuation;

    // ----- Specular lighting -----
	vec3 viewDir    = normalize(inCamPos - inFragPos);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 specular   = light.specular.xyz * specularity * spec * attenuation;

    // ----- Result -----
    return vec3(ambient + diffuse + specular);
}


vec3 SpotLightColor(Light light, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    float distance = length(light.position.xyz - inFragPos);
    float attenuation = 1.0 / (light.degree[0] + light.degree[1] * distance + light.degree[2] * distance * distance);
    vec3 fraglightDir = normalize(inFragPos - light.position.xyz);
	
	if(dot(fraglightDir, normal) > 0) return vec3(0,0,0);				// If light comes from below the tangent plane
	
    // ----- Ambient lighting -----
    vec3 ambient = light.ambient.xyz * albedo * attenuation;

    // ----- Diffuse lighting -----
    float theta = dot(fraglightDir, normalize(light.direction.xyz));	// The closer to 1, the more direct the light gets to fragment.
    if(theta < light.cutOff[1]) return vec3(ambient);

    float epsilon   = light.cutOff[0] - light.cutOff[1];
    float intensity = clamp((theta - light.cutOff[1]) / epsilon, 0.0, 1.0);
    float diff      = max(dot(normal, -fraglightDir), 0.0);
    vec3 diffuse    = light.diffuse.xyz * albedo * diff * attenuation * intensity;

    // ----- Specular lighting -----
	vec3 viewDir    = normalize(inCamPos - inFragPos);
	vec3 reflectDir = reflect(fraglightDir, normal);
	float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 specular   = light.specular.xyz * specularity * spec * attenuation * intensity;

    // ----- Result -----
    return vec3(ambient + diffuse + specular);
}


vec3 toRGB(vec3 vec)
{
	vec3 linear;
	
	if (vec.x <= 0.04045) linear.x = vec.x / 12.92;
	else linear.x = pow((vec.x + 0.055) / 1.055, 2.4);
	
	if (vec.y <= 0.04045) linear.y = vec.y / 12.92;
	else linear.y = pow((vec.y + 0.055) / 1.055, 2.4);
	
	if (vec.z <= 0.04045) linear.z = vec.z / 12.92;
	else linear.z = pow((vec.z + 0.055) / 1.055, 2.4);
	
	return linear;
}

vec3 toSRGB(vec3 vec)
{
	vec3 nonLinear;
	
	if (vec.x <= 0.0031308) nonLinear.x = vec.x * 12.92;
	else nonLinear.x = 1.055 * pow(vec.x, 1.0/2.4) - 0.055;
	
	if (vec.y <= 0.0031308) nonLinear.y = vec.y * 12.92;
	else nonLinear.y = 1.055 * pow(vec.y, 1.0/2.4) - 0.055;
	
	if (vec.z <= 0.0031308) nonLinear.z = vec.z * 12.92;
	else nonLinear.z = 1.055 * pow(vec.z, 1.0/2.4) - 0.055;
	
	return nonLinear;
}

// modulus(%) = a - (b * floor(a/b))