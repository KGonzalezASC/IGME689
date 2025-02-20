#include "ShaderStructs.hlsli"

// Per-frame matrices
cbuffer PerFrameData : register(b0)
{
    matrix view;
    matrix projection;
}

// Per-object transform
cbuffer PerObjectData : register(b1)
{
    matrix world;
}

// Bone transforms constant buffer (support up to 100 bones)
cbuffer BoneBuffer : register(b3)
{
    matrix boneTransforms[100];
}

VertexToPixel main(VertexShaderInput input)
{
    VertexToPixel output;
    
    // Skinning: blend the vertex position and normal using up to 4 bone influences.
    float4 skinnedPos = float4(0, 0, 0, 0);
    float3 skinnedNormal = float3(0, 0, 0);
    
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        uint boneIndex = input.boneIndices[i];
        float weight = input.boneWeights[i];
        skinnedPos += mul(boneTransforms[boneIndex], float4(input.localPosition, 1.0)) * weight;
        skinnedNormal += mul((float3x3)boneTransforms[boneIndex], input.normal) * weight;
    }
    
    // Compute final position with world, view, projection
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, skinnedPos);
    output.uv = input.uv;
    output.normal = normalize(skinnedNormal);
    output.worldPos = mul(world, float4(skinnedPos.xyz, 1.0)).xyz;
    
    return output;
}