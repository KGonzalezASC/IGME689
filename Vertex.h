#pragma once
#include <DirectXMath.h>


struct Vertex //ensure it is consistent with VertexToPixel
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT2 UV;			// The texture coordinates of the vertex
	DirectX::XMFLOAT3 Normal;		// The normal of the vertex
	//tangent here when needed
	UINT InstanceID;
};