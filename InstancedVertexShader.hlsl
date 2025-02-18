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
    //matrix worldInvTrans;
}

/*
cbuffer PerMaterialData : register(b2)
{
    PBR stuff goes here
}
*/


// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	// Use the instance ID to get the correct world matrix
    matrix wvp = mul(projection, mul(view, instances[input.instanceID].world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.uv = input.uv;
    output.normal = input.normal;
    output.worldPos = mul(instances[input.instanceID].world, float4(input.localPosition, 1.0f)).xyz;
    
	return output;
}