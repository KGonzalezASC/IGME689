#include "Camera.h"
#include "Input.h"
#include <stdio.h>


using namespace Input;
using namespace DirectX;

Camera::Camera(float aspectRatio, float x, float y, float z, float fov, float nearP, float farP, float mSpeed, float lSpeed)
{
	transform.setPosition(x, y, z);
	this->fov = fov;
	this->aspectRatio = aspectRatio;
	this->nearP = nearP;
	this->farP = farP;
	this->movementSpeed = movementSpeed;
	mouseLookSpeed = mouseLookSpeed;
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
	viewType = CameraViewType::perspective;
}

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 pos, float fov, float nearP, float farP, float mSpeed, float lSpeed)
{
	transform.setPosition(pos);
	this->fov = fov;
	this->aspectRatio = aspectRatio;
	this->nearP = nearP;
	this->farP = farP;
	this->movementSpeed = mSpeed;
	this->mouseLookSpeed = lSpeed;
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
	viewType = CameraViewType::perspective;
}

Camera::~Camera()
{
}

//getters

DirectX::XMFLOAT4X4 Camera::getViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::getProjectionMatrix()
{
	return projectionMatrix;
}

Transform Camera::getTransform()
{
	return transform;
}

Transform Camera::getRelativeMotion()
{
	return relativeMotion;
}

//update

void Camera::Update(float dt) {
	//printf("View Matrix: %f, %f, %f, %f\n", viewMatrix._11, viewMatrix._12, viewMatrix._13, viewMatrix._14);
	//printf("Projection Matrix: %f, %f, %f, %f\n", projectionMatrix._11, projectionMatrix._12, projectionMatrix._13, projectionMatrix._14);
	relativeMotion.setPosition(0, 0, 0);
	float speed = dt * movementSpeed;
	if (Input::KeyDown('W') || Input::KeyDown(VK_UP)) {
		transform.moveRelative(0, 0, speed);
		relativeMotion.moveAbsolute(relativeMotion.getPosition().x, relativeMotion.getPosition().y, speed);
	}
	if (Input::KeyDown('S') || Input::KeyDown(VK_DOWN)) {
		transform.moveRelative(0, 0, -speed);
		relativeMotion.moveAbsolute(relativeMotion.getPosition().x, relativeMotion.getPosition().y, -speed);
	}
	if (Input::KeyDown('A') || Input::KeyDown(VK_LEFT)) {
		transform.moveRelative(-speed, 0, 0);
		relativeMotion.moveAbsolute(-speed, relativeMotion.getPosition().y, relativeMotion.getPosition().z);
	}
	if (Input::KeyDown('D') || Input::KeyDown(VK_RIGHT)) {
		transform.moveRelative(speed, 0, 0);
		relativeMotion.moveAbsolute(speed, relativeMotion.getPosition().y, relativeMotion.getPosition().z);
	}

	//use q and e to move up and down
	if (Input::KeyDown('X')) {
		transform.moveRelative(0, -speed, 0);
		relativeMotion.moveAbsolute(relativeMotion.getPosition().x, -speed, relativeMotion.getPosition().z);
	}
	if (Input::KeyDown(VK_SPACE)) {
		transform.moveRelative(0, speed, 0);
		relativeMotion.moveAbsolute(relativeMotion.getPosition().x, speed, relativeMotion.getPosition().z);
	}

	//use mouse to look around if mouse is down
	if (Input::MouseLeftDown()) {
		int cursorMovementX = Input::GetMouseXDelta();  //make float
		int cursorMovementY = Input::GetMouseYDelta();
		transform.Rotate(cursorMovementY * mouseLookSpeed, (cursorMovementX * mouseLookSpeed), 0);
		relativeMotion.moveAbsolute(cursorMovementX * mouseLookSpeed * 2, cursorMovementY * mouseLookSpeed * 2, 0);
	}

	// clamp pitch to prevent flipping over
	// also when camera moves it will rotate around the x axis to maintain what the user sees since the look at is indepdant from the position
	if (transform.getPitchYawRoll().x > (3.141592f / 2.0f)) {
		transform.setRotation((3.141592f / 2.0f), transform.getPitchYawRoll().y, transform.getPitchYawRoll().z);
	}
	if (transform.getPitchYawRoll().x < -(3.141592f / 2.0f)) {
		transform.setRotation(-(3.141592f / 2.0f), transform.getPitchYawRoll().y, transform.getPitchYawRoll().z);
	}


	UpdateViewMatrix();
}

//it needs to create a view matrix,
//most likely using XMMatrixLookToLH() – that’s Look To, not Look At.
void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform.getPosition();
	XMFLOAT3 forward = transform.getForward();

	XMStoreFloat4x4(
		&viewMatrix,
		XMMatrixLookToLH(
			XMVectorSet(pos.x, pos.y, pos.z, 1.0f),          // Position (W = 1.0f)
			XMVectorSet(forward.x, forward.y, forward.z, 0.0f), // Forward direction (W = 0.0f)
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)             // Up vector
		)
	);
}



void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearP, farP));
}