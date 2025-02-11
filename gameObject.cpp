#include "gameObject.h"
using namespace DirectX;

GameObject::GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material) : mesh(target), material(material)
{
	transform = std::make_shared<Transform>();
	usingPhysicsBody = false;
}

GameObject::GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material, JPH::BodyID physBody): mesh(target), material(material), physicsBody(physBody)
{
	transform = std::make_shared<Transform>();
	usingPhysicsBody = true;
}

GameObject::~GameObject(){}

std::shared_ptr<Mesh> GameObject::GetMesh(){return mesh;}
std::shared_ptr<Transform> GameObject::GetTransform(){return transform;}
std::shared_ptr<Material> GameObject::GetMaterial() { return material; }
JPH::BodyID GameObject::GetPhysicsBody() { return physicsBody; }
bool GameObject::GetIsUsingPhysics() { return usingPhysicsBody; }

void GameObject::UpdateTransformFromPhysicsBody(PhysicsManager* physicsManager)
{
	RVec3 position = physicsManager->body_interface->GetCenterOfMassPosition(physicsBody);
	Vec3 rotation = physicsManager->body_interface->GetRotation(physicsBody).GetEulerAngles();

	GetTransform()->setPosition(position.GetX(), position.GetY(), position.GetZ());
	GetTransform()->setRotation(rotation.GetX(), rotation.GetY(), rotation.GetZ());
}

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