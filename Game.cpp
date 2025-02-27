#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "InputManager.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Camera.h"
#include "AudioManager.h"
#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context11_1.Get());
	LoadShaders();

	//TODO: REPLACE WITH PBR MATERIALS (i.e remove the tint and or make accomodations for both in PS)
	redMaterial = std::make_shared<Material>("Red Solid", pixelShader, vertexShader, XMFLOAT3(1.0f, 0.0f, 0.0f), 0.5);

	std::shared_ptr<Material> orangeMaterial = std::make_shared<Material>("Orange Solid", pixelShader, vertexShader, XMFLOAT3(1.0f, 165.f/255.f, 0.0f), 0.5);
	materials.insert(materials.end(), { orangeMaterial });
	
	std::shared_ptr<Material> yellowMaterial = std::make_shared<Material>("Yellow Solid", pixelShader, vertexShader, XMFLOAT3(1.0f, 1.0f, 0.0f), 0.5);
	materials.insert(materials.end(), { yellowMaterial });
	
	std::shared_ptr<Material> greenMaterial = std::make_shared<Material>("Green Solid", pixelShader, vertexShader, XMFLOAT3(0.0f, 1.0f, 0.0f), 0.5);
	materials.insert(materials.end(), { greenMaterial });
	
	std::shared_ptr<Material> blueMaterial = std::make_shared<Material>("Blue Solid", pixelShader, vertexShader, XMFLOAT3(0.0f, 0.0f, 1.0f), 0.5);
	materials.insert(materials.end(), { blueMaterial });
	
	std::shared_ptr<Material> purpleMaterial = std::make_shared<Material>("Purple Solid", pixelShader, vertexShader, XMFLOAT3(0.5f, 0.0f, 0.5f), 0.5);
	materials.insert(materials.end(), { purpleMaterial });

	physicsManager = new PhysicsManager();
	sphere1 = physicsManager->CreatePhysicsSphereBody(RVec3(0.0_r, 20.0_r, 0.0_r),1);
	physicsManager->AddBodyVelocity(sphere1, Vec3(0.0f, -5.0f, 0.0f));
	sphere2 = physicsManager->CreatePhysicsSphereBody(RVec3(0.1_r, 0.0_r, 0.1_r),1);

	
	CreateGeometry(); //updating for A03
	// Set initial graphics API state pipeline settings
	{
		Graphics::Context11_1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	InputActionManager::GetAction(L"Value").OnTrigger.push_back([=](InputActionManager::InputData data)
	{
			if (data.inputType == InputActionManager::InputType::Value)
			{
				
			}
	});

	//camera basic setup //TODO: CAMERA NEEDS IMPROVED CONTROLS and bug fix so we can set proper looking at position instead of directly at mouse pos.
	std::shared_ptr<Camera> camera = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, -15.0f), XM_PIDIV4, 0.01f, 1000.0f, 5.0f, 0.0055f);
	cameras.push_back(camera);
	//other camera at diff angle
	std::shared_ptr<Camera> camera2 = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.5f, 0.0f, -15.0f), XM_PIDIV4, 0.01f, 1000.0f, 5.0f, 0.0055f);
	camera2.get()->getTransform().moveRelative(0.5f, 0.0f, 0.0f);
	cameras.push_back(camera2);

	audioManager = std::make_shared<AudioManager>();
}

// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	physicsManager->DeInitPhysics();

	//cleanup imgui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device,
		Graphics::Context11_1, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context11_1, FixPath(L"PixelShader.cso").c_str());
	instancedVertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context11_1, FixPath(L"InstancedVertexShader.cso").c_str());
}

// --------------------------------------------------------
// Creates the geometry we're going to draw TEMPORARY
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create the mesh
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>("cube", FixPath(L"../../Assets/Models/cube.obj").c_str());
	meshes.push_back(cube);

	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>("sphere", FixPath(L"../../Assets/Models/sphere.obj").c_str());
	meshes.push_back(sphere);

	// Create a single GameObject for the mesh
	std::shared_ptr<GameObject> cubeObject = std::make_shared<GameObject>(cube, redMaterial);
	entities.push_back(cubeObject);

	entities.push_back(std::make_shared<GameObject>(sphere, materials[0], sphere1));
	entities.push_back(std::make_shared<GameObject>(sphere, materials[0], sphere2));

	// Create instance data 
	std::vector<InstanceData> instanceData;
	for (int i = 0; i < NUM_INSTANCES; i++)
	{
		InstanceData data;
		// Set up the world matrix for each instance
		XMStoreFloat4x4(&data.world, XMMatrixTranslation(i * 4.0f, 0.0f, 0.0f));
		instanceData.push_back(data);
	}

	// Update the instance buffer with the instance data
	Graphics::UpdateInstanceBuffer(instanceData);

	//lighting
	Light pointLight1 = {};
	pointLight1.Color = XMFLOAT3(1, 1, 1);
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Intensity = 1.0f;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Range = 10.0f;

	Light pointLight2 = {};
	pointLight2.Color = XMFLOAT3(1, 1, 1);
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Intensity = 0.5f;
	pointLight2.Position = XMFLOAT3(1.5f, 0, 0);
	pointLight2.Range = 10.0f;

	Light spotLight1 = {};
	spotLight1.Color = XMFLOAT3(1, 1, 1);
	spotLight1.Type = LIGHT_TYPE_SPOT;
	spotLight1.Intensity = 2.0f;
	spotLight1.Position = XMFLOAT3(6.0f, 1.5f, 0);
	spotLight1.Direction = XMFLOAT3(0, -1, 0);
	spotLight1.Range = 10.0f;
	spotLight1.SpotOuterAngle = XMConvertToRadians(30.0f);
	spotLight1.SpotInnerAngle = XMConvertToRadians(20.0f);

	lights.push_back(pointLight1);
	lights.push_back(pointLight2);
	lights.push_back(spotLight1);

	//normalize directions of all non-point lights
	for (int i = 0; i < lights.size(); i++)
		if (lights[i].Type != LIGHT_TYPE_POINT)
			XMStoreFloat3(
				&lights[i].Direction,
				XMVector3Normalize(XMLoadFloat3(&lights[i].Direction))
			);
}


void Game::OnResize()
{
	for (auto& camera : cameras)
	{
		camera->UpdateProjectionMatrix(Window::AspectRatio());
	}
}

//TODO :move UI frame creation into seperate method to reduce bloat in game.cpp
void Game:: updateUi(float deltaTime) {
	// Start a new ImGui frame that is made for each frame
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize = ImVec2((float)Window::Width(), (float)Window::Height());
	//reset frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Determine the new input capture state which is what the user is currently doing
	InputManager::SetKeyboardCapture(io.WantCaptureKeyboard); // make sure no ! flag on this..
	InputManager::SetMouseCapture(io.WantCaptureMouse);

	//create window, second param appears to save last window size?
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);

	ImGui::Begin("kg4668 custom ui"); 
	//set imgui size
	if (ImGui::TreeNode("App Details:")) {

		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		//%dx%d is a format string that will be replaced with the width and height of the window
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

		//color picker
		ImGui::ColorEdit4("RGBA color editor", bgColor);
		//info bout each mesh
		if (ImGui::TreeNode("Meshes:")) {
			for (auto& mesh : meshes) {
				ImGui::Text("Mesh: %s", mesh->GetName());
				//vertex count
				ImGui::Text("Vertex Count: %d", mesh->GetVertexCount());
				//index count
				ImGui::Text("Index Count: %d", mesh->GetIndexCount());
			}
			ImGui::TreePop(); //close tree node
		}
		//info bout each entity
		if (ImGui::TreeNode("Entities:")) {
			for (size_t i = 0; i < entities.size(); i++) {
				auto& entity = entities[i];
				ImGui::Text("Entity %d: %s", i, entity->GetMesh()->GetName());

				// Transform Component
				std::string transformNodeLabel = "Transform Component##" + std::to_string(i); // Unique label
				if (ImGui::TreeNode(transformNodeLabel.c_str())) {
					// Position
					XMFLOAT3 position = entity->GetTransform()->getPosition();
					std::string sliderLabel = "Position##" + std::to_string(i);
					if (ImGui::SliderFloat3(sliderLabel.c_str(), &position.x, -1.0f, 1.0f)) {
						entity->GetTransform()->setPosition(position); // Update position
					}

					// Rotation
					XMFLOAT3 rotation = entity->GetTransform()->getRotation();
					sliderLabel = "Rotation##" + std::to_string(i);
					if (ImGui::SliderFloat3(sliderLabel.c_str(), &rotation.x, -XM_2PI, XM_2PI)) {
						entity->GetTransform()->setRotation(rotation); // Update rotation
					}

					// Scale
					XMFLOAT3 scale = entity->GetTransform()->getScale();
					sliderLabel = "Scale##" + std::to_string(i);
					if (ImGui::SliderFloat3(sliderLabel.c_str(), &scale.x, 0.1f, 2.0f)) {
						entity->GetTransform()->setScale(scale); // Update scale
					}

					ImGui::TreePop(); // Close transform node
				}
			}
			ImGui::TreePop(); // Close entities node
		}


		ImGui::TreePop(); //close tree node
	}
	//camera manager
	XMFLOAT3 camPos = cameras[activeCamera]->getRelativeMotion().getPosition();
	ImGui::InputInt("Camera switch", &activeCamera);
	if (activeCamera > cameras.size() - 1) {
		activeCamera = 0;
	}
	if (activeCamera < 0) {
		activeCamera = (int)cameras.size() - 1;
	}

	if (ImGui::Button("ImGui demo window"))
	{
		showDemoWindow = !showDemoWindow; 
	}

	if (ImGui::Button("Run Physics"))
	{
		runPhysics = true;
		timeSincePhysicsStep = 0.f;
	}

	//close window
	ImGui::End();
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{

	if (InputManager::KeyDown(VK_ESCAPE))
		Window::Quit();
	//updates should update worldmatrix of each entity which in turns means we have to 
	//reallocate the constant buffer for world matrix because it is per object and it is dirty
	//stoping rotation for now to test above claim
	//entities[0]->GetTransform()->Rotate(0, 0, deltaTime);
	cameras[activeCamera]->Update(deltaTime);
	updateUi(deltaTime);

	if (InputManager::KeyPress(VK_DELETE))
	{
		int matLocation = rand() % materials.size();
		
		BodyID id = physicsManager->CreatePhysicsCubeBody(Vec3(0.0f, 10.0f, 0.0f), Vec3(1, 1, 1));
		std::shared_ptr<GameObject> entity = std::make_shared<GameObject>(meshes[0], materials[matLocation], id);
		entities.push_back(entity);
		bodyObjects[id] = entity.get();
	}

	if (InputManager::KeyPress(VK_INSERT))
	{
		XMFLOAT3 pos = cameras[0]->getTransform().getPosition();
		XMFLOAT3 forward = cameras[0]->getTransform().getForward();

		AllHitCollisionCollector<RayCastBodyCollector> collector = physicsManager->JoltRayCast(Vec3(pos.x, pos.y, pos.z), Vec3Arg(forward.x, forward.y, forward.z), 100);

		bool hasHit = collector.HadHit();

		if (hasHit)
		{
			for (auto& hitBody : collector.mHits)
			{
				if (bodyObjects.contains(hitBody.mBodyID))
				{
					bodyObjects[hitBody.mBodyID]->SetMaterial(redMaterial);
				}				
			}
		}
	}
	
	timeSincePhysicsStep += deltaTime;

	while (timeSincePhysicsStep >= cDeltaTime && runPhysics)
	{
		physicsManager->JoltPhysicsFrame();
		timeSincePhysicsStep -= cDeltaTime;

		// Next step
		++step;

		for (auto& entity : entities)
		{
			if (entity->GetIsUsingPhysics())
			{
				entity->UpdateTransformFromPhysicsBody(physicsManager);
			}
		}
	}
	audioManager->update_audio(deltaTime);

	// To play a sound, call audioManager->playSound("filepath"). For example:
	//audioManager->playSound("Sounds/vine-thud.wav");
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context11_1->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	bgColor);
		Graphics::Context11_1->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//post processing goes here

	//then drawing

	// Draw each GameObject with instancing
	for (auto& entity : entities)
	{
		std::shared_ptr<SimplePixelShader> pixelShader = entity->GetMaterial()->GetPixelShader();
		pixelShader->SetFloat3("ambientColor", ambientColor);
		pixelShader->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		//entity->DrawInstanced(cameras[activeCamera], NUM_INSTANCES);
		entity->Draw(cameras[activeCamera]);
	}

	//draw ui we have to do this after drawing everything else to ensure sorting
	{
		// Render the UI
		ImGui::Render(); //render as triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); //draws to screen
	}

	//clear any allocated buffered used in post processing around the frame end

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context11_1->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}