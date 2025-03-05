#include "Transform.h"

using namespace DirectX;

//using s r t to do transformations

Transform::Transform()
{
	XMStoreFloat3(&positionVector, { 0,0,0 });
	XMStoreFloat3(&pitchYawRoll, { 0,0,0 });
	XMStoreFloat3(&scaleVector, { 1,1,1 });
	XMStoreFloat3(&up, { 0,1,0 });
	XMStoreFloat3(&forward, { 0,0,1 });
	XMStoreFloat3(&right, { 1,0,0 });
	//set world matrix as identity matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}
Transform::~Transform(){}


//R

XMFLOAT3 Transform::getPitchYawRoll()
{
	return pitchYawRoll;
}

XMFLOAT3 Transform::getRotation()
{
	return pitchYawRoll;
}

void Transform::setRotation(float pitch, float yaw, float roll)
{
	shouldUpdateWorldMatrix = true;
	shouldUpdateCameraVectors = true;
	XMStoreFloat3(&pitchYawRoll, { pitch, yaw, roll });
}

void Transform::setRotation(XMFLOAT3 rotation)
{
	shouldUpdateWorldMatrix = true;
	shouldUpdateCameraVectors = true;
	pitchYawRoll = rotation;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	shouldUpdateCameraVectors = true;
	shouldUpdateWorldMatrix = true;
	//take objects current rotation
	XMVECTOR currentPitchYawRoll = XMLoadFloat3(&pitchYawRoll);
	//if we are doing float math we need to convert to xmvector
	XMVECTOR deltaPitchYawRoll = XMVectorSet(pitch, yaw, roll, 0);
	currentPitchYawRoll = XMVectorAdd(currentPitchYawRoll, deltaPitchYawRoll);
	// Store the result back into the pitchYawRoll member
	XMStoreFloat3(&pitchYawRoll, currentPitchYawRoll);
}

//both rotate and moveAbsolute should add according to notes
void Transform::Rotate(XMFLOAT3 rotation)
{
	shouldUpdateCameraVectors = true;
	shouldUpdateWorldMatrix = true;
	XMVECTOR currentPitchYawRoll = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR deltaPitchYawRoll = XMLoadFloat3(&rotation); 
	currentPitchYawRoll = XMVectorAdd(currentPitchYawRoll, deltaPitchYawRoll);
	XMStoreFloat3(&pitchYawRoll, currentPitchYawRoll);
}

//camera stuff related to R sorta..
XMFLOAT3 Transform::getForward()
{
	if (shouldUpdateCameraVectors)
	{
		updateRelativeVectors();
	}
	return forward;
}

XMFLOAT3 Transform::getRight()
{
	if (shouldUpdateCameraVectors)
	{
		updateRelativeVectors();
	}
	return right;
}

XMFLOAT3 Transform::getUp()
{
	if (shouldUpdateCameraVectors)
	{
		updateRelativeVectors();
	}
	return up;
}

bool Transform::isDirty()
{
	return shouldUpdateWorldMatrix; //indicates if world matrix needs to be updated
}

//P
XMFLOAT3 Transform::getPosition()
{
	return positionVector;
}

void Transform::setPosition(float x, float y, float z)
{
	//XMStoreFloat3 does not modify the original vector, it returns a new one in a thread safe manner
	shouldUpdateWorldMatrix = true;
	XMStoreFloat3(&positionVector, { x,y,z });
}

void Transform::setPosition(XMFLOAT3 position)
{
	shouldUpdateWorldMatrix = true;
	positionVector = position;
}

//moves only position  
void Transform::moveAbsolute(float x, float y, float z)
{
	/*Define member variables using XMFLOATs
		“Load” into XMVECTOR / XMMATRIX before a calculation
		Perform all necessary operations
		Do
		as much as you can at this step
		For this particular calculation
		“Store” result(s) back into XMFLOATs when done*/
	shouldUpdateWorldMatrix = true;


	XMVECTOR position = XMLoadFloat3(&positionVector);
	XMVECTOR translation = XMVectorSet(x, y, z, 0);
	position = XMVectorAdd(position, translation);
	XMStoreFloat3(&positionVector, position);
}

//means only transformation is done
void Transform::moveAbsolute(XMFLOAT3 translation)
{
	shouldUpdateWorldMatrix = true;
	XMVECTOR position = XMLoadFloat3(&positionVector);
	XMVECTOR translationVector = XMLoadFloat3(&translation);
	position = XMVectorAdd(position, translationVector);
	XMStoreFloat3(&positionVector, position);
}


//move relative
void Transform::moveRelative(float x, float y, float z)
{
	shouldUpdateWorldMatrix = true;
	XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	XMVECTOR relativeMovement = XMVectorSet(x, y, z, 0.0f);
	XMVECTOR existingPosition = XMLoadFloat3(&positionVector);

	XMVECTOR direction = XMVector3Rotate(relativeMovement, quat);
	direction = XMVectorAdd(direction, existingPosition);
	XMStoreFloat3(&positionVector, direction);
}

void Transform::moveRelative(XMFLOAT3 offset)
{
	shouldUpdateWorldMatrix = true; //just to be safe..
	moveRelative(offset.x, offset.y, offset.z);
}


//S
XMFLOAT3 Transform::getScale()
{
	return scaleVector;
}

void Transform::setScale(float x, float y, float z)
{
	shouldUpdateWorldMatrix = true;
	XMStoreFloat3(&scaleVector, { x,y,z });
}

void Transform::setScale(XMFLOAT3 scale)
{
	shouldUpdateWorldMatrix = true;
	XMStoreFloat3(&scaleVector, XMLoadFloat3(&scale));
}

//scale should multiply 
void Transform::Scale(float x, float y, float z)
{
	shouldUpdateWorldMatrix = true;
	XMVECTOR currentScale = XMLoadFloat3(&scaleVector);
	XMVECTOR scalingFactor = XMVectorSet(x, y, z, 0);
	currentScale = XMVectorMultiply(currentScale, scalingFactor);
	XMStoreFloat3(&scaleVector, currentScale);
}

void Transform::Scale(XMFLOAT3 scale)
{
	shouldUpdateWorldMatrix = true;
	XMVECTOR currentScale = XMLoadFloat3(&scaleVector);
	XMVECTOR scalingFactor = XMLoadFloat3(&scale);
	currentScale = XMVectorMultiply(currentScale, scalingFactor);
	XMStoreFloat3(&scaleVector, currentScale);
}



XMFLOAT4X4 Transform::getWorldMatrix()
{
	if (shouldUpdateWorldMatrix)
	{
		updateWorldMatrix();
	}
	return worldMatrix;
}

XMFLOAT4X4 Transform::getWorldInverseTransposeMatrix()
{
	return worldInverseTransposeMatrix;
}

void Transform::updateRelativeVectors() {

	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));
	XMStoreFloat3(&up, XMVector3Cross(XMLoadFloat3(&forward), XMLoadFloat3(&right)));
	shouldUpdateCameraVectors = false;
}


void Transform::updateWorldMatrix()
{
	//create translation matrix
	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&positionVector));
	//create rotation matrix
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	//create scale matrix
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&scaleVector));
	//create world matrix column major is gpu , row major is cpu friendly
	XMMATRIX world = scaleMatrix * rotationMatrix * translationMatrix;
	//store world matrix
	XMStoreFloat4x4(&worldMatrix, world);
	//store inverse transpose matrix
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));
	shouldUpdateWorldMatrix = false;
}