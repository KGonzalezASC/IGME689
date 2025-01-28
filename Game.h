#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "Mesh.h"
#include "GameObject.h"
#include "Camera.h"
#include "Material.h"
#include "SimpleShader/SimpleShader.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "PhysicsManager.h"

class Game
{
public:
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void updateUi(float deltaTime);

	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCamera = 0;



	//Meshes shared smart pointer
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameObject>> entities;
	std::vector<std::shared_ptr<Material>> materials;


	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> uvPixelShader;
	std::shared_ptr<SimplePixelShader> normalPixelShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;

	


	//ImGui
	bool showDemoWindow = false;
	float bgColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f }; // Background color
	float tintColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Tint color

	PhysicsManager* physicsManager;

	// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
	const float cDeltaTime = 1.0f / 60.0f;

	float timeSincePhysicsStep = 0.f;

	bool runPhysics = false;
};


