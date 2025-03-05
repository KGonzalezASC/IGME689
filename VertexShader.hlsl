#include "ShaderStructs.hlsli"


cbuffer PerFrameData : register(b0)
{
    matrix view;
    matrix projection;
    // matrix shadowView;
    // matrix shadowProjection;
}

cbuffer PerObjectData : register(b1)
{
    matrix world;
    matrix worldInvTrans;
}

/*
cbuffer PerMaterialData : register(b2)
{
    PBR stuff goes here
}
*/


VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;
	
	//matrix 
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.uv = input.uv;
    //once lights	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
    output.normal = normalize(mul((float3x3) worldInvTrans, input.normal));
    output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
    return output;
}