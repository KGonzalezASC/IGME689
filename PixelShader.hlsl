#include "Lighting.hlsli"
#include "ShaderStructs.hlsli"

#define NUM_LIGHTS 6

cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 ambientColor;
    float3 cameraPosition;
    
    Light lights[NUM_LIGHTS];
}

//uniforms buffer for textures goes here
//uniforms / constant buffers are used to pass data from the CPU to the GPU
//what differs is that for Texture2D we use the register keyword to specify the texture slot
//where as a cbuffer is just a buffer of data not a reference to a texture resource
//Texture2D SurfaceTexture : register(t0);

//output struct //needs lighting shadows emission gamma correction etc
float4 main(VertexToPixel input) : SV_TARGET
{
    //clean up un-normalized normals
    input.normal = normalize(input.normal);

	//start off with ambient
    float3 totalLight = ambientColor * colorTint;
	
	//loop and handle all lights
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
		//grab this light and normalize the direction (just in case)
        Light light = lights[i];
        light.Direction = normalize(light.Direction);

		//run the correct lighting calculation based on the light's type
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirLight(light, input.normal, input.worldPos, cameraPosition, roughness, colorTint);
                break;

            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, input.worldPos, cameraPosition, roughness, colorTint);
                break;

            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, input.worldPos, cameraPosition, roughness, colorTint);
                break;
        }
    }

	//should have the complete light contribution at this point
    return float4(totalLight, 1);
}