#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include "SimpleShader/SimpleShader.h"
#include <d3d11_1.h>

class Material
{
public:
	Material(const char* name, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<SimpleVertexShader> vs, DirectX::XMFLOAT3 tint, float rough);
	~Material();

	//gets and setters
	//Although multiple materials may use the same vertex and pixel shader code(i.e., the same compiled shader files),
	//each material typically has its own instance of the vertex and pixel shaders.T
	//This allows the shaders to be applied with different parameter 
	//values for each material instance.
	//for any material using the same shader code, (which for now is all)
	//the shaders is just the same instance of the shader code being called with different params
	//on the gpu meaning that compiled instance is being reused in memory on the gpu
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	DirectX::XMFLOAT3 GetColorTint();
	const char* GetName();

	void SetPixelShader(std::shared_ptr<SimplePixelShader>);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader>);
	void SetColorTint(DirectX::XMFLOAT3 color);
	void SetRoughness(float rough);

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

	float roughness;
	const char* name;
};


