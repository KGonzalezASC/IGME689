#pragma once
#include <DirectXMath.h>


struct Vertex //ensure it is consistent with VertexToPixel
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT2 UV;			// The texture coordinates of the vertex
	DirectX::XMFLOAT3 Normal;		// The normal of the vertex
	DirectX::XMFLOAT4 Weights; // Weights of the bones affecting this vertex
	DirectX::XMUINT4 BoneIDs;  // Indices of the bones affecting this vertex
	//tangent here when needed
};