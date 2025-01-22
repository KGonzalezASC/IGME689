#pragma once

#include "InputManager.h"
#include <hidusage.h>

void InputManager::Initialize(HWND windowHandle)
{
	// Init Keyboard
	kbState = new unsigned char[256];
	prevKbState = new unsigned char[256];

	memset(kbState, 0, sizeof(unsigned char) * 256);
	memset(prevKbState, 0, sizeof(unsigned char) * 256);
}

void InputManager::ProcessInputs()
{
	// Copy the old keys so we have last frame's data
	memcpy(prevKbState, kbState, sizeof(unsigned char) * 256);

	// Get the latest keys (from Windows)
	// Note the use of (void), which denotes to the compiler
	// that we're intentionally ignoring the return value
	(void)GetKeyboardState(kbState);
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
		kbState[key] & 0x80 &&
		!(prevKbState[key] & 0x80);
}
