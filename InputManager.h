#pragma once 

#include <Windows.h>
#include "InputActionManager.h"

// Manages inputs from connected devices
namespace InputManager
{
	namespace
	{
		// ===== | Variables | =====
		unsigned char* kbState = 0;
		unsigned char* prevKbState = 0;

		// Mouse position and wheel data
		int mouseX = 0;
		int mouseY = 0;
		int prevMouseX = 0;
		int prevMouseY = 0;
		int mouseXDelta = 0;
		int mouseYDelta = 0;
		int rawMouseXDelta = 0;
		int rawMouseYDelta = 0;
		float wheelDelta = 0;
		float prevWheelDelta = 0;

		// Support for capturing input outside the input manager
		bool keyboardCaptured = false;
		bool mouseCaptured = false;

		HWND hWnd = 0;
	}

	// ===== | Methods | =====
	void Initialize(HWND windowHandle);
	void ShutDown();
	// Process and the inputs. Should be called every frame
	void Update();
	void EndOfFrame();
	int GetMouseX();
	int GetMouseY();
	int GetMouseXDelta();
	int GetMouseYDelta();
	void ProcessRawMouseInput(LPARAM lParam);
	int GetRawMouseXDelta();
	int GetRawMouseYDelta();
	float GetMouseWheel();
	float GetPrevMouseWheel();
	void SetWheelDelta(float delta);
	void SetKeyboardCapture(bool captured);
	void SetMouseCapture(bool captured);
	bool KeyDown(int key);
	bool KeyUp(int key);
	bool KeyPress(int key);
	bool KeyRelease(int key);
	bool GetKeyArray(bool* keyArray, int size);
	bool MouseLeftDown();
	bool MouseMiddleDown();
	bool MouseRightDown();
	bool MouseLeftUp();
	bool MouseMiddleUp();
	bool MouseRightUp();
	bool MouseLeftPress();
	bool MouseMiddlePress();
	bool MouseRightPress();
	bool MouseLeftRelease();
	bool MouseMiddleRelease();
	bool MouseRightRelease();
}