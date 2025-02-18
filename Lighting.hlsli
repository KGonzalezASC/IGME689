#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

//basic lighting functions, constants and other useful structures

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
    int Type;
    float3 Direction; 

    float Range;
    float3 Position;

    float Intensity;
    float3 Color;
    
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding; 
};


//constants

static const float PI = 3.14159265359f;
static const float TWO_PI = PI * 2.0f;
static const float HALF_PI = PI / 2.0f;
static const float QUARTER_PI = PI / 4.0f;


//utility functions

//range-based attenuation function
float Attenuate(Light light, float3 worldPos)
{
	//calculate the distance between the surface and the light
    float dist = distance(light.Position, worldPos);

	//tanged-based attenuation
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));

	//soft falloff
    return att * att;
}


//lighting

//lambert diffuse BRDF
float Diffuse(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}


//phong (specular) BRDF
float SpecularPhong(float3 normal, float3 dirToLight, float3 toCamera, float roughness)
{
	//calculate reflection vector
    float3 refl = reflect(-dirToLight, normal);

	//compare reflection vector & view vector and raise to a power
    return roughness == 1 ? 0.0f : pow(max(dot(toCamera, refl), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
}


//blinn-Phong (specular) BRDF
float SpecularBlinnPhong(float3 normal, float3 dirToLight, float3 toCamera, float roughness)
{
	//calculate halfway vector
    float3 halfwayVector = normalize(dirToLight + toCamera);

	//compare halflway vector & normal and raise to a power
    return roughness == 1 ? 0.0f : pow(max(dot(halfwayVector, normal), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
}




//light types

float3 DirLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
	//get normalize direction to the light
    float3 toLight = normalize(-light.Direction);
    float3 toCam = normalize(camPos - worldPos);

	//calculate the light amounts
    float diff = Diffuse(normal, toLight);
    float spec = SpecularPhong(normal, toLight, toCam, roughness);

	//combine
    return (diff * surfaceColor + spec) * light.Intensity * light.Color;
}


float3 PointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
	//calc light direction
    float3 toLight = normalize(light.Position - worldPos);
    float3 toCam = normalize(camPos - worldPos);

	//calculate the light amounts
    float atten = Attenuate(light, worldPos);
    float diff = Diffuse(normal, toLight);
    float spec = SpecularPhong(normal, toLight, toCam, roughness);

	//combine
    return (diff * surfaceColor + spec) * atten * light.Intensity * light.Color;
}


float3 SpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor)
{
	//calculate the spot falloff
    float3 toLight = normalize(light.Position - worldPos);
    float pixelAngle = saturate(dot(-toLight, light.Direction));
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
	
	//linear falloff as the angle to the pixel gets closer to the outer range
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);

	// Combine with the point light calculation
    return PointLight(light, normal, worldPos, camPos, roughness, surfaceColor) * spotTerm;

}

#endif