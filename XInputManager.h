#pragma once

#include "InputActionManager.h"
#include "Xinput.h"

class XInputManager
{
// ===== | Variables | =====
public:
	static enum Controller
	{
		Controller1,
		Controller2,
		Controller3,
		Controller4
	};

private:

// ===== | Methods | =====
public:
	XInputManager();
	~XInputManager();

private:
};
