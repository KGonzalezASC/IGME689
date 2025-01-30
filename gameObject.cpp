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

void GameObject::Draw(std::shared_ptr<Camera> camera)
{
	material->PrepareMaterial(transform, camera);
	mesh->Draw();
}

void GameObject::UpdateAnimation(float deltaTime) {
	if (mesh) {
		mesh->UpdateAnimation(deltaTime);
	}
}