#pragma once
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "Camera.h"

class GameObject
{
   public:
	 GameObject(std::shared_ptr<Mesh> target, std::shared_ptr<Material> material);
	 ~GameObject();
	 std::shared_ptr<Mesh> GetMesh();
	 std::shared_ptr<Transform> GetTransform();
	 std::shared_ptr<Material> GetMaterial();

	 void SetMaterial(std::shared_ptr<Material> material);
	 void SetMesh(std::shared_ptr<Mesh> mesh);
	 void Draw(std::shared_ptr<Camera> camera);
	 //void UpdateAnimation(float deltaTime);

   private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};