#pragma once
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "Camera.h"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

class GameObject
{
   public:
	 GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material);
	 GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material, JPH::BodyID physBody);
	 ~GameObject();
	 std::shared_ptr<Mesh> GetMesh();
	 std::shared_ptr<Transform> GetTransform();
	 std::shared_ptr<Material> GetMaterial();
	 JPH::BodyID GetPhysicsBody();

	 void SetMaterial(std::shared_ptr<Material> material);
	 void SetMesh(std::shared_ptr<Mesh> mesh);
	 void Draw(std::shared_ptr<Camera> camera);

	 bool usingPhysicsBody;

   private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;

	JPH::BodyID physicsBody;
};