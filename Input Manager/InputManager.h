#pragma once

#include <Windows.h>

// Manages inputs from connected devices
namespace InputManager
{
	namespace
	{
		// ===== | Variables | =====
		unsigned char* kbState = 0;
		unsigned char* prevKbState = 0;

		HWND hWnd = 0;
	}
	
	// ===== | Methods | =====
	void Initialize(HWND windowHandle);
	// Process and the inputs. Should be called every frame
	void ProcessInputs();
	bool KeyPress(int key)
};