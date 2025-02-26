#pragma once

#include "InputActionManager.h"
#include <Xinput.h>
#include <any>
#include <DirectXMath.h>
using namespace DirectX;

#define NOT_A_CONTROLLER -1
#define CONTROLLER_1 0
#define CONTROLLER_2 1
#define CONTROLLER_3 2
#define CONTROLLER_4 3

#define CONTROLLER_CONNECTED 100
#define CONTROLLER_DISCONNECTED 101

using InputActionManager::InputType;
using InputActionManager::InputBindings;

class XInputManager
{
// ===== | Variables | =====
public:
	static XInputManager* Instance;
	XINPUT_STATE* controllerStates = new XINPUT_STATE[4];
	WORD* prevConButtonStates = new WORD[4];
private:

// ===== | Methods | =====
public:
	XInputManager();
	~XInputManager();
	// Update all of the inputs that are being inputed 
	// during the frame on this method being called
	void UpdateControllerStates();
	InputType CheckButtonState(uint16_t button, int index);
	std::any GetValueFromController(InputBindings value, int index);
	static void Initialize();
private:
	InputType CheckButtonState(bool currentInput, bool prevInput);
};
