#pragma once
#include <DirectXMath.h>

struct MaterialBuffer
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};