#pragma once

#include "InputActionManager.h"
#include <Xinput.h>

#define NOT_A_CONTROLLER -1
#define CONTROLLER_1 0
#define CONTROLLER_2 1
#define CONTROLLER_3 2
#define CONTROLLER_4 3

class XInputManager
{
// ===== | Variables | =====
public:
	static XInputManager* Instance;
	XINPUT_STATE* controllerStates = new XINPUT_STATE[4];
private:

// ===== | Methods | =====
public:
	XInputManager();
	~XInputManager();
	void UpdateControllerStates();
	// This is debug and can eventually be removed
	void CheckControllerState(DWORD dwUserIndex);
	static void Initialize();
private:
	
};
