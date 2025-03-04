#include "PhysicsManager.h"

// Program entry point
PhysicsManager::PhysicsManager()
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

	// We need a temp allocator for temporary allocations during the physics update. We're pre-allocating 10 MB to avoid having to do allocations during the physics update.
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use. If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to malloc / free.
	temp_allocator = new TempAllocatorImpl(10 * 1024 * 104);

	// Now we can create the actual physics system.
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

	// A body activation listener gets notified when bodies activate and go to sleep. Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	//MyBodyActivationListener body_activation_listener;
	//physics_system.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again. Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	physics_system.SetContactListener(&contact_listener);

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

	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	physics_system.OptimizeBroadPhase();

	PhysicsSettings settings = physics_system.GetPhysicsSettings();
	settings.mTimeBeforeSleep = 2.f;

	physics_system.SetPhysicsSettings(settings);
}

PhysicsManager::~PhysicsManager()
{
	delete body_interface;
	delete temp_allocator;
	delete job_system;
}

void PhysicsManager::DeInitPhysics()
{
	for (auto& body : bodies)
	{
		// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
		body_interface->RemoveBody(body);
		// Destroy the sphere. After this the sphere ID is no longer valid.
		body_interface->DestroyBody(body);
	}

	bodies.clear();

	// Remove and destroy the floor
	body_interface->RemoveBody(floor->GetID());
	body_interface->DestroyBody(floor->GetID());

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

//Runs a step of the physics simulation
void PhysicsManager::JoltPhysicsFrame()
{
	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
	const int cCollisionSteps = 1;

	// Step the world
	physics_system.Update(cDeltaTime, cCollisionSteps, temp_allocator, job_system);
}

//creates a sphere body and adds it to the physics sim
BodyID PhysicsManager::CreatePhysicsSphereBody(RVec3 position, float size)
{
	// Now create a dynamic body to bounce on the floor
	// Note that this uses the shorthand version of creating and adding a body to the world
	BodyCreationSettings sphere_settings(new SphereShape(size), position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	BodyID newSphereID = body_interface->CreateAndAddBody(sphere_settings, EActivation::Activate);
	bodies.push_back(newSphereID);
	return newSphereID;
}

//creates a cube body and adds it to the physics sim
BodyID PhysicsManager::CreatePhysicsCubeBody(RVec3 position, Vec3 size)
{
	// Now create a dynamic body to bounce on the floor
	// Note that this uses the shorthand version of creating and adding a body to the world
	BodyCreationSettings cube_settings(new BoxShape(size), position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	BodyID newSphereID = body_interface->CreateAndAddBody(cube_settings, EActivation::Activate);
	bodies.push_back(newSphereID);

	return newSphereID;
}

//Applied velocity to a given body in the sim
void PhysicsManager::AddBodyVelocity(BodyID body, Vec3 velocity)
{
	// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
	// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
	body_interface->SetLinearVelocity(body, velocity);
}

//Raycasts from a given point in a given direction
//Returns a list of all physics objects hit by the ray
AllHitCollisionCollector<RayCastBodyCollector> PhysicsManager::JoltRayCast(Vec3::ArgType origin, Vec3Arg direction, float length)
{
	RayCast raycast{ origin, direction * length };
	AllHitCollisionCollector<RayCastBodyCollector> collector;
	
	physics_system.GetBroadPhaseQuery().CastRay(raycast, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());
	
	return collector;
}

AllHitCollisionCollector<CollideShapeBodyCollector> PhysicsManager::JoltShapeCast(Vec3Arg min, Vec3Arg max, Vec3Arg direction)
{
	AABox box = AABox(max, min);
	Mat44Arg matrix = Mat44Arg(Vec4Arg(1, 1, 1, 1));
	OrientedBox orientedBox{ matrix ,direction};

	AllHitCollisionCollector<CollideShapeBodyCollector> collector;

	physics_system.GetBroadPhaseQuery().CollideOrientedBox(orientedBox, collector, BroadPhaseLayerFilter(), ObjectLayerFilter());

	return collector;
}