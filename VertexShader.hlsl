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

cbuffer PerBoneData : register(b3)
{
    matrix BoneData[64];
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

	//matrix 
	matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.uv = input.uv;
    //once lights	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
    output.normal = input.normal;
    output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
	return output;
}

//VertexToPixel main(VertexShaderInput input)
//{
//	// Set up output
//    VertexToPixel output;

//    matrix BoneTransform = mul(input.Weights.x, BoneData[input.BoneIDs.x]);
//    BoneTransform = BoneTransform + mul(input.Weights.y, BoneData[input.BoneIDs.y]);
//    BoneTransform = BoneTransform + mul(input.Weights.z, BoneData[input.BoneIDs.z]);
//    BoneTransform = BoneTransform + mul(input.Weights.w, BoneData[input.BoneIDs.w]);

//    matrix worldViewProj = mul(mul(world, view), projection);
//    float4 PosL = mul(float4(input.localPosition, 1.0f), BoneTransform);
//    output.screenPosition = mul(PosL, worldViewProj);

//	// Calculate the world position for this vertex
//    output.worldPos = mul(PosL, world).xyz;

//    float4 NormL = mul(float4(input.normal, 0.0f), BoneTransform);
//	// Transform the normal using the world matrix
//    output.normal = mul(NormL, (float3x3) world);

//	// Remember to normalize the normal since it's probably also scaled
//    output.normal = normalize(output.normal);

//	// Pass through the uv
//    output.uv = input.uv;

//    return output;
//}