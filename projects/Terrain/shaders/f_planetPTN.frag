#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NUMLIGHTS 2

struct LightPD
{
    vec4 position;		// vec3
    vec4 direction;		// vec3
};

struct LightProps
{
    int type;			// int   0: no light   1: directional   2: point   3: spot

    vec4 ambient;		// vec3
    vec4 diffuse;		// vec3
    vec4 specular;		// vec3

    vec4 degree;		// vec3	(constant, linear, quadratic)
    vec4 cutOff;		// vec2 (cuttOff, outerCutOff)
};

layout(set = 0, binding = 1) uniform ubobject		// https://www.reddit.com/r/vulkan/comments/7te7ac/question_uniforms_in_glsl_under_vulkan_semantics/
{
    vec4 time;				// float
	LightProps light[2];
} ubo;

layout(set = 0, binding  = 2) uniform sampler2D texSampler[41];		// sampler1D, sampler2D, sampler3D

layout(location = 0) in vec3    inFragPos;				// Vertex position transformed with TBN matrix
layout(location = 1) in vec3    inPos;					// Vertex position not transformed with TBN matrix
layout(location = 2) in vec2    inUV;
layout(location = 3) in vec3    inCamPos;
layout(location = 4) in float   inSlope;
layout(location = 5) in vec3    inNormal;
layout(location = 6) in float   inDist;
layout(location = 7) in float   inHeight;
layout(location = 8) in LightPD inLight[NUMLIGHTS];

layout(location = 0) out vec4 outColor;					// layout(location=0) specifies the index of the framebuffer (usually, there's only one).


// Declarations:

vec3  getFragColor	  (vec3 albedo, vec3 normal, vec3 specularity, float roughness);
void  getTex		  (inout vec3 result, int albedo, int normal, int specular, int roughness, float scale);
vec4  triplanarTexture(sampler2D tex, float texFactor);
vec4  triplanarTextureGrad(sampler2D tex, float texFactor);	// https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Non-uniform_flow_control
vec3  triplanarNormal (sampler2D tex, float texFactor);		// https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
vec3  toRGB			  (vec3 vec);							// Transforms non-linear sRGB color to linear RGB. Note: Usually, input is non-linear sRGB, but it's automatically converted to linear RGB in the shader, and output later in sRGB.
vec3  toSRGB		  (vec3 vec);							// Transforms linear RGB color to non-linear sRGB
vec3  unpackNormal    (vec3 normal);
vec3  applyLinearFog  (vec3 fragColor, vec3 fogColor, float minDist, float maxDist);
float applyLinearFog  (float value, float fogValue, float minDist, float maxDist);
vec3  applyFog		  (vec3 fragColor, vec3 fogColor);
float applyFog		  (float value,   float fogValue);
float getTexScaling	  (float initialTexFactor, float stepSize, float mixRange, inout float texFactor1, inout float texFactor2);
float modulus		  (float dividend, float divider);		// modulus(%) = a - (b * floor(a/b))

void getTexture_Sand(inout vec3 result);
void getTexture_GrassRock(inout vec3 result);


// Definitions:

void main()
{
	//outColor = vec4(inColor, 1.0);
	//outColor = texture(texSampler[0], inTexCoord);
	//outColor = vec4(inColor * texture(texSampler, inTexCoord).rgb, 1.0);

	vec3 color;

	//getTexture_Sand(color);
	getTexture_GrassRock(color);
	
	outColor = vec4(color, 1.0);
	//outColor = vec4(abs(normalize(inNormal.xyz)), 1.0);
}


void getTexture_Sand(inout vec3 result)
{
    float slopeThreshold = 0.04;          // sand-plainSand slope threshold
    float mixRange       = 0.02;          // threshold mixing range (slope range)
    float tf             = 50;            // texture factor
	
	//float ratio;
	//if (inSlope < slopeThreshold - mixRange) ratio = 0;
	//else if(inSlope > slopeThreshold + mixRange) ratio = 1;
	//else ratio = (inSlope - (slopeThreshold - mixRange)) / (2 * mixRange);	// <<< change for clamp()
	float ratio = clamp((inSlope - slopeThreshold) / (2 * mixRange), 0.f, 1.f);
		
	vec3 dunes  = getFragColor(
						triplanarTexture(texSampler[17], tf).rgb,
						normalize(toSRGB(triplanarTexture(texSampler[18], tf).rgb) * 2.f - 1.f).rgb,
						triplanarTexture(texSampler[19], tf).rgb,
						triplanarTexture(texSampler[20], tf).r * 255 );
						
	vec3 plains = getFragColor(
						triplanarTexture(texSampler[21], tf).rgb,
						normalize(toSRGB(triplanarTexture(texSampler[22], tf).rgb) * 2.f - 1.f).rgb,
						triplanarTexture(texSampler[23], tf).rgb,
						triplanarTexture(texSampler[24], tf).r * 255 );

	result = (ratio) * plains + (1-ratio) * dunes;
}

void getTexture_GrassRock(inout vec3 result)
{
	float tf[2];
	float ratio = getTexScaling(10, 40, 0.1, tf[0], tf[1]);	// initialTF, stepSize, mixRange, resultingTFs

	vec3 grass  = getFragColor(
						triplanarTexture(texSampler[5], tf[0]).rgb,
						//unpackNormal(triplanarTexture(texSampler[6], tf[0])),
						triplanarNormal(texSampler[6], tf[0]),
						triplanarTexture(texSampler[7], tf[0]).rgb,
						triplanarTexture(texSampler[8], tf[0]).r * 255 );
		
	vec3 grass1  = getFragColor(
						triplanarTexture(texSampler[5], tf[1]).rgb,
						//unpackNormal(triplanarTexture(texSampler[6], tf[1])),
						triplanarNormal(texSampler[6], tf[1]),
						triplanarTexture(texSampler[7], tf[1]).rgb,
						triplanarTexture(texSampler[8], tf[1]).r * 255 );
	
	vec3 rock = getFragColor(
						triplanarTexture(texSampler[9], tf[0]).rgb,
						//unpackNormal(triplanarTexture(texSampler[10], tf[0])),
						triplanarNormal(texSampler[10], tf[0]),
						triplanarTexture(texSampler[11], tf[0]).rgb,
						triplanarTexture(texSampler[12], tf[0]).r * 255 );
				
	vec3 rock1 = getFragColor(
						triplanarTexture(texSampler[9], tf[1]).rgb,
						//unpackNormal(triplanarTexture(texSampler[10], tf[1])),
						triplanarNormal(texSampler[10], tf[1]),
						triplanarTexture(texSampler[11], tf[1]).rgb,
						triplanarTexture(texSampler[12], tf[1]).r * 255 );
		
	vec3 snow = getFragColor(
						triplanarTexture(texSampler[34], tf[0]).rgb,
						//unpackNormal(triplanarTexture(texSampler[35], tf[0]).rgb),
						triplanarNormal(texSampler[35], tf[0]),
						triplanarTexture(texSampler[36], tf[0]).rgb,
						triplanarTexture(texSampler[37], tf[0]).r * 255 );
						
	vec3 snow1 = getFragColor(
						triplanarTexture(texSampler[34], tf[1]).rgb,
						//unpackNormal(triplanarTexture(texSampler[35], tf[1]).rgb),
						triplanarNormal(texSampler[35], tf[1]),
						triplanarTexture(texSampler[36], tf[1]).rgb,
						triplanarTexture(texSampler[37], tf[1]).r * 255 );	

	grass = (ratio) * grass + (1-ratio) * grass1;
	rock  = (ratio) * rock  + (1-ratio) * rock1;
	snow  = (ratio) * snow  + (1-ratio) * snow1;	// <<< BUG: Artifact lines between textures of different scale. Possible cause: Textures are get with non-constant tf values, which determine the texture scale. Possible solutions: (1) Not using mipmaps (and maybe AntiAliasing & Anisotropic filthering); (2) Getting all textures of all scales used; (3) Maybe using dFdx() & dFdy() properly. See more in: https://community.khronos.org/t/artifact-in-the-limit-between-textures/109162

	result = grass;
	return;

	// Grass + Rock:

	float slopeThreshold = 0.22;          // grass-rock slope threshold
    float mixRange       = 0.02;          // threshold mixing range (slope range)
	
	ratio = clamp((inSlope - (slopeThreshold - mixRange)) / (2 * mixRange), 0.f, 1.f);
	result = rock * (ratio) + grass * (1-ratio);

	// Snow:
	float radius = 2000;
	//float levels[2] = {1010, 1100};								// min/max snow height (Min: zero snow down from here. Max: Up from here, there's only snow within the maxSnowSlopw)
	//slopeThreshold  = (inHeight-levels[0])/(levels[1]-levels[0]);	// maximum slope where snow can rest
	float lat[2]      = {radius * 0.7, 2 * radius};
	slopeThreshold    = (abs(inPos.z)-lat[0]) / (lat[1]-lat[0]);
	mixRange          = 0.015;										// slope threshold mixing range
	
	ratio = clamp((inSlope - (slopeThreshold - mixRange)) / (2 * mixRange), 0.f, 1.f);
	result = result * (ratio) + snow * (1-ratio);
}


// Tools ---------------------------------------------------------------------------------------------

vec3 directionalLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
	//normal = vec3(0, 0, 1);
	
	// ----- Ambient lighting -----
	vec3 ambient = ubo.light[i].ambient.xyz * albedo;
	if(dot(inLight[i].direction.xyz, normal) > 0) return ambient;		// If light comes from below the tangent plane
	
	// ----- Diffuse lighting -----
	float diff   = max(dot(normal, -inLight[i].direction.xyz), 0.f);
	vec3 diffuse = ubo.light[i].diffuse.xyz * albedo * diff;		
	
	// ----- Specular lighting -----
	vec3 viewDir      = normalize(inCamPos - inFragPos);
	//vec3 reflectDir = normalize(reflect(inLight[i].direction.xyz, normal));
	//float spec	  = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 halfwayDir   = normalize(-inLight[i].direction.xyz + viewDir);
	float spec        = pow(max(dot(normal, halfwayDir), 0.0), roughness * 4);
	vec3 specular     = ubo.light[i].specular.xyz * specularity * spec;
	
	// ----- Result -----
	return vec3(ambient + diffuse + specular);
}


vec3 PointLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    float distance    = length(inLight[i].position.xyz - inFragPos);
    float attenuation = 1.0 / (ubo.light[i].degree[0] + ubo.light[i].degree[1] * distance + ubo.light[i].degree[2] * distance * distance);	// How light attenuates with distance
	vec3 lightDir = normalize(inFragPos - inLight[i].position.xyz);			// Direction from light source to fragment

    // ----- Ambient lighting -----
    vec3 ambient = ubo.light[i].ambient.xyz * albedo * attenuation;
	if(dot(lightDir, normal) > 0) return ambient;							// If light comes from below the tangent plane

    // ----- Diffuse lighting -----
    float diff   = max(dot(normal, -lightDir), 0.f);
    vec3 diffuse = ubo.light[i].diffuse.xyz * albedo * diff * attenuation;
	
    // ----- Specular lighting -----
	vec3 viewDir      = normalize(inCamPos - inFragPos);
	//vec3 reflectDir = normalize(reflect(lightDir, normal));
	//float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 halfwayDir   = normalize(-lightDir + viewDir);
	float spec        = pow(max(dot(normal, halfwayDir), 0.0), roughness * 4);
	vec3 specular     = ubo.light[i].specular.xyz * specularity * spec * attenuation;
	
    // ----- Result -----
    return vec3(ambient + diffuse + specular);	
}


vec3 SpotLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    float distance = length(inLight[i].position.xyz - inFragPos);
    float attenuation = 1.0 / (ubo.light[i].degree[0] + ubo.light[i].degree[1] * distance + ubo.light[i].degree[2] * distance * distance);	// How light attenuates with distance
    vec3 lightDir = normalize(inFragPos - inLight[i].position.xyz);			// Direction from light source to fragment

    // ----- Ambient lighting -----
    vec3 ambient = ubo.light[i].ambient.xyz * albedo * attenuation;
	if(dot(lightDir, normal) > 0) return ambient;							// If light comes from below the tangent plane

    // ----- Diffuse lighting -----
	float theta		= dot(lightDir, inLight[i].direction.xyz);	// The closer to 1, the more direct the light gets to fragment.
	float epsilon   = ubo.light[i].cutOff[0] - ubo.light[i].cutOff[1];
    float intensity = clamp((theta - ubo.light[i].cutOff[1]) / epsilon, 0.0, 1.0);
	float diff      = max(dot(normal, -lightDir), 0.f);
    vec3 diffuse    = ubo.light[i].diffuse.xyz * albedo * diff * attenuation * intensity;

    // ----- Specular lighting -----
	vec3 viewDir      = normalize(inCamPos - inFragPos);
	//vec3 reflectDir = normalize(reflect(lightDir, normal));
	//float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness);
	vec3 halfwayDir   = normalize(-lightDir + viewDir);
	//float spec      = pow(max(dot(viewDir, reflectDir), 0.f), roughness * 4);
	float spec        = pow(max(dot(normal, halfwayDir), 0.0), roughness * 4);
	vec3 specular     = ubo.light[i].specular.xyz * specularity * spec * attenuation * intensity;
	
    // ----- Result -----
    return vec3(ambient + diffuse + specular);	
}

// Apply the lighting type you want to a fragment
vec3 getFragColor(vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
	//albedo      = applyLinearFog(albedo, vec3(.1,.1,.1), 100, 500);
	//specularity = applyLinearFog(specularity, vec3(0,0,0), 100, 500);
	//roughness   = applyLinearFog(roughness, 0, 100, 500);

	vec3 result = vec3(0,0,0);

	for(int i = 0; i < NUMLIGHTS; i++)		// for each light source
	{
		if(ubo.light[i].type == 1)
			result += directionalLightColor	(i, albedo, normal, specularity, roughness);
		else if(ubo.light[i].type == 2)
			result += PointLightColor		(i, albedo, normal, specularity, roughness);
		else if(ubo.light[i].type == 3)
			result += SpotLightColor		(i, albedo, normal, specularity, roughness);
	}
	
	return result;
}

void getTex(inout vec3 result, int albedo, int normal, int specular, int roughness, float scale)
{
	result   = getFragColor(
				texture(texSampler[albedo], inUV/scale).rgb,
				normalize(toSRGB(texture(texSampler[normal], inUV/scale).rgb) * 2.f - 1.f).rgb,
				texture(texSampler[specular], inUV/scale).rgb, 
				texture(texSampler[roughness], inUV/scale).r * 255 );
}

vec2 unpackUV(vec2 UV, float texFactor)
{
	return UV.xy * vec2(1, -1) / texFactor;
}

vec4 triplanarTexture(sampler2D tex, float texFactor)
{
	vec4 dx = texture(tex, unpackUV(inPos.zy, texFactor));
	vec4 dy = texture(tex, unpackUV(inPos.xz, texFactor));
	vec4 dz = texture(tex, unpackUV(inPos.xy, texFactor));
	
	vec3 weights = abs(normalize(inNormal));
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

vec3 unpackNormal(vec3 normal)
{
	return normalize(toSRGB(normal) * vec3(2.f, 2.f, 2.f) - vec3(1.f, 1.f, 1.f));
}

vec3 rnmBlendUnpacked(vec3 n1, vec3 n2)		// n1 (up-normal), n2(map-normal)
{
    n1 += vec3( 0, 0, 1);
    n2 *= vec3(-1,-1, 1);
    return n1 * dot(n1, n2) / n1.z - n2;
}

vec3 triplanarNormal(sampler2D tex, float texFactor)
{
	//vec4 result = triplanarTexture(tex, texFactor);
	//return unpackNormal(result.rgb);
	//return normalize(inNormal);
	
	// Triplanar uvs
	vec2 uvX = unpackUV(inPos.zy, texFactor); // x facing plane
	vec2 uvY = unpackUV(inPos.xz, texFactor); // y facing plane
	vec2 uvZ = unpackUV(inPos.xy, texFactor); // z facing plane

	// Tangent space normal maps
	vec3 tnormalX = unpackNormal(texture(tex, uvX).xyz);
	vec3 tnormalY = unpackNormal(texture(tex, uvY).xyz);
	vec3 tnormalZ = unpackNormal(texture(tex, uvZ).xyz);
	
	// Get the sign (-1 or 1) of the surface normal
	vec3 axisSign = sign(normalize(inNormal));
	
	if(true)		// My approach
	{
		//if(axisSign.x == 1) tnormalX *= vec3(-1, 1, 1);
		//if(axisSign.y == 1) tnormalY *= vec3( 1, 1, 1);// <<< (++-)(+--)(-+-)(---)  // (-++)(+-+)(--+)(+++)
		//if(axisSign.y ==-1) tnormalY *= vec3( 1, 1, 1);
		// Ok in theory:
		//if(axisSign.z ==-1) tnormalZ *= vec3(-1, 1, 1);
		//if(axisSign.x == 1) tnormalX *= vec3(-1, 1, 1);
		tnormalZ.x *= axisSign.z;
		tnormalX.x *= -axisSign.x;
		
		return tnormalY;
	}
	if(true)		// (RNM) Reoriented Normal Mapping blend
	{
			// <<< Get absolute value of normal to ensure positive tangent "z" for blend
			vec3 absVertNormal = abs(inNormal);
			
			// <<< Swizzle world normals to match tangent space and apply RNM blend
			tnormalX = rnmBlendUnpacked(vec3(inNormal.zy, absVertNormal.x), tnormalX);
			tnormalY = rnmBlendUnpacked(vec3(inNormal.xz, absVertNormal.y), tnormalY);
			tnormalZ = rnmBlendUnpacked(vec3(inNormal.xy, absVertNormal.z), tnormalZ);
			
			// <<< Get the sign (-1 or 1) of the surface normal
			vec3 axisSign = sign(inNormal);
			
			// <<< Reapply sign to Z
			tnormalX.z *= axisSign.x;
			tnormalY.z *= axisSign.y;
			tnormalZ.z *= axisSign.z;
		
		// Triblend normals and add to world normal
		vec3 weights = abs(normalize(inNormal));
		//weights *= weights;
		weights /= weights.x + weights.y + weights.z;
		
		vec3 worldNormal = normalize(
			tnormalX.xyz * weights.x +
			tnormalY.xyz * weights.y +
			tnormalZ.xyz * weights.z +
			vec3(0,0,1)
			);
			
		return worldNormal;
	}
	else if(true)	// Naive method
	{
		vec3 dx = unpackNormal(texture(tex, inPos.zy / texFactor).xyz);
		vec3 dy = unpackNormal(texture(tex, inPos.xz / texFactor).xyz);
		vec3 dz = unpackNormal(texture(tex, inPos.xy / texFactor).xyz);
		
		vec3 weights = abs(normalize(inNormal));
		//weights *= weights;
		weights /= weights.x + weights.y + weights.z;
		
		return normalize(dx * weights.x + dy * weights.y + dz * weights.z);
	}
	else if(true)	// Basic Swizzle
	{
		// Flip tangent normal z to account for surface normal facing
		tnormalX.z *= axisSign.x;
		tnormalY.z *= axisSign.y;
		tnormalZ.z *= axisSign.z;
		
		// Swizzle tangent normals to match world orientation and triblend
		vec3 weights = abs(normalize(inNormal));
		weights *= weights;
		weights /= weights.x + weights.y + weights.z;
		
		vec3 worldNormal = normalize(
			tnormalX.zyx * weights.x +
			tnormalY.xzy * weights.y +
			tnormalZ.xyz * weights.z );
			
		return worldNormal;
	}
	else if(true)	// SimonDev
	{		
		vec3 tx = unpackNormal(texture(tex, inPos.zy / texFactor).xyz);
		vec3 ty = unpackNormal(texture(tex, inPos.xz / texFactor).xyz);
		vec3 tz = unpackNormal(texture(tex, inPos.xy / texFactor).xyz);
		
		vec3 weights = abs(normalize(inNormal));
		weights *= weights;
		weights = weights / (weights.x + weights.y + weights.z);
		
		vec3 axis = sign(inNormal);
		
		vec3 tangentX = normalize(cross(inNormal, vec3(0., axis.x, 0.)));	// z,y
		vec3 bitangentX = normalize(cross(tangentX, inNormal)) * axis.x;
		mat3 tbnX = mat3(tangentX, bitangentX, inNormal);
		
		vec3 tangentY = normalize(cross(inNormal, vec3(0., 0., axis.y)));	// x,z
		vec3 bitangentY = normalize(cross(tangentY, inNormal)) * axis.y;
		mat3 tbnY = mat3(tangentY, bitangentY, inNormal);
		
		vec3 tangentZ = normalize(cross(inNormal, vec3(0., -axis.z, 0.)));	// 
		vec3 bitangentZ = normalize(-cross(tangentZ, inNormal)) * axis.z;
		mat3 tbnZ = mat3(tangentZ, bitangentZ, inNormal);
		
		vec3 worldNormal = normalize (
			clamp(tbnX * tx, -1., 1.) * weights.x +
			clamp(tbnY * ty, -1., 1.) * weights.y +
			clamp(tbnZ * tz, -1., 1.) * weights.z );
	
		return worldNormal;
	}

	vec3 weights = abs(normalize(inNormal));
	//weights *= weights;
	weights /= weights.x + weights.y + weights.z;
		
	return normalize(tnormalX * weights.x + tnormalY * weights.y + tnormalZ * weights.z);

/*
	// Triplanar uvs
	float2 uvX = i.worldPos.zy; // x facing plane
	float2 uvY = i.worldPos.xz; // y facing plane
	float2 uvZ = i.worldPos.xy; // z facing plane
	
	// Tangent space normal maps
	half3 tnormalX = UnpackNormal(tex2D(_BumpMap, uvX));
	half3 tnormalY = UnpackNormal(tex2D(_BumpMap, uvY));
	half3 tnormalZ = UnpackNormal(tex2D(_BumpMap, uvZ));
	
	// Get absolute value of normal to ensure positive tangent "z" for blend
	half3 absVertNormal = abs(i.worldNormal);
	
	// Swizzle world normals to match tangent space and apply RNM blend
	tnormalX = rnmBlendUnpacked(half3(i.worldNormal.zy, absVertNormal.x), tnormalX);
	tnormalY = rnmBlendUnpacked(half3(i.worldNormal.xz, absVertNormal.y), tnormalY);
	tnormalZ = rnmBlendUnpacked(half3(i.worldNormal.xy, absVertNormal.z), tnormalZ);
	
	// Get the sign (-1 or 1) of the surface normal
	half3 axisSign = sign(i.worldNormal);
	
	// Reapply sign to Z
	tnormalX.z *= axisSign.x;
	tnormalY.z *= axisSign.y;
	tnormalZ.z *= axisSign.z;
	
	// Triblend normals and add to world normal
	half3 worldNormal = normalize(
		normalX.xyz * blend.x +
		normalY.xyz * blend.y +
		normalZ.xyz * blend.z +
		i.worldNormal
    );
*/
/*
	float tf = 50;            // texture factor
	
	vec3 tx = texture(tex, inFragPos.zy / tf);
	vec3 ty = texture(tex, inFragPos.xz / tf);
	vec3 tz = texture(tex, inFragPos.xy / tf);

	vec3 weights = abs(inNormal);
	weights *= weights;
	weights /= weights.x + weights.y + weights.z;
	
	vec3 axis = sign(normal);
	vec3 tangentX = normalize(cross(inNormal, vec3(0., axis.x, 0.)));
	vec3 bitangentX = normalize(cross(tangentX, inNormal)) * axis.x;
	mat3 tbnX = mat3(tangentY, bitangentY, inNormal);

	vec3 tangentY = normalize(cross(inNormal, vec3(0., 0., axis.y)));
	vec3 bitangentY = normalize(cross(tangentY, inNormal)) * axis.y;
	mat3 tbnY = mat3(tangentY, bitangentY, inNormal);

	vec3 tangentZ = normalize(cross(inNormal, vec3(0., -axis.z, 0.)));
	vec3 bitangentZ = normalize(-cross(tangentZ, inNormal)) * axis.z;
	mat3 tbnZ = mat3(tangentZ, bitangentZ, inNormal);
	
	vec3 worldNormal = normalize (
		clamp(tbnX * tx, -1., 1.) * weights.x +
		clamp(tbny * ty, -1., 1.) * weights.y +
		clamp(tbnZ * tz, -1., 1.) * weights.z);
	
	return vec4(worldNormal, 0.);
*/
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

vec3 applyLinearFog(vec3 fragColor, vec3 fogColor, float minDist, float maxDist)
{
	float minSqrRadius = minDist * minDist;
	float maxSqrRadius = maxDist * maxDist;
	vec3 diff = inFragPos - inCamPos;
	float sqrDist  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if(sqrDist > maxSqrRadius) return fogColor;
    else
    {
        float ratio  = (sqrDist - minSqrRadius) / (maxSqrRadius - minSqrRadius);
        return fragColor * (1-ratio) + fogColor * ratio;
    }
}

float applyLinearFog(float value, float fogValue, float minDist, float maxDist)
{
	float minSqrRadius = minDist * minDist;
	float maxSqrRadius = maxDist * maxDist;
	vec3 diff = inFragPos - inCamPos;
	float sqrDist  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if(sqrDist > maxSqrRadius) return fogValue;
    else
    {
        float ratio  = (sqrDist - minSqrRadius) / (maxSqrRadius - minSqrRadius);
        return value * (1-ratio) + fogValue * ratio;
    }
}

vec3 applyFog(vec3 fragColor, vec3 fogColor)
{
	float coeff[3] = { 1, 0.000000000001, 0.000000000001 };		// coefficients  ->  a + b*dist + c*dist^2
	vec3 diff = inFragPos - inCamPos;
	float sqrDist  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	
	float attenuation = 1.0 / (coeff[0] + coeff[1] * sqrDist + coeff[2] * sqrDist * sqrDist);
	return fragColor * attenuation + fogColor * (1. - attenuation);
}

float applyFog(float value, float fogValue)
{
	float coeff[3] = { 1, 0.000000000001, 0.000000000001 };		// coefficients  ->  a + b*dist + c*dist^2
	vec3 diff = inFragPos - inCamPos;
	float sqrDist  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	
	float attenuation = 1.0 / (coeff[0] + coeff[1] * sqrDist + coeff[2] * sqrDist * sqrDist);
	return value * attenuation + fogValue * (1. - attenuation);
}

float getTexScaling(float initialTexFactor, float stepSize, float mixRange, inout float texFactor1, inout float texFactor2)
{
	// initialTexFactor: Starting texture factor
	// stepSize: Size of each step (meters)
	// mixRange: Percentage of mix area with respect to max distance
	
	// Compute current and next step
	float linearStep = 1 + floor(inDist / stepSize);	// Linear step [1, inf)
	float quadraticStep = ceil(log(linearStep) / log(2));
	float step[2];
	step[0] = pow (2, quadraticStep);					// Exponential step [0, inf)
	step[1] = pow(2, quadraticStep + 1);				// Next exponential step
	
	// Get texture resolution for each section
	texFactor1 = step[0] * initialTexFactor;
	texFactor2 = step[1] * initialTexFactor;
	
	// Get mixing ratio
	float maxDist = stepSize * step[0];
	mixRange = mixRange * maxDist;						// mixRange is now an absolute value (not percentage)
	return clamp((maxDist - inDist) / mixRange, 0.f, 1.f);
}

float modulus(float dividend, float divider) { return dividend - (divider * floor(dividend/divider)); }