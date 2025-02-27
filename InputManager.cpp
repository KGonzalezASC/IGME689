#pragma once

#include "InputManager.h"
#include <hidusage.h>

#include <iostream>

#include "XInputManager.h"

using InputActionManager::InputBindings;

// ---------------------------------------------------
//  Initializes the input variables and sets up the
//  initial arrays of key states
//
//  windowHandle - the handle (id) of the window,
//                 which is necessary for mouse input
// ---------------------------------------------------
void InputManager::Initialize(HWND windowHandle)
{
	kbState = new unsigned char[256];
	prevKbState = new unsigned char[256];
	
	memset(kbState, 0, sizeof(unsigned char) * 256);
	memset(prevKbState, 0, sizeof(unsigned char) * 256);
	
	wheelDelta = 0.0f;
	mouseX = 0; mouseY = 0;
	prevMouseX = 0; prevMouseY = 0;
	mouseXDelta = 0; mouseYDelta = 0;
	keyboardCaptured = false; mouseCaptured = false;

	hWnd = windowHandle;
	// Register for raw input from the mouse
	RAWINPUTDEVICE mouse = {};
	mouse.usUsagePage = HID_USAGE_PAGE_GENERIC;
	mouse.usUsage = HID_USAGE_GENERIC_MOUSE;
	mouse.dwFlags = RIDEV_INPUTSINK;
	mouse.hwndTarget = windowHandle;
	RegisterRawInputDevices(&mouse, 1, sizeof(mouse));

	// ND: Initialize the Action Manager
	InputActionManager::Initialize();
	XInputManager::Initialize();

	InputActionManager::CreateAction(L"Value");

	InputActionManager::AssignBindingToAction(L"Value", InputBindings::MouseDelta);

	InputActionManager::GetAction(L"Value").OnTrigger.push_back([](InputActionManager::InputData data) 
	{
		if (data.inputType == InputActionManager::InputType::Value)
		{
			std::optional<XMFLOAT2> vector = data.value.GetValue<XMFLOAT2>();
			if (vector.value().x != 0 || vector.value().y != 0)
				std::cout << vector.value().x << " " << vector.value().y << std::endl;
		}
	});

	InputActionManager::CreateAction(L"ButtonTest");

	InputActionManager::AssignBindingToAction(L"ButtonTest", InputBindings::MouseWheelDown);
	InputActionManager::AssignBindingToAction(L"ButtonTest", InputBindings::MouseWheelUp);

	InputActionManager::GetAction(L"ButtonTest").OnTrigger.push_back([](InputActionManager::InputData data)
	{
		if (data.inputType == InputActionManager::InputType::Pressed)
		{
			std::cout << "Key Pressed: " << data.key << std::endl;
		}

		if (data.inputType == InputActionManager::InputType::Released)
		{
			std::cout << "Key Released: " << data.key << std::endl;
		}

		if (data.inputType == InputActionManager::InputType::Down)
		{
			std::cout << "Key Down: " << data.key << std::endl;
		}
	});
}

// ---------------------------------------------------
//  Shuts down the input system, freeing any
//  allocated memory
// ---------------------------------------------------
void InputManager::ShutDown()
{
	delete[] kbState;
	delete[] prevKbState;
}

// ----------------------------------------------------------
//  Updates the input manager for this frame.  This should
//  be called at the beginning of every Game::Update(), 
//  before anything that might need input
// ----------------------------------------------------------
void InputManager::Update()
{
	// Copy the old keys so we have last frame's data
	memcpy(prevKbState, kbState, sizeof(unsigned char) * 256);

	// Get the latest keys (from Windows)
	// Note the use of (void), which denotes to the compiler
	// that we're intentionally ignoring the return value
	(void)GetKeyboardState(kbState);

	// Get the current mouse position then make it relative to the window
	POINT mousePos = {};
	GetCursorPos(&mousePos);
	ScreenToClient(hWnd, &mousePos);

	// Save the previous mouse position, then the current mouse 
	// position and finally calculate the change from the previous frame
	prevMouseX = mouseX;
	prevMouseY = mouseY;
	mouseX = mousePos.x;
	mouseY = mousePos.y;
	mouseXDelta = mouseX - prevMouseX;
	mouseYDelta = mouseY - prevMouseY;

	XInputManager::Instance->UpdateControllerStates();
	InputActionManager::CheckActionBindings();
}

// ----------------------------------------------------------
//  Resets the mouse wheel value and raw mouse delta at the 
//  end of the frame. This cannot occur earlier in the frame, 
//  since these come from Win32 windowing messages, which are
//  handled between frames.
// ----------------------------------------------------------
void InputManager::EndOfFrame()
{
	// Reset wheel value
	wheelDelta = 0;
	rawMouseXDelta = 0;
	rawMouseYDelta = 0;
}

// ----------------------------------------------------------
//  Get the mouse's current position in pixels relative
//  to the top left corner of the window.
// ----------------------------------------------------------
int InputManager::GetMouseX() { return mouseX; }
int InputManager::GetMouseY() { return mouseY; }


// ---------------------------------------------------------------
//  Get the mouse's change (delta) in position since last
//  frame in pixels relative to the top left corner of the window.
// ---------------------------------------------------------------
int InputManager::GetMouseXDelta() { return mouseXDelta; }
int InputManager::GetMouseYDelta() { return mouseYDelta; }


// ---------------------------------------------------------------
//  Passes raw mouse input data to the input manager to be
//  processed.  This input is the lParam of the WM_INPUT
//  windows message, captured from (presumably) DXCore.
// 
//  See the following article for a discussion on different
//  types of mouse input, not including GetCursorPos():
//  https://learn.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement
// ---------------------------------------------------------------
void InputManager::ProcessRawMouseInput(LPARAM lParam)
{
	// Variables for the raw data and its size
	unsigned char rawInputBytes[sizeof(RAWINPUT)] = {};
	unsigned int sizeOfData = sizeof(RAWINPUT);

	// Get raw input data from the lowest possible level and verify
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawInputBytes, &sizeOfData, sizeof(RAWINPUTHEADER)) == -1)
		return;

	// Got data, so cast to the proper type and check the results
	RAWINPUT* raw = (RAWINPUT*)rawInputBytes;
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		// This is mouse data, so grab the movement values
		rawMouseXDelta = raw->data.mouse.lLastX;
		rawMouseYDelta = raw->data.mouse.lLastY;
	}
}

// ---------------------------------------------------------------
//  Get the mouse's change (delta) in position since last
//  frame based on raw mouse data (no pointer acceleration)
// ---------------------------------------------------------------
int InputManager::GetRawMouseXDelta() { return rawMouseXDelta; }
int InputManager::GetRawMouseYDelta() { return rawMouseYDelta; }


// ---------------------------------------------------------------
//  Get the mouse wheel delta for this frame.  Note that there is 
//  no absolute position for the mouse wheel; this is either a
//  positive number, a negative number or zero.
// ---------------------------------------------------------------
float InputManager::GetMouseWheel() { return wheelDelta; }

float InputManager::GetPrevMouseWheel() { return prevWheelDelta; }

// ---------------------------------------------------------------
//  Sets the mouse wheel delta for this frame.  This is called
//  by DXCore whenever an OS-level mouse wheel message is sent
//  to the application.  You'll never need to call this yourself.
// ---------------------------------------------------------------
void InputManager::SetWheelDelta(float delta)
{
	prevWheelDelta = wheelDelta;
	wheelDelta = delta;
}

// ---------------------------------------------------------------
//  Sets whether or not keyboard input is "captured" elsewhere.
//  If the keyboard is "captured", the input manager will report 
//  false on all keyboard input.
// ---------------------------------------------------------------
void InputManager::SetKeyboardCapture(bool captured)
{
	keyboardCaptured = captured;
}


// ---------------------------------------------------------------
//  Sets whether or not mouse input is "captured" elsewhere.
//  If the mouse is "captured", the input manager will report 
//  false on all mouse input.
// ---------------------------------------------------------------
void InputManager::SetMouseCapture(bool captured)
{
	mouseCaptured = captured;
}


// ----------------------------------------------------------
//  Is the given key down this frame?
//  
//  key - The key to check, which could be a single character
//        like 'W' or '3', or a virtual key code like VK_TAB,
//        VK_ESCAPE or VK_SHIFT.
// ----------------------------------------------------------
bool InputManager::KeyDown(int key)
{
	if (key < 0 || key > 255) return false;

	return (kbState[key] & 0x80) != 0 && !keyboardCaptured;
}

// ----------------------------------------------------------
//  Is the given key up this frame?
//  
//  key - The key to check, which could be a single character
//        like 'W' or '3', or a virtual key code like VK_TAB,
//        VK_ESCAPE or VK_SHIFT.
// ----------------------------------------------------------
bool InputManager::KeyUp(int key)
{
	if (key < 0 || key > 255) return false;

	return !(kbState[key] & 0x80) && !keyboardCaptured;
}

// ----------------------------------------------------------
//  Was the given key initially pressed this frame?
//  
//  key - The key to check, which could be a single character
//        like 'W' or '3', or a virtual key code like VK_TAB,
//        VK_ESCAPE or VK_SHIFT.
// ----------------------------------------------------------
bool InputManager::KeyPress(int key)
{
	if (key < 0 || key > 255) return false;

	return
		kbState[key] & 0x80 &&			// Down now
		!(prevKbState[key] & 0x80) &&	// Up last frame
		!keyboardCaptured;
}

// ----------------------------------------------------------
//  Was the given key initially released this frame?
//  
//  key - The key to check, which could be a single character
//        like 'W' or '3', or a virtual key code like VK_TAB,
//        VK_ESCAPE or VK_SHIFT.
// ----------------------------------------------------------
bool InputManager::KeyRelease(int key)
{
	if (key < 0 || key > 255) return false;

	return
		!(kbState[key] & 0x80) &&	// Up now
		prevKbState[key] & 0x80 &&	// Down last frame
		!keyboardCaptured;
}


// ----------------------------------------------------------
//  A utility function to fill a given array of booleans 
//  with the current state of the keyboard.  This is most
//  useful when hooking the engine's input up to another
//  system, such as a user interface library.  (You probably 
//  won't use this very much, if at all!)
// 
//  keyArray - pointer to a boolean array which will be
//             filled with the current keyboard state
//  size - the size of the boolean array (up to 256)
// 
//  Returns true if the size parameter was valid and false
//  if it was <= 0 or > 256
// ----------------------------------------------------------
bool InputManager::GetKeyArray(bool* keyArray, int size)
{
	if (size <= 0 || size > 256) return false;

	// Loop through the given size and fill the
	// boolean array.  Note that the double exclamation
	// point is on purpose; it's a quick way to
	// convert any number to a boolean.
	for (int i = 0; i < size; i++)
		keyArray[i] = !!(kbState[i] & 0x80);

	return true;
}


// ----------------------------------------------------------
//  Is the specific mouse button down this frame?
// ----------------------------------------------------------
bool InputManager::MouseLeftDown() { return (kbState[VK_LBUTTON] & 0x80) != 0 && !mouseCaptured; }
bool InputManager::MouseRightDown() { return (kbState[VK_RBUTTON] & 0x80) != 0 && !mouseCaptured; }
bool InputManager::MouseMiddleDown() { return (kbState[VK_MBUTTON] & 0x80) != 0 && !mouseCaptured; }


// ----------------------------------------------------------
//  Is the specific mouse button up this frame?
// ----------------------------------------------------------
bool InputManager::MouseLeftUp() { return !(kbState[VK_LBUTTON] & 0x80) && !mouseCaptured; }
bool InputManager::MouseRightUp() { return !(kbState[VK_RBUTTON] & 0x80) && !mouseCaptured; }
bool InputManager::MouseMiddleUp() { return !(kbState[VK_MBUTTON] & 0x80) && !mouseCaptured; }


// ----------------------------------------------------------
//  Was the specific mouse button initially 
// pressed or released this frame?
// ----------------------------------------------------------
bool InputManager::MouseLeftPress() { return kbState[VK_LBUTTON] & 0x80 && !(prevKbState[VK_LBUTTON] & 0x80) && !mouseCaptured; }
bool InputManager::MouseLeftRelease() { return !(kbState[VK_LBUTTON] & 0x80) && prevKbState[VK_LBUTTON] & 0x80 && !mouseCaptured; }

bool InputManager::MouseRightPress() { return kbState[VK_RBUTTON] & 0x80 && !(prevKbState[VK_RBUTTON] & 0x80) && !mouseCaptured; }
bool InputManager::MouseRightRelease() { return !(kbState[VK_RBUTTON] & 0x80) && prevKbState[VK_RBUTTON] & 0x80 && !mouseCaptured; }

bool InputManager::MouseMiddlePress() { return kbState[VK_MBUTTON] & 0x80 && !(prevKbState[VK_MBUTTON] & 0x80) && !mouseCaptured; }
bool InputManager::MouseMiddleRelease() { return !(kbState[VK_MBUTTON] & 0x80) && prevKbState[VK_MBUTTON] & 0x80 && !mouseCaptured; }