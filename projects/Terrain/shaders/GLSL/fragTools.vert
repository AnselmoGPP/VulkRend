
// Index ------------------------------------------------------------------------
/*
	Constants;
		NUMLIGHTS
		PI
		E
		SR05
	Data structures:
		vec3 fragPos
		vec3 normal
		LightPD
		LightProps
		Light
		PreCalcValues
		TB3
		uvGradient
	Graphics:
		toRGB
		toSRGB
		unpackUV
		unpackUVmirror
		unpackNormal
		mixByHeight
	Save:
		saveP
		saveTB3
		savePN
		savePNT
		savePrecalcLightValues
	Math:
		getDist
		getSqrDist
		getLength
		getSqrLength
		getRatio
		getDirection
		getAngle
		getModulus
		lerp
		reflectRay
	Lighting:
		directionalLightColor
		PointLightColor
		SpotLightColor
		getFragColor
	Planar texture:
		cubemapTex
		getTex   (example)
	Triplanar texture:
		triplanarTexture
		triplanarNoColor
		triplanarNormal
		getGradients
		triplanarTexture_Sea
		triplanarNoColor_Sea
		triplanarNormal_Sea
	Others:
		planarNormal
		getTexScaling
		getLowResDist
		applyParabolicFog
		rand
		applyOrderedDithering
*/


// Constants ------------------------------------------------------------------------

#define NUMLIGHTS 3
#define PI 3.141592653589793238462
#define E  2.718281828459045235360
#define SR05 0.707106781	// == sqrt(0.5)     (vec2(SR05, SR05) has module == 1)
#define GAMMA 2.2


// Data structures (& global variables) ------------------------------------------------------------------------

vec3 fragPos;
vec3 normal;

// Generated in VS and passed to FS
struct LightPD
{
    vec4 position;		// vec3
    vec4 direction;		// vec3
};

// Generated in FS
struct LightProps
{
    int type;			// int   0: no light   1: directional   2: point   3: spot

    vec4 ambient;		// vec3
    vec4 diffuse;		// vec3
    vec4 specular;		// vec3

    vec4 degree;		// vec3	(constant, linear, quadratic) (for attenuation)
    vec4 cutOff;		// vec2 (cuttOff, outerCutOff)
};

// Mix of LightPD and LightProps
struct Light
{
	int type;			// int   0: no light   1: directional   2: point   3: spot

    vec3 position;		// vec3
    vec3 direction;		// vec3

    vec3 ambient;		// vec3
    vec3 diffuse;		// vec3
    vec3 specular;		// vec3

    vec3 degree;		// vec3	(constant, linear, quadratic) (for attenuation)
    vec2 cutOff;		// vec2 (cuttOff, outerCutOff)
} light[NUMLIGHTS];

// Variables used for calculating light that should be precalculated.
struct PreCalcValues
{
	vec3 halfwayDir[NUMLIGHTS];		// Bisector of the angle viewDir-lightDir
	vec3 lightDirFrag[NUMLIGHTS];	// Light direction from lightSource to fragment
	float attenuation[NUMLIGHTS];	// How light attenuates with distance (ratio)
	float intensity[NUMLIGHTS];		//
} pre;

// Tangent (T) and Bitangent (B)
struct TB
{
	vec3 tan;		// U direction in bump map
	vec3 bTan;		// V direction in bump map
	//vec3 normal;	// Z direction in tangent space
};

// Tangent (T) and Bitangent (B) of each dimension (3)
struct TB3
{
	vec3 tanX;
	vec3 bTanX;
	vec3 tanY;
	vec3 bTanY;
	vec3 tanZ;
	vec3 bTanZ;
} tb3;

// Gradients for the X and Y texture coordinates can be used for fetching the textures when non-uniform flow control exists (https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Non-uniform_flow_control).  
struct uvGradient
{
	vec2 uv;		// xy texture coords
	vec2 dFdx;		// Gradient of x coords
	vec2 dFdy;		// Gradient of y coords
};


// Math functions ------------------------------------------------------------------------

// Get distance between two 3D points
float getDist(vec3 a, vec3 b) 
{
	vec3 diff = a - b;
	return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z); 
}

// Get square distance between two 3D points
float getSqrDist(vec3 a, vec3 b) 
{
	vec3 diff = a - b;
	return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z; 
}

float getLength(vec3 a)
{
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

float getSqrLength(vec3 a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

// Get the ratio for a given "value" within a range [min, max]. Result's range: [0, 1].
float getRatio(float value, float min, float max)
{
	return clamp((value - min) / (max - min), 0, 1);
}

// Get the ratio for a given "value" within a range [min, max]. Result's range: [minR, maxR].
float getRatio(float value, float min, float max, float minR, float maxR)
{
	float ratio = clamp((value - min) / (max - min), 0, 1);
	return minR + ratio * (maxR - minR);
}

// Get a direction given 2 points.
vec3 getDirection(vec3 origin, vec3 end)
{
	return normalize(end - origin);
}

// Get angle between 2 unit vectors (directions).
float getAngle(vec3 a, vec3 b)
{
	//return acos( dot(a, b) / ((length(a) * length(b)) );	// Non-unit vectors
	return acos(dot(a, b));									// Unit vectors
}

// Get modulus(%) = a - (b * floor(a/b)) (https://registry.khronos.org/OpenGL-Refpages/gl4/html/mod.xhtml)
float getModulus(float dividend, float divider) { return dividend - (divider * floor(dividend/divider)); }

// Linear interpolation. Position between a and b located at a given ratio [0,1]
float lerp(float a, float b, float ratio) { return a + (b - a) * ratio; }

vec3 lerp(vec3 a, vec3 b, float t) { return a + (b - a) * t; }

vec3 reflectRay(vec3 camPos, vec3 fragPos, vec3 normal)
{
	return reflect(
		normalize(fragPos - camPos),	// incident ray
		normal);
}

// Graphic functions ------------------------------------------------------------------------

// Transforms non-linear sRGB color to linear RGB. Note: Usually, input is non-linear sRGB, but it's automatically converted to linear RGB in the shader, and output later in sRGB.
vec3 toRGB(vec3 vec)
{
	//return pow(vec, vec3(1.0/GAMMA));
	
	vec3 linear;
	
	if (vec.x <= 0.04045) linear.x = vec.x / 12.92;
	else linear.x = pow((vec.x + 0.055) / 1.055, 2.4);
	
	if (vec.y <= 0.04045) linear.y = vec.y / 12.92;
	else linear.y = pow((vec.y + 0.055) / 1.055, 2.4);
	
	if (vec.z <= 0.04045) linear.z = vec.z / 12.92;
	else linear.z = pow((vec.z + 0.055) / 1.055, 2.4);
	
	return linear;
}

vec4 toRGB(vec4 vec)
{
	//return pow(vec, vec3(1.0/GAMMA));
	
	vec4 linear;
	
	if (vec.x <= 0.04045) linear.x = vec.x / 12.92;
	else linear.x = pow((vec.x + 0.055) / 1.055, 2.4);
	
	if (vec.y <= 0.04045) linear.y = vec.y / 12.92;
	else linear.y = pow((vec.y + 0.055) / 1.055, 2.4);
	
	if (vec.z <= 0.04045) linear.z = vec.z / 12.92;
	else linear.z = pow((vec.z + 0.055) / 1.055, 2.4);
	
	linear.w = vec.w;
	
	return linear;
}

// Transforms linear RGB color to non-linear sRGB
vec3 toSRGB(vec3 vec)
{
	//return pow(vec, vec3(GAMMA));
	
	vec3 nonLinear;
	
	if (vec.x <= 0.0031308) nonLinear.x = vec.x * 12.92;
	else nonLinear.x = 1.055 * pow(vec.x, 1.0/2.4) - 0.055;
	
	if (vec.y <= 0.0031308) nonLinear.y = vec.y * 12.92;
	else nonLinear.y = 1.055 * pow(vec.y, 1.0/2.4) - 0.055;
	
	if (vec.z <= 0.0031308) nonLinear.z = vec.z * 12.92;
	else nonLinear.z = 1.055 * pow(vec.z, 1.0/2.4) - 0.055;
	
	return nonLinear;
}

vec4 toSRGB(vec4 vec)
{
	//return pow(vec, vec3(GAMMA));
	
	vec4 nonLinear;
	
	if (vec.x <= 0.0031308) nonLinear.x = vec.x * 12.92;
	else nonLinear.x = 1.055 * pow(vec.x, 1.0/2.4) - 0.055;
	
	if (vec.y <= 0.0031308) nonLinear.y = vec.y * 12.92;
	else nonLinear.y = 1.055 * pow(vec.y, 1.0/2.4) - 0.055;
	
	if (vec.z <= 0.0031308) nonLinear.z = vec.z * 12.92;
	else nonLinear.z = 1.055 * pow(vec.z, 1.0/2.4) - 0.055;
	
	nonLinear.w = vec.w;
	
	return nonLinear;
}

// Invert Y axis (for address mode == repeat) and apply scale
vec2 unpackUV(vec2 UV, float texFactor)
{
	return UV.xy * vec2(1, -1) / texFactor;
}

// Invert Y axis (for address mode == mirror) and apply scale
vec2 unpackUVmirror(vec2 UV, float texFactor)
{
	return (vec2(0, 1) + UV.xy * vec2(1, -1)) / texFactor;
}

// Correct color space (to linear), put in range [-1,1], and normalize
vec3 unpackNormal(vec3 normal)
{
	return normalize(toSRGB(normal) * 2.f - 1.f);		// Color space correction is required to counter gamma correction.
}

// Correct color space (to linear), put in range [-1,1], normalize, and adjust normal strength.
vec3 unpackNormal(vec3 normal, float multiplier)
{
	normal = normalize(toSRGB(normal) * 2.f - 1.f);		// Color space correction is required to counter gamma correction.

	normal.z -= (1 - normal.z) * multiplier;
	normal.z = clamp(normal.z, 0, 1);
	
	return normalize(normal);

	// ------------------

	normal = normalize(toSRGB(normal) * 2.f - 1.f);

	float phi   = atan(normal.y / normal.x);
	float theta = atan(sqrt(normal.x * normal.x + normal.y * normal.y), normal.z);

	//theta = clamp(theta * multiplier, 0, PI/2);
	
	//if(theta < 0.1) return normal;

	return normalize(
			vec3(
				sin(theta) * cos(phi),
				sin(theta) * sin(phi),
				cos(theta) ));
}

// Mix 2 textures based on their height maps.
vec3 mixByHeight(vec3 tex_A, vec3 tex_B, float height_A, float height_B, float ratio, float depth)
{
	float ma = max(height_B  + ratio, height_A + (1-ratio)) - depth;
	float b1 = max(height_B  + ratio     - ma, 0);
	float b2 = max(height_A + (1-ratio) - ma, 0);
	return (tex_B * b1 + tex_A * b2) / (b1 + b2);
}

// Save functions ------------------------------------------------------------------------
// They store shader variables in this library, making them global for this library and allowing it to use them.

// Save fragment position.
//void saveP(vec3 pos) { fragPos = pos; }

// Save Tangent and Bitangent vectors.
//void saveTB3(TB3 tb_3) { tb3 = tb_3; }

// Save fragment position, normal, Tangent, and Bitangent.
void savePNT(vec3 pos, vec3 norm, TB3 tb_3)
{
	fragPos = pos;
	normal  = normalize(norm);
	tb3     = tb_3;				// Tangent & Bitangent
}

// (UNOPTIMIZED) Precalculate (to avoid repeating calculations) and save (for making them global for this library) variables required for calculating light.
void savePrecalcLightValues(vec3 fragPos, vec3 camPos, LightProps uboLight[NUMLIGHTS], LightPD inLight[NUMLIGHTS])
{
	vec3 viewDir = normalize(camPos - fragPos);	// Camera view direction
	float distFragLight;						// Distance fragment-lightSource
	float theta;								// The closer to 1, the more direct the light gets to fragment.
	float epsilon;								// Cutoff range
	
	for(int i = 0; i < NUMLIGHTS; i++)
	{
		light[i].type      = uboLight[i].type;

		light[i].position  = inLight[i].position.xyz;
		light[i].direction = inLight[i].direction.xyz;
	
		light[i].ambient   = uboLight[i].ambient.xyz;
		light[i].diffuse   = uboLight[i].diffuse.xyz;
		light[i].specular  = uboLight[i].specular.xyz;
	
		light[i].degree    = uboLight[i].degree.xyz;
		light[i].cutOff    = uboLight[i].cutOff.xy;
		
		switch(light[i].type)
		{
		case 1:		// directional
			pre.halfwayDir[i]	= normalize(-light[i].direction + viewDir);
			break;
		case 2:		// point
			distFragLight		= length(light[i].position - fragPos);
			pre.lightDirFrag[i]	= normalize(fragPos - light[i].position);
			pre.halfwayDir[i]   = normalize(-pre.lightDirFrag[i] + viewDir);
			pre.attenuation[i]  = 1.0 / (light[i].degree[0] + light[i].degree[1] * distFragLight + light[i].degree[2] * distFragLight * distFragLight);
			break;
		case 3:		// spot
			distFragLight		= length(light[i].position - fragPos);
			pre.lightDirFrag[i]	= normalize(fragPos - light[i].position);
			pre.halfwayDir[i]   = normalize(-pre.lightDirFrag[i] + viewDir);
			pre.attenuation[i]	= 1.0 / (light[i].degree[0] + light[i].degree[1] * distFragLight + light[i].degree[2] * distFragLight * distFragLight);
			theta				= dot(pre.lightDirFrag[i], light[i].direction);
			epsilon				= light[i].cutOff[0] - light[i].cutOff[1];
			pre.intensity[i]	= clamp((theta - light[i].cutOff[1]) / epsilon, 0.0, 1.0);
			break;
		default:
			break;
		}
	}
}


// (For planets) Reduce sunlight intensity (diffuse & specular) if sunlight source is below the object.
void modifySavedSunLight(vec3 fragPos)
{
	float ratio = getRatio(dot(normalize(light[0].direction), normalize(fragPos)), 0.3, -0.8);

	//light[0].ambient  *= ratio;
	light[0].diffuse  *= ratio;
	light[0].specular *= ratio;
}


// Lightning functions ------------------------------------------------------------------------

// Directional light (sun)
vec3 directionalLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
	// ----- Ambient lighting -----
	vec3 ambient = light[i].ambient * albedo;
	if(dot(light[i].direction, normal) > 0) return ambient;			// If light comes from below the tangent plane
	
	// ----- Diffuse lighting -----
	float diff   = max(dot(normal, -light[i].direction), 0.f);
	vec3 diffuse = light[i].diffuse * albedo * diff;		
	
	// ----- Specular lighting -----
	float spec		= pow(max(dot(normal, pre.halfwayDir[i]), 0.0), roughness * 4);
	vec3 specular	= light[i].specular * specularity * spec;
	
	// ----- Result -----
	return vec3(ambient + diffuse + specular);
}

// Point light (bulb)
vec3 PointLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    // ----- Ambient lighting -----
    vec3 ambient = light[i].ambient * albedo * pre.attenuation[i];
	if(dot(pre.lightDirFrag[i], normal) > 0) return ambient;				// If light comes from below the tangent plane

    // ----- Diffuse lighting -----
    float diff   = max(dot(normal, -pre.lightDirFrag[i]), 0.f);
    vec3 diffuse = light[i].diffuse * albedo * diff * pre.attenuation[i];
	
    // ----- Specular lighting -----
	float spec        = pow(max(dot(normal, pre.halfwayDir[i]), 0.0), roughness * 4);
	vec3 specular     = light[i].specular * specularity * spec * pre.attenuation[i];
	
    // ----- Result -----
    return vec3(ambient + diffuse + specular);	
}

// Spot light (spotlight)
vec3 SpotLightColor(int i, vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
    // ----- Ambient lighting -----
    vec3 ambient = light[i].ambient * albedo * pre.attenuation[i];
	if(dot(pre.lightDirFrag[i], normal) > 0) return ambient;				// If light comes from below the tangent plane

    // ----- Diffuse lighting -----
	float diff      = max(dot(normal, -pre.lightDirFrag[i]), 0.f);
    vec3 diffuse    = light[i].diffuse * albedo * diff * pre.attenuation[i] * pre.intensity[i];

    // ----- Specular lighting -----
	float spec        = pow(max(dot(normal, pre.halfwayDir[i]), 0.0), roughness * 4);
	vec3 specular     = light[i].specular * specularity * spec * pre.attenuation[i] * pre.intensity[i];
	
    // ----- Result -----
    return vec3(ambient + diffuse + specular);
}

// Apply the lights to a fragment
vec3 getFragColor(vec3 albedo, vec3 normal, vec3 specularity, float roughness)
{
	//albedo      = applyLinearFog(albedo, vec3(.1,.1,.1), 100, 500);
	//specularity = applyLinearFog(specularity, vec3(0,0,0), 100, 500);
	//roughness   = applyLinearFog(roughness, 0, 100, 500);

	vec3 result = vec3(0,0,0);

	for(int i = 0; i < NUMLIGHTS; i++)		// for each light source
	{
		switch(light[i].type)
		{
		case 1:
			result += directionalLightColor	(i, albedo, normal, specularity, roughness);
			break;
		case 2:
			result += PointLightColor		(i, albedo, normal, specularity, roughness);
			break;
		case 3:
			result += SpotLightColor		(i, albedo, normal, specularity, roughness);
			break;
		default:
			break;
		}
	}
	
	return result;
}

// Get axis towards a direction vector is closer.
vec3 getMajorAxis(vec3 dir)
{
	vec3 mAxis = vec3(abs(dir.x), abs(dir.y), abs(dir.z));
	
	if(mAxis.x > mAxis.y) 
	{
		if(mAxis.x > mAxis.z) return normalize(vec3(dir.x, 0, 0));
		else return normalize(vec3(0, 0, dir.z));
	}
	else if(mAxis.y > mAxis.z) return normalize(vec3(0, dir.y, 0));
	else return normalize(vec3(0, 0, dir.z));
}

// Get axis towards a reflected direction vector is closer.
//vec3 getMajorAxis(vec3 fragPos, vec3 camPos, vec3 normal)
//{
//	vec3 I = normalize(fragPos - camPos);
//	vec3 R = reflect(I, normal);
//	return getMajorAxis(R);
//}

// Get UV coordinates from str coordinates (s, t, r).
vec2 getUVsFromCube(vec3 str)
{
	return vec2(
		0.5 + 0.5 * str.x / abs(str.z), 
		0.5 + 0.5 * str.y / abs(str.z)
	);
}

// Use a vector (fragment's position in a cube) to sample from a cubemap
vec3 cubemapTex(vec3 pos, sampler2D front, sampler2D back, sampler2D up, sampler2D down, sampler2D right, sampler2D left)
{
	pos = normalize(pos);
	vec3 majorAxis = getMajorAxis(pos);
	
	vec2 uv;
	
	if(majorAxis.x != 0.f)
	{
		if(majorAxis.x > 0.f) {
			uv = getUVsFromCube(vec3(-pos.z, -pos.y, pos.x));
			return texture(front, uv).rgb;
			}
		else {
			uv = getUVsFromCube(vec3(pos.z, -pos.y, pos.x));
			return texture(back, uv).rgb;
			}
	}
	else if(majorAxis.y != 0.f)
	{
		if(majorAxis.y > 0.f) {
			uv = getUVsFromCube(vec3(pos.x, pos.z, pos.y));
			return texture(left, uv).rgb;
			}
		else {
			uv = getUVsFromCube(vec3(pos.x, -pos.z, pos.y));
			return texture(right, uv).rgb;
			}
	}
	else if(majorAxis.z != 0.f)
	{
		if(majorAxis.z > 0.f) {
			uv = getUVsFromCube(vec3(pos.x, -pos.y, pos.z));
			return texture(up, uv).rgb;
			}
		else {
			uv = getUVsFromCube(vec3(-pos.x, -pos.y, pos.z));
			return texture(down, uv).rgb;
			}
	}
	
	return vec3(0,0,0);
}

// (EXAMPLE) Get fragment color given 4 texture maps (albedo, normal, specular, roughness).
vec3 getTex(sampler2D albedo, sampler2D normal, sampler2D specular, sampler2D roughness, float scale, vec2 UV)
{
	return getFragColor(
				texture(albedo, unpackUV(UV, scale)).rgb,
				unpackNormal(texture(normal, unpackUV(UV, scale)).rgb),
				texture(specular, unpackUV(UV, scale)).rgb, 
				texture(roughness, unpackUV(UV, scale)).r * 255 );
}


// Triplanar functions ------------------------------------------------------------------------

// Texture projected from 3 axes (x,y,z) and mixed.
vec4 triplanarTexture(sampler2D tex, float texFactor)
{
	vec4 dx = texture(tex, unpackUV(fragPos.zy, texFactor));
	vec4 dy = texture(tex, unpackUV(fragPos.xz, texFactor));
	vec4 dz = texture(tex, unpackUV(fragPos.xy, texFactor));
	
	vec3 weights = abs(normalize(normal));
	weights *= weights;
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

// Non-coloring texture projected from 3 axes (x, y, z) and mixed. 
vec4 triplanarNoColor(sampler2D tex, float texFactor)
{
	vec4 dx = toSRGB(texture(tex, unpackUV(fragPos.zy, texFactor)));	// Color space correction (textures are converted from sRGB to RGB, but non-coloring textures are in RGB already).
	vec4 dy = toSRGB(texture(tex, unpackUV(fragPos.xz, texFactor)));
	vec4 dz = toSRGB(texture(tex, unpackUV(fragPos.xy, texFactor)));
	
	vec3 weights = abs(normalize(normal));
	weights *= weights;
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

// Normal map projected from 3 axes (x,y,z) and mixed.
// https://bgolus.medium.com/normal-mapping-for-a-triplanar-shader-10bf39dca05a
vec3 triplanarNormal(sampler2D tex, float texFactor)
{	
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	vec3 tnormalX = unpackNormal(texture(tex, unpackUV(fragPos.zy, texFactor)).xyz);
	vec3 tnormalY = unpackNormal(texture(tex, unpackUV(fragPos.xz, texFactor)).xyz);
	vec3 tnormalZ = unpackNormal(texture(tex, unpackUV(fragPos.xy, texFactor)).xyz);
	
	// Fix X plane projection over positive X axis
	vec3 axis = sign(normal);
	tnormalX.x *= -axis.x;	
	
	// World space normals
	tnormalX = mat3(tb3.tanX, tb3.bTanX, normal) * tnormalX;	// TBN_X * tnormalX
	tnormalY = mat3(tb3.tanY, tb3.bTanY, normal) * tnormalY;	// TBN_Y * tnormalY
	tnormalZ = mat3(tb3.tanZ, tb3.bTanZ, normal) * tnormalZ;	// TBN_Z * tnormalZ
	
	// Weighted average
	vec3 weights = abs(normal);
	weights *= weights;
	weights /= weights.x + weights.y + weights.z;
	
	return normalize(tnormalX * weights.x  +  tnormalY * weights.y  +  tnormalZ * weights.z);
}

// Dynamic texture map projected from 3 axes (x,y,z) and mixed. 
vec4 triplanarTexture_Sea(sampler2D tex, float texFactor, float speed, float t)
{	
	// Get normal map coordinates
	float time = t * speed;
	float offset = texFactor / 4;
	const int n = 3;
	
	vec2 coordsXY[n] = { 
		vec2(       fragPos.x, time + fragPos.y + offset),
		vec2(time + fragPos.x,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.y) };

	vec2 coordsZY[n] = {
		vec2(       fragPos.z, time + fragPos.y + offset),
		vec2(time + fragPos.z,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.z, time * SR05 + fragPos.y) };
		
	vec2 coordsXZ[n] = {
		vec2(       fragPos.x, time + fragPos.z + offset),
		vec2(time + fragPos.x,        fragPos.z - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.z) };
		
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	vec4 dx = {0,0,0,0};
	vec4 dy = {0,0,0,0};
	vec4 dz = {0,0,0,0};
	int i = 0;
		
	for(i = 0; i < n; i++) 
		dx += texture(tex, unpackUV(coordsZY[i], texFactor));	
	dx = normalize(dx);

	for(i = 0; i < n; i++) 
		dy += texture(tex, unpackUV(coordsXZ[i], texFactor));
	dy = normalize(dy);
		
	for(i = 0; i < n; i++)
		dz += texture(tex, unpackUV(coordsXY[i], texFactor));
	dz = normalize(dz);
	
	//vec4 dx = texture(tex, unpackUV(fragPos.zy, texFactor));
	//vec4 dy = texture(tex, unpackUV(fragPos.xz, texFactor));
	//vec4 dz = texture(tex, unpackUV(fragPos.xy, texFactor));
	
	// Weighted average
	vec3 weights = abs(normalize(normal));
	weights *= weights;
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

// Dynamic texture map projected from 3 axes (x,y,z) and mixed. 
vec4 triplanarNoColor_Sea(sampler2D tex, float texFactor, float speed, float t)
{	
	// Get normal map coordinates
	float time = t * speed;
	float offset = texFactor / 4;
	const int n = 3;
	
	vec2 coordsXY[n] = { 
		vec2(       fragPos.x, time + fragPos.y + offset),
		vec2(time + fragPos.x,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.y) };

	vec2 coordsZY[n] = {
		vec2(       fragPos.z, time + fragPos.y + offset),
		vec2(time + fragPos.z,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.z, time * SR05 + fragPos.y) };
		
	vec2 coordsXZ[n] = {
		vec2(       fragPos.x, time + fragPos.z + offset),
		vec2(time + fragPos.x,        fragPos.z - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.z) };
		
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	vec4 dx = {0,0,0,0};
	vec4 dy = {0,0,0,0};
	vec4 dz = {0,0,0,0};
	vec4 temp;
	int i = 0;
		
	for(i = 0; i < n; i++) 
		dx += toSRGB(texture(tex, unpackUV(coordsZY[i], texFactor)));		// Color space correction (textures are converted from sRGB to RGB, but non-coloring textures are in RGB already).
	dx = normalize(dx);

	for(i = 0; i < n; i++) 
		dy += toSRGB(texture(tex, unpackUV(coordsXZ[i], texFactor)));
	dy = normalize(dy);
		
	for(i = 0; i < n; i++)
		dz += toSRGB(texture(tex, unpackUV(coordsXY[i], texFactor)));
	dz = normalize(dz);
	
	//vec4 dx = texture(tex, unpackUV(fragPos.zy, texFactor));
	//vec4 dy = texture(tex, unpackUV(fragPos.xz, texFactor));
	//vec4 dz = texture(tex, unpackUV(fragPos.xy, texFactor));
	
	// Weighted average
	vec3 weights = abs(normalize(normal));
	weights *= weights;
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

// Dynamic normal map projected from 3 axes (x,y,z) and mixed. 
vec3 triplanarNormal_Sea(sampler2D tex, float texFactor, float speed, float t)
{	
	// Get normal map coordinates
	float time = t * speed;
	float offset = texFactor / 4;
	const int n = 3;
	
	vec2 coordsXY[n] = { 
		vec2(       fragPos.x, time + fragPos.y + offset),
		vec2(time + fragPos.x,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.y) };

	vec2 coordsZY[n] = {
		vec2(       fragPos.z, time + fragPos.y + offset),
		vec2(time + fragPos.z,        fragPos.y - offset),
		vec2(time * SR05 + fragPos.z, time * SR05 + fragPos.y) };
		
	vec2 coordsXZ[n] = {
		vec2(       fragPos.x, time + fragPos.z + offset),
		vec2(time + fragPos.x,        fragPos.z - offset),
		vec2(time * SR05 + fragPos.x, time * SR05 + fragPos.z) };
		
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	vec3 tnormalX = {0,0,0};
	vec3 tnormalY = {0,0,0};
	vec3 tnormalZ = {0,0,0};
	int i = 0;
		
	for(i = 0; i < n; i++) 
		tnormalX += unpackNormal(texture(tex, unpackUV(coordsZY[i], texFactor)).xyz, 7);
	tnormalX = normalize(tnormalX);

	for(i = 0; i < n; i++) 
		tnormalY += unpackNormal(texture(tex, unpackUV(coordsXZ[i], texFactor)).xyz, 7);
	tnormalY = normalize(tnormalY);
		
	for(i = 0; i < n; i++)
		tnormalZ += unpackNormal(texture(tex, unpackUV(coordsXY[i], texFactor)).xyz, 7);
	tnormalZ = normalize(tnormalZ);
	
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	//vec3 tnormalX = unpackNormal(texture(tex, unpackUV(inPos.zy, texFactor)).xyz);
	//vec3 tnormalY = unpackNormal(texture(tex, unpackUV(inPos.xz, texFactor)).xyz);
	//vec3 tnormalZ = unpackNormal(texture(tex, unpackUV(inPos.xy, texFactor)).xyz);
	
	// Fix X plane projection over positive X axis
	vec3 axis = sign(normal);
	tnormalX.x *= -axis.x;	
	
	// World space normals
	tnormalX = mat3(tb3.tanX, tb3.bTanX, normal) * tnormalX;	// TBN_X * tnormalX
	tnormalY = mat3(tb3.tanY, tb3.bTanY, normal) * tnormalY;	// TBN_Y * tnormalY
	tnormalZ = mat3(tb3.tanZ, tb3.bTanZ, normal) * tnormalZ;	// TBN_Z * tnormalZ
	
	// Weighted average
	vec3 weights = abs(normal);
	weights *= weights;
	weights /= weights.x + weights.y + weights.z;
	
	return normalize(tnormalX * weights.x  +  tnormalY * weights.y  +  tnormalZ * weights.z);
}

// (EXAMPLE) Get triplanar fragment color given 4 texture maps (albedo, normal, specular, roughness).
vec3 getTriTex(sampler2D albedo, sampler2D normal, sampler2D specular, sampler2D roughness, float scale, vec2 UV)
{
	return getFragColor(
				triplanarTexture(albedo, scale).rgb,
				triplanarNormal (normal, scale),
				triplanarTexture(specular, scale).rgb,
				triplanarTexture(roughness, scale).r * 255 );
}

// Get gradients
uvGradient getGradients(vec2 uvs)
{
	uvGradient result;
	result.uv = uvs;
	result.dFdx = dFdx(uvs);
	result.dFdy = dFdy(uvs);
	return result;
}

// Triplanar texture using gradients
vec4 triplanarTexture(sampler2D tex, uvGradient dzy, uvGradient dxz, uvGradient dxy)
{
	vec4 dx = textureGrad(tex, dzy.uv, dzy.dFdx, dzy.dFdy);
	vec4 dy = textureGrad(tex, dxz.uv, dxz.dFdx, dxz.dFdy);
	vec4 dz = textureGrad(tex, dxy.uv, dxy.dFdx, dxy.dFdy);
	
	vec3 weights = abs(normalize(normal));
	weights *= weights;
	weights = weights / (weights.x + weights.y + weights.z);

	return dx * weights.x + dy * weights.y + dz * weights.z;
}

// Triplanar normals using gradients
vec3 triplanarNormal(sampler2D tex, uvGradient dzy, uvGradient dxz, uvGradient dxy)
{	
	// Tangent space normal maps (retrieved using triplanar UVs; i.e., 3 facing planes)
	vec3 tnormalX = unpackNormal(texture(tex, dzy.uv).xyz);
	vec3 tnormalY = unpackNormal(texture(tex, dxz.uv).xyz);
	vec3 tnormalZ = unpackNormal(texture(tex, dxy.uv).xyz);
	
	// Fix X plane projection over positive X axis
	vec3 axis = sign(normal);
	tnormalX.x *= -axis.x;	
	
	// World space normals
	tnormalX = mat3(tb3.tanX, tb3.bTanX, normal) * tnormalX;	// TBN_X * tnormalX
	tnormalY = mat3(tb3.tanY, tb3.bTanY, normal) * tnormalY;	// TBN_Y * tnormalY
	tnormalZ = mat3(tb3.tanZ, tb3.bTanZ, normal) * tnormalZ;	// TBN_Z * tnormalZ
	
	// Weighted average
	vec3 weights = abs(normal);
	weights *= weights;
	weights /= weights.x + weights.y + weights.z;
		
	return normalize(tnormalX * weights.x  +  tnormalY * weights.y  +  tnormalZ * weights.z);
}

// Others ------------------------------------------------------------------------

// Get normal from the vertex normal (world normal) and a normal map (tangent space normal).
vec3 planarNormal(sampler2D tex, vec2 UVs, TB tb, vec3 normal, float texFactor)
{	
	// Tangen space normal
	vec3 tnormal = unpackNormal(texture(tex, unpackUV(UVs, texFactor)).xyz);
	
	// Final World space normal
	return mat3(tb.tan, tb.bTan, normal) * tnormal;	// TBN * tnormal  (convert tangent space normal to world space)
}

// Get ratio (return value) of 2 different texture resolutions (texFactor1, texFactor2). Used for getting textures of different resolutions depending upon distance to fragment, and for mixing them.
float getTexScaling(float fragDist, float initialTexFactor, float baseDist, float mixRange, inout float texFactor1, inout float texFactor2)
{
	// Compute current and next step
	float linearStep = 1 + floor(fragDist / baseDist);	// Linear step [1, inf)
	float quadraticStep = ceil(log(linearStep) / log(2));
	float step[2];
	step[0] = pow (2, quadraticStep);					// Exponential step [0, inf)
	step[1] = pow(2, quadraticStep + 1);				// Next exponential step
	
	// Get texture resolution for each section
	texFactor1 = step[0] * initialTexFactor;
	texFactor2 = step[1] * initialTexFactor;
	
	// Get mixing ratio
	float maxDist = baseDist * step[0];
	mixRange = mixRange * maxDist;						// mixRange is now an absolute value (not percentage)
	return clamp((maxDist - fragDist) / mixRange, 0.f, 1.f);
}

// Get a distance that increases with the distance between cam and center of a sphere. Useful for getting a distance from which non-visible things can be poorly rendered.
float getLowResDist(float camSqrHeight, float radius, float minDist)
{
	float lowResDist = sqrt(camSqrHeight - radius * radius);
	
	if(lowResDist > minDist) return lowResDist;
	else return minDist;
}

// Apply a fogColor to the fragment depending upon distance.
vec3 applyParabolicFog(vec3 fragColor, vec3 fogColor, float minDist, float maxDist, vec3 fragPos, vec3 camPos)
{
	float minSqrRadius = minDist * minDist;
	float maxSqrRadius = maxDist * maxDist;
	vec3 diff = fragPos - camPos;
	float sqrDist  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	float ratio = getRatio(sqrDist, minSqrRadius, maxSqrRadius);
	//float attenuation = 1.0 / (coeff[0] + coeff[1] * sqrDist + coeff[2] * sqrDist * sqrDist);
		
	return fragColor * (1-ratio) + fogColor * ratio;
	//return fragColor * attenuation + fogColor * (1. - attenuation);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const mat4 thresholdMap = {		// for dithering
	{0.,     8/16.,  2/16.,  10/16.}, 
	{12/16., 4/16.,  14/16., 6/16.}, 
	{3/16.,  11/16., 1/16.,  9/16.}, 
	{15/16., 7/16.,  13/16., 5/16.}
};

bool applyOrderedDithering(float value, float minValue, float maxValue)
{
	float ratio = getRatio(value, minValue, maxValue);
	ivec2 index = { int(mod(gl_FragCoord.x, 4)), int(mod(gl_FragCoord.y, 4)) };
	
	if(ratio > thresholdMap[index.x][index.y]) return true;
	return false;
}