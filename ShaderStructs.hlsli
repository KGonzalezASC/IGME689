#ifndef __GGP_SHADER_STRUCTS__
#define __GPP_SHADER_STRUCTS__

#define MAX_INSTANCES 1000

// Structs for various shaders
// Basic VS input for a standard Pos/UV/Normal vertex
struct VertexShaderInput
{
    float3 localPosition : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint instanceID : SV_InstanceID;
    //float3 tangent : TANGENT;
};

struct InstanceData
{
    matrix world;
};

cbuffer PerInstanceData : register(b2)
{
    InstanceData instances[MAX_INSTANCES];
};

// VS Output / PS Input struct for basic lighting
struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    //float3 tangent : TANGENT;
    float3 worldPos : POSITION;
    //float4 posForShadow : SHADOWPOS; // Position for shadow mapping 
};

/*
struct VertexToPixel_Sky
{
    float4 screenPos : SV_POSITION;
    float3 sampleDir : DIRECTION;
};
*/

#endif