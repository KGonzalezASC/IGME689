#include "gameObject.h"
using namespace DirectX;
#include <iostream>


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

void GameObject::Draw(std::shared_ptr<Camera> camera, float deltaTime)
{
	material->PrepareMaterial(transform, camera);
	UpdateAnimation(deltaTime);
	material->GetVertexShader()->SetData("BoneData", &boneTransforms[0], sizeof(float) * 832);
	material->GetVertexShader()->CopyAllBufferData();

	mesh->Draw(deltaTime);
}

void GameObject::UpdateAnimation(float deltaTime)
{
	static float animationTime = 0.0f;
	animationTime += deltaTime;

	
	mesh->BoneTransform(deltaTime, boneTransforms);
}