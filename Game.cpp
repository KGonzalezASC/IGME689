#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Camera.h"
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
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	LoadShaders();



	//TODO: REPLACE WITH PBR MATERIALS (i.e remove the tint and or make accomodations for both in PS)
	std::shared_ptr<Material> redMaterial = std::make_shared<Material>("Red Solid", pixelShader, vertexShader, XMFLOAT3(1.0f, 0.0f, 0.0f));
	materials.insert(materials.end(), { redMaterial});


	CreateGeometry(); //updating for A03
	// Set initial graphics API state pipeline settings
	{
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	//camera basic setup //TODO: CAMERA NEEDS IMPROVED CONTROLS and bug fix so we can set proper looking at position instead of directly at mouse pos.
	std::shared_ptr<Camera> camera = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, -15.0f), XM_PIDIV4, 0.01f, 1000.0f, 5.0f, 0.0055f);
	cameras.push_back(camera);
	//other camera at diff angle
	std::shared_ptr<Camera> camera2 = std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.5f, 0.0f, -15.0f), XM_PIDIV4, 0.01f, 1000.0f, 5.0f, 0.0055f);
	camera2.get()->getTransform().moveRelative(0.5f, 0.0f, 0.0f);
	cameras.push_back(camera2);

	JoltPhysicsTest();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	//cleanup imgui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}



void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device,
		Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"PixelShader.cso").c_str());
}


// --------------------------------------------------------
// Creates the geometry we're going to draw TEMPORARY
// --------------------------------------------------------
void Game::CreateGeometry()
{
	{
		std::shared_ptr<Mesh> cube = std::make_shared<Mesh>("cube", FixPath(L"../../Assets/Models/cube.obj").c_str());
		meshes.push_back(cube);
	}
	//TODO REORDER WHERE GEOMETRY COMES BEFORE MATERIALS AND ASSIGN SUCH IN INIT. THIS IS BECAUSE THE SKYBOX needs geometry for the cube map
	{
		for (auto& mesh : meshes)
		{
			entities.push_back(std::make_shared<GameObject>(mesh, materials[0]));
			entities.push_back(std::make_shared<GameObject>(mesh, materials[0]));
		}
	}
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
	Input::SetKeyboardCapture(io.WantCaptureKeyboard); // make sure no ! flag on this..
	Input::SetMouseCapture(io.WantCaptureMouse);

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

	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
	//updates should update worldmatrix of each entity which in turns means we have to 
	//reallocate the constant buffer for world matrix because it is per object and it is dirty
	//stoping rotation for now to test above claim
	//entities[0]->GetTransform()->Rotate(0, 0, deltaTime);
	cameras[activeCamera]->Update(deltaTime);
	updateUi(deltaTime);

	
	timeSincePhysicsStep += deltaTime;

	while (timeSincePhysicsStep >= cDeltaTime && runPhysics)
	{
		JoltPhysicsFrame();
		timeSincePhysicsStep -= cDeltaTime;
	}


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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	bgColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//post processing goes here

	//then drawing

	//draw each gameObject instead of mesh 
	for (auto& entity : entities)
	{
		//set color tint
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
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



// Program entry point
void Game::JoltPhysicsTest()
{
	// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
	// This needs to be done before any other Jolt function is called.
	RegisterDefaultAllocator();

	// Install trace and assert callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		Factory::sInstance = new Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	RegisterTypes();

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	temp_allocator = new TempAllocatorImpl(10 * 1024 * 104);

	// Now we can create the actual physics system.
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	//MyBodyActivationListener body_activation_listener;
	//physics_system.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	//MyContactListener contact_listener;
	//physics_system.SetContactListener(&contact_listener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	body_interface = &physics_system.GetBodyInterface();

	// Next we can create a rigid body to serve as the floor, we make a large box
	// Create the settings for the collision volume (the shape).
	// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
	floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

	// Create the shape
	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

	// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -5.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	// Create the actual rigid body
	floor = body_interface->CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

	// Add it to the world
	body_interface->AddBody(floor->GetID(), EActivation::DontActivate);

	// Now create a dynamic body to bounce on the floor
	// Note that this uses the shorthand version of creating and adding a body to the world
	BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	sphere_id = body_interface->CreateAndAddBody(sphere_settings, EActivation::Activate);

	BodyCreationSettings sphere_settings2(new SphereShape(0.5f), RVec3(0.1_r, 0.0_r, 0.1_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	sphere_id2 = body_interface->CreateAndAddBody(sphere_settings2, EActivation::Activate);

	// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
	// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
	body_interface->SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));

	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	physics_system.OptimizeBroadPhase();

	// Now we're ready to simulate the body, keep simulating until it goes to sleep
	//while (body_interface->IsActive(sphere_id))
	//{
	//	JoltPhysicsFrame();
	//}

	//DeInitPhysics();
}

void Game::DeInitPhysics()
{

	// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
	body_interface->RemoveBody(sphere_id);

	// Destroy the sphere. After this the sphere ID is no longer valid.
	body_interface->DestroyBody(sphere_id);

	// Remove and destroy the floor
	body_interface->RemoveBody(floor->GetID());
	body_interface->DestroyBody(floor->GetID());

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

void Game::JoltPhysicsFrame()
{
	// Next step
	++step;

	// Output current position and velocity of the sphere
	RVec3 position = body_interface->GetCenterOfMassPosition(sphere_id);
	Vec3 rotation = body_interface->GetRotation(sphere_id).GetEulerAngles();
	Vec3 velocity = body_interface->GetLinearVelocity(sphere_id);
	cout << "Step " << step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;
	
	entities[0]->GetTransform()->setPosition(position.GetX(), position.GetY(), position.GetZ());
	entities[0]->GetTransform()->setRotation(rotation.GetX(),rotation.GetY(), rotation.GetZ());

	//---------------------------
	position = body_interface->GetCenterOfMassPosition(sphere_id2);
	rotation = body_interface->GetRotation(sphere_id2).GetEulerAngles();

	entities[1]->GetTransform()->setPosition(position.GetX(), position.GetY(), position.GetZ());
	entities[1]->GetTransform()->setRotation(rotation.GetX(), rotation.GetY(), rotation.GetZ());

	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
	const int cCollisionSteps = 1;

	// Step the world
	physics_system.Update(cDeltaTime, cCollisionSteps, temp_allocator, job_system);
}