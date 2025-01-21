#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include "SimpleShader/SimpleShader.h"
#include <d3d11.h>

class Material
{
public:
	Material(const char* name, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<SimpleVertexShader> vs, DirectX::XMFLOAT3 tint);
	~Material();


	//gets and setters
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	DirectX::XMFLOAT3 GetColorTint();
	const char* GetName();

	void SetPixelShader(std::shared_ptr<SimplePixelShader>);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader>);
	void SetColorTint(DirectX::XMFLOAT3 color);

	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);


	/*	
	• A 4 - component color tint
	• A SimpleVertexShader
	• A SimplePixelShader
	*/

private:
	//simple shader does not use comptr or com objects
	DirectX::XMFLOAT3 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	const char* name;
};


