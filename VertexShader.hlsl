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

cbuffer BoneBuffer : register(b3)
{
    matrix boneTransforms[100];
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
	//// Set up output struct
	//VertexToPixel output;
	//
	////matrix 
	//matrix wvp = mul(projection, mul(view, world));
 //   output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
 //   output.uv = input.uv;
 //   //once lights	output.normal = normalize(mul((float3x3)worldInvTrans, input.normal));
 //   output.normal = input.normal;
 //   output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
	//return output;

    // Set up output struct
    VertexToPixel output;

    // Calculate skinning matrix
    matrix skinMatrix = boneTransforms[input.boneIndices[0]] * input.boneWeights[0];
    skinMatrix += boneTransforms[input.boneIndices[1]] * input.boneWeights[1];
    skinMatrix += boneTransforms[input.boneIndices[2]] * input.boneWeights[2];
    skinMatrix += boneTransforms[input.boneIndices[3]] * input.boneWeights[3];

    // Transform the vertex position and normal using the skinning matrix
    float4 skinnedPosition = mul(skinMatrix, float4(input.localPosition, 1.0f));
    float3 skinnedNormal = mul((float3x3)skinMatrix, input.normal);

    // Calculate the final world-view-projection matrix
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(skinnedPosition.xyz, 1.0f));
    output.uv = input.uv;
    // Once lights are added, uncomment the following line
    // output.normal = normalize(mul((float3x3)worldInvTrans, skinnedNormal));
    output.normal = skinnedNormal;
    output.worldPos = mul(world, skinnedPosition).xyz;

    return output;
}