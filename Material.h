#pragma once
#include <DirectXMath.h>
#include "Transform.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include "SimpleShader/SimpleShader.h"
#include <d3d11_1.h>

//enabled_shared_from_this is a base class
//that enables objects of derived classes to be shared pointers
//and to be shared from a member function
//which allows for registering materials with shaders because
//the shared ptrs are not initialized in the ctor
//and we need to register the material with the shader
//to optimize per frame data
class Material : public std::enable_shared_from_this<Material>
{
public:
	Material(const char* name, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<ISimpleShader> vs, DirectX::XMFLOAT3 tint, float rough);
	~Material();
	void Initialize(); // to prevent weak ptrs as the shared ptrs are not initialized in the ctor
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<ISimpleShader> GetVertexShader();
	DirectX::XMFLOAT3 GetColorTint() const;
	const char* GetName();

	void SetPixelShader(std::shared_ptr<SimplePixelShader>);
	void SetVertexShader(std::shared_ptr<ISimpleShader>);
	void SetColorTint(DirectX::XMFLOAT3 color);
	void SetRoughness(float rough);

	//render loop related code:
	static std::unordered_map<std::shared_ptr<ISimpleShader>, std::vector<std::shared_ptr<Material>>> sharedVertexShaders; //collection of materials grouped by shader
	std::unordered_map<unsigned int, bool> perObjectDataProcessed; //i need another unordered map as a material can be shared by multiple objects i need a key and way to track if their "first pass" has been set
	//marks as dirty for drawFrame
	bool CheckAndSetPerObjectData(unsigned int objectIndex);
	//updates only perFrameData per shader group:
	static void UpdatePerFrameData(std::shared_ptr<Camera> camera);


	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<ISimpleShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	void RegisterMaterialWithShader(); //registers this material with the shader


	float roughness;
	const char* name;
	DirectX::XMFLOAT3 colorTint;
};


