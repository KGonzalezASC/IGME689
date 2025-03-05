#include "gameObject.h"
using namespace DirectX;



GameObject::GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material): mesh(target), material(material)
{
	transform = std::make_shared<Transform>();
}


GameObject::~GameObject(){}

std::shared_ptr<Mesh> GameObject::GetMesh(){return mesh;}
std::shared_ptr<Transform> GameObject::GetTransform(){return transform;}
std::shared_ptr<Material> GameObject::GetMaterial() { return material; }


void GameObject::SetMesh(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
}

void GameObject::SetMaterial(std::shared_ptr<Material> material)
{
	this->material = material;
}

void GameObject::Draw(std::shared_ptr<Camera> camera, UINT ObjectIndex)
{
	if (auto lessSimpleVertexShader = std::dynamic_pointer_cast<LessSimpleVertexShader>(material->GetVertexShader()))
	{
		material->PrepareLesserMaterial(transform, camera, ObjectIndex);
	}
	else
	{
		material->PrepareMaterial(transform, camera);
	}
	mesh->Draw();
}

// New method for instanced rendering
void GameObject::DrawInstanced(std::shared_ptr<Camera> camera, int instanceCount)
{
	material->PrepareMaterial(transform, camera);
	mesh->DrawInstanced(instanceCount);
}