#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#define MAX_INSTANCES 1000

struct MaterialBuffer
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

struct InstanceData
{
	DirectX::XMFLOAT4X4 world;
};

namespace SharedBuffers
{
	extern Microsoft::WRL::ComPtr<ID3D11Buffer> InstanceBuffer; // Declare as extern
}
