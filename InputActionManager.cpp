#include "InputActionManager.h"
#include <iostream>

namespace InputActionManager
{
	std::unordered_map<std::wstring, InputAction> actions;
	std::unordered_map<InputBindings, std::pair<InputBindingType, uint16_t>> bindings;
	std::unordered_map<InputBindings, std::unordered_set<std::wstring>> actionBindings;

	// ===== | Structs | =====
	

	void Initialize()
	{
		using BindingPair = std::pair<InputBindingType, uint16_t>;

		bindings.clear();

		// Helper lambdas to add keyboard/mouse entries more succinctly
		auto addKey = [&](InputBindings binding, uint16_t vkCode) {
			bindings.emplace(binding, BindingPair(Keyboard, vkCode));
			};
		auto addMouse = [&](InputBindings binding, uint16_t vkCode) {
			bindings.emplace(binding, BindingPair(Mouse, vkCode));
			};

		// Add keyboard keys
		addKey(KeyA, 0x41); // 'A' key
		addKey(KeyB, 0x42); // 'B' key
		addKey(KeyC, 0x43); // 'C' key
		addKey(KeyD, 0x44); // 'D' key
		addKey(KeyE, 0x45); // 'E' key
		addKey(KeyF, 0x46); // 'F' key
		addKey(KeyG, 0x47); // 'G' key
		addKey(KeyH, 0x48); // 'H' key
		addKey(KeyI, 0x49); // 'I' key
		addKey(KeyJ, 0x4A); // 'J' key
		addKey(KeyK, 0x4B); // 'K' key
		addKey(KeyL, 0x4C); // 'L' key
		addKey(KeyM, 0x4D); // 'M' key
		addKey(KeyN, 0x4E); // 'N' key
		addKey(KeyO, 0x4F); // 'O' key
		addKey(KeyP, 0x50); // 'P' key
		addKey(KeyQ, 0x51); // 'Q' key
		addKey(KeyR, 0x52); // 'R' key
		addKey(KeyS, 0x53); // 'S' key
		addKey(KeyT, 0x54); // 'T' key
		addKey(KeyU, 0x55); // 'U' key
		addKey(KeyV, 0x56); // 'V' key
		addKey(KeyW, 0x57); // 'W' key
		addKey(KeyX, 0x58); // 'X' key
		addKey(KeyY, 0x59); // 'Y' key
		addKey(KeyZ, 0x5A); // 'Z' key
		addKey(Key0, 0x30); // '0' key
		addKey(Key1, 0x31); // '1' key
		addKey(Key2, 0x32); // '2' key
		addKey(Key3, 0x33); // '3' key
		addKey(Key4, 0x34); // '4' key
		addKey(Key5, 0x35); // '5' key
		addKey(Key6, 0x36); // '6' key
		addKey(Key7, 0x37); // '7' key
		addKey(Key8, 0x38); // '8' key
		addKey(Key9, 0x39); // '9' key
		addKey(KeyEscape, VK_ESCAPE);
		addKey(KeySpace, VK_SPACE);
		addKey(KeyEnter, VK_RETURN);
		addKey(KeyTab, VK_TAB);
		addKey(KeyBackspace, VK_BACK);
		addKey(KeyInsert, VK_INSERT);
		addKey(KeyDelete, VK_DELETE);
		addKey(KeyRight, VK_RIGHT);
		addKey(KeyLeft, VK_LEFT);
		addKey(KeyDown, VK_DOWN);
		addKey(KeyUp, VK_UP);
		addKey(KeyPageUp, VK_PRIOR);
		addKey(KeyPageDown, VK_NEXT);
		addKey(KeyHome, VK_HOME);
		addKey(KeyEnd, VK_END);
		addKey(KeyCapsLock, VK_CAPITAL);
		addKey(KeyScrollLock, VK_SCROLL);
		addKey(KeyNumLock, VK_NUMLOCK);
		addKey(KeyPrintScreen, VK_SNAPSHOT);
		addKey(KeyPause, VK_PAUSE);
		addKey(KeyF1, VK_F1);
		addKey(KeyF2, VK_F2);
		addKey(KeyF3, VK_F3);
		addKey(KeyF4, VK_F4);
		addKey(KeyF5, VK_F5);
		addKey(KeyF6, VK_F6);
		addKey(KeyF7, VK_F7);
		addKey(KeyF8, VK_F8);
		addKey(KeyF9, VK_F9);
		addKey(KeyF10, VK_F10);
		addKey(KeyF11, VK_F11);
		addKey(KeyF12, VK_F12);
		addKey(KeyLeftShift, VK_LSHIFT);
		addKey(KeyRightShift, VK_RSHIFT);
		addKey(KeyLeftControl, VK_LCONTROL);
		addKey(KeyRightControl, VK_RCONTROL);
		addKey(KeyLeftAlt, VK_LMENU);
		addKey(KeyRightAlt, VK_RMENU);

		// Add mouse buttons
		addMouse(MouseLeftButton, VK_LBUTTON);
		addMouse(MouseRightButton, VK_RBUTTON);
		addMouse(MouseMiddleButton, VK_MBUTTON);
		addMouse(MouseWheelUp, 0); // Custom code for mouse wheel up
		addMouse(MouseWheelDown, 0); // Custom code for mouse wheel down
	}


	void InputActionManager::CreateAction(const wchar_t* name)
	{
		actions.insert(std::make_pair(name, std::move(InputAction(name))));
	}

	InputAction& InputActionManager::GetAction(std::wstring name)
	{
		return actions.at(name);
	}

	void InputActionManager::AssignBindingToAction(std::wstring actionName, InputBindings key)
	{
		InputActionManager::actionBindings[key].insert(actionName);
	}

	void InputActionManager::RemoveBindingFromAction(std::wstring actionName, InputBindings key)
	{
		actionBindings[key].erase(actionName);
	}

	void InputActionManager::CheckActionBindings()
	{
		// Loop through all the bindings in actionBindings
		for (auto& binding : actionBindings)
		{
			InputBindings input = binding.first;
			InputBindingType type = bindings[input].first;
			InputType inputType = InputType::Up;

			if (type == InputBindingType::Keyboard)
			{
				inputType = InputActionManager::ProcessKey(bindings[input].second);
			}
			else if (type == InputBindingType::Mouse)
			{

			}

			InputData data = {};
			data.inputType = inputType;
			data.key = input;

			// Loop through all the actions associated with this binding
			for (auto& actionName : binding.second)
			{
				InputAction& action = actions.at(actionName);
				for (auto& event : action.OnTrigger)
				{
					event(data);			
				}
			}
		}
	}

	InputType InputActionManager::ProcessKey(uint16_t key)
	{
		if (InputManager::KeyPress(key))
			return InputType::Pressed;
		if (InputManager::KeyRelease(key))
			return InputType::Released;
		if (InputManager::KeyDown(key))
			return InputType::Down;
		if (InputManager::KeyUp(key))
			return InputType::Up;

		return InputType::Up;
	}

	InputType InputActionManager::ProcessMouse(InputBindings mouseInput)
	{
		return InputType::Up;
	}
}