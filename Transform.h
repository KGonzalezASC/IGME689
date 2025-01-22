#pragma once
#include <DirectXMath.h>

class Transform
{
public:
	Transform();
	~Transform();

	//position
	DirectX::XMFLOAT3 getPosition();
	void setPosition(float x, float y, float z);
	void setPosition(DirectX::XMFLOAT3 position);
	void moveAbsolute(float x, float y, float z);
	void moveAbsolute(DirectX::XMFLOAT3 translation);
	void moveRelative(float x, float y, float z);
	void moveRelative(DirectX::XMFLOAT3 offset);

	//rotation
	DirectX::XMFLOAT3 getPitchYawRoll();
	DirectX::XMFLOAT3 getRotation();
	void setRotation(float pitch, float yaw, float roll);
	void setRotation(DirectX::XMFLOAT3 rotation);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation); //not doing the xmfloat4 version but i have a rough idea as to how

	//scale 
	DirectX::XMFLOAT3 getScale();
	void setScale(float x, float y, float z);
	void setScale(DirectX::XMFLOAT3 scale);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);


	//world matrix 
	DirectX::XMFLOAT4X4 getWorldMatrix();
	DirectX::XMFLOAT4X4 getWorldInverseTransposeMatrix();
	//check dirty on world
	bool isDirtyWorld();

	//camera stuff
	DirectX::XMFLOAT3 getForward();
	DirectX::XMFLOAT3 getRight();
	DirectX::XMFLOAT3 getUp();

private:
	DirectX::XMFLOAT3 positionVector;
	DirectX::XMFLOAT3 scaleVector;
	DirectX::XMFLOAT3 pitchYawRoll;
	DirectX::XMFLOAT4X4 worldMatrix;

    DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	//camera stuff
	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 forward;
	DirectX::XMFLOAT3 right;

	void updateRelativeVectors(); //updates rotation vectors only when needed
	void updateWorldMatrix();

	bool shouldUpdateWorldMatrix = false;
	bool shouldUpdateCameraVectors = false;
};

