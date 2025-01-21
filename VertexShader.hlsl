#include "ShaderStructs.hlsli"


cbuffer DataFromCpu : register(b0) //register b0 is the binding point for the constant buffer setup constant buffer in game.cpp
{
    matrix world;
    //matrix worldInvTrans;
	matrix view;
	matrix projection;
    
    //matrix shadowView;
    //matrix shadowProjection;
}



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
	
	//matrix 
	matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.uv = input.uv;
    //once lights	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
    output.normal = input.normal;
    output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
	return output;
}