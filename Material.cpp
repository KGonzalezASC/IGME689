#include "Material.h"
//ctor
Material::Material(const char* name, std::shared_ptr<SimplePixelShader> ps, std::shared_ptr<SimpleVertexShader> vs, DirectX::XMFLOAT3 tint) :
	name(name),
	pixelShader(ps),
	vertexShader(vs),
	colorTint(tint)
{

}

//dtor
Material::~Material()
{
}

//getters and setters
const char* Material::GetName()
{
	return name;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return colorTint;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> ps)
{
	pixelShader = ps;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vs)
{
	vertexShader = vs;
}

void Material::SetColorTint(DirectX::XMFLOAT3 color)
{
	colorTint = color;
}


//LIKELY THIS AND SIMPLE SHADER NEEDS A REWRITE
void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	// 'Turn on' these shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	vertexShader->SetMatrix4x4("world", transform->getWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->getViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->getProjectionMatrix());
	vertexShader->CopyAllBufferData();

	pixelShader->SetFloat3("colorTint", colorTint);
	pixelShader->CopyAllBufferData();
}