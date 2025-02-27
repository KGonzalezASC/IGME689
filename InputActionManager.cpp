#include "InputActionManager.h"
#include "XInputManager.h"
#include <iostream>
#include <vector>

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

		auto addController = [&](InputBindings binding, uint16_t vkCode) {
			bindings.emplace(binding, BindingPair(XController, vkCode));
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

		addMouse(MouseDelta, 0);
		addMouse(MousePosition, 0);
		addMouse(MouseWheelDelta, 0);

		// Add controller buttons
		addController(XControllerA, 0x1000);
		addController(XControllerB, 0x2000);
		addController(XControllerX, 0x4000);
		addController(XControllerY, 0x8000);
		addController(XControllerDPadUp, 0x0001);
		addController(XControllerDPadDown, 0x0002);
		addController(XControllerDPadLeft, 0x0004);
		addController(XControllerDPadRight, 0x0008);
		addController(XControllerStart, 0x0010);
		addController(XControllerBack, 0x0020);
		addController(XControllerLeftThumb, 0x0040);
		addController(XControllerRightThumb, 0x0080);
		addController(XControllerLeftShoulder, 0x0100);
		addController(XControllerRightShoulder, 0x0200);

		addController(XControllerLeftStick, 0);
		addController(XControllerRightStick, 0);
		addController(XControllerLeftTrigger, 0);
		addController(XControllerRightTrigger, 0);
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
		std::vector<InputBindings> conBinds;

		// Loop through all the bindings in actionBindings
		for (auto& binding : actionBindings)
		{
			InputBindings input = binding.first;
			InputBindingType type = bindings[input].first;
			InputType inputType = InputType::Up;
			std::any inputValue = std::any();

			if (type == InputBindingType::Keyboard)
			{
				inputType = InputActionManager::ProcessKey(bindings[input].second);
			}
			else if (type == InputBindingType::Mouse)
			{
				if (input >= 74 && input <= 78)
				{
					inputType = ProcessMouse(input);
				}
				else if (input >= 97 && input <= 99)
				{
					inputType = InputType::Value;
					inputValue = GetMouseValue(input);
				}
			}
			else if (type == InputBindingType::XController)
			{
				conBinds.push_back(input);
			}

			InputData data = {};
			data.inputType = inputType;
			data.key = input;
			data.controllerIndex = -1;
			data.value = InputValue(inputValue);

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

		for (InputBindings conbind : conBinds)
		{
			for (int i = 0; i < 4; i++)
			{
				InputBindingType type = bindings[conbind].first;
				InputType inputType = InputType::Up;
				std::any inputValue = std::any();
				

				if (conbind >= 79 && conbind <= 92)
				{
					inputType = XInputManager::Instance->CheckButtonState(
						bindings[conbind].second, i);
				}
				else if (conbind >= 93 && conbind <= 96)
				{
					inputValue = XInputManager::Instance->GetValueFromController(
						conbind, i);

					inputType = InputType::Value;
				}

				InputData data = {};
				data.inputType = inputType;
				data.key = conbind;
				data.controllerIndex = i;
				data.value = InputValue(inputValue);

				// Loop through all the actions associated with this binding
				for (auto& actionName : actionBindings[conbind])
				{
					InputAction& action = actions.at(actionName);
					for (auto& event : action.OnTrigger)
					{
						event(data);
					}
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

	InputType InputActionManager::ProcessKey(bool current, bool prev)
	{
		if (current && !prev)
			return InputType::Pressed;
		if (!current && prev)
			return InputType::Released;
		if (current && prev)
			return InputType::Down;
		if (!current && !prev)
			return InputType::Up;

		return InputType::Up;
	}

	InputType InputActionManager::ProcessMouse(InputBindings mouseInput)
	{
		using namespace InputManager;

		bool current;
		bool prev;
		
		switch (mouseInput)
		{

		case 74:
			if (MouseLeftPress())
				return InputType::Pressed;
			if (MouseLeftRelease())
				return InputType::Released;
			if (MouseLeftDown())
				return InputType::Down;
			if (MouseLeftUp())
				return InputType::Up;
			break;

		case 75:
			if (MouseRightPress())
				return InputType::Pressed;
			if (MouseRightRelease())
				return InputType::Released;
			if (MouseRightDown())
				return InputType::Down;
			if (MouseRightUp())
				return InputType::Up;
			break;
			
		case 76:
			if (MouseMiddlePress())
				return InputType::Pressed;
			if (MouseMiddleRelease())
				return InputType::Released;
			if (MouseMiddleDown())
				return InputType::Down;
			if (MouseMiddleUp())
				return InputType::Up;
			break;

		case 77:
			current = GetMouseWheel() > 0;
			prev = GetPrevMouseWheel() > 0;

			if (current && !prev)
				return InputType::Pressed;
			if (!current && prev)
				return InputType::Released;
			if (current && prev)
				return InputType::Down;
			if (!current && !prev)
				return InputType::Up;
			break;

		case 78:
			current = GetMouseWheel() < 0;
			prev = GetPrevMouseWheel() < 0;

			if (current && !prev)
				return InputType::Pressed;
			if (!current && prev)
				return InputType::Released;
			if (current && prev)
				return InputType::Down;
			if (!current && !prev)
				return InputType::Up;
			break;

		default:
			return InputType::Up;
			break;
		}
		
	}
	std::any GetMouseValue(InputBindings mouseInput)
	{
		using namespace InputManager;

		switch (mouseInput)
		{

		case 99:
			return GetMouseWheel();
			break;

		case 98:
			XMFLOAT2 position(GetMouseX(), GetMouseY());
			return position;
			break;

		case 97:
			XMFLOAT2 delta(GetMouseXDelta(), GetMouseYDelta());
			return delta;
			break;

		default:
			return std::any();
		}
	}
}