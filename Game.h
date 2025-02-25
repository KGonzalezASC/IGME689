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
#include "Lights.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"


#include "AudioManager.h"
#include <iostream>
#include "SharedBuffers.h"

#include "InputManager.h"

#define NUM_INSTANCES 10

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
	void PlayFunnySoundsOnPress();

	std::vector<std::shared_ptr<Camera>> cameras;
	int activeCamera = 0;



	//Meshes shared smart pointer
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameObject>> entities;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<Light> lights;

	//lighting
	DirectX::XMFLOAT3 ambientColor = { 0.1314f, 0.1977f, 0.2768f };


	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> instancedVertexShader;
	std::shared_ptr<SimplePixelShader> uvPixelShader;
	std::shared_ptr<SimplePixelShader> normalPixelShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;

	//ImGui
	bool showDemoWindow = false;
	float bgColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f }; // Background color
	float tintColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Tint color

	// Audio
	std::shared_ptr<AudioManager> audioManager;
};

