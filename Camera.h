#pragma once
#include "DirectXMath.h"
#include "Transform.h"
#include "Input.h"

enum CameraViewType
{
	perspective,
	orthographic
};


class Camera
{
	//needs Transform field, xmfloat4x4 view matrix, xmfloat4x4 proj matrix
	//float for fov, aspect ratio, near plane, far plane
	//movementspeed, mouse look speed
public:
	Camera(float aspectRatio, float x, float y, float z, float fov = (3.141592f / 4.0f), float nearP = 0.01f, float farP = 100.0f, float mSpeed = 1.0f, float lSpeed = 0.01f);
	Camera(float aspectRatio, DirectX::XMFLOAT3 pos, float fov = (3.141592f / 4.0f), float nearP = 0.01f, float farP = 100.0f, float mSpeed = 1.0f, float lSpeed = 0.01f);

	~Camera();

	DirectX::XMFLOAT4X4 getViewMatrix();
	DirectX::XMFLOAT4X4 getProjectionMatrix();
	Transform getTransform();
	Transform getRelativeMotion();

	void Update(float deltaTime);
	void UpdateProjectionMatrix(float aspectRatio);	

private:

	Transform transform;
	Transform relativeMotion;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float fov;
	float aspectRatio;
	float nearP;
	float farP;

	float movementSpeed;
	float mouseLookSpeed;

	CameraViewType viewType;

	void UpdateViewMatrix();
};

