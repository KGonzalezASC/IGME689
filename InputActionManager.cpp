#include "InputActionManager.h"

namespace InputActionManager
{
	std::unordered_map<std::wstring, InputAction> actions;
	std::unordered_map<InputBindings, std::pair<InputBindingType, uint16_t>> bindings;
	std::unordered_map<InputBindings, std::unordered_set<InputAction, InputActionHash>> actionBindings;

	// ===== | Structs | =====
	struct InputAction
	{
		wchar_t* name;
		std::vector<ActionEvent> OnTrigger;

		InputAction(const wchar_t* name)
		{
			this->name = new wchar_t[wcslen(name) + 1];
			wcscpy_s(this->name, wcslen(name) + 1, name);
		}

		~InputAction()
		{
			delete[] name;
		}

		bool operator==(const InputAction& other) const
		{
			return wcscmp(name, other.name) == 0;
		}
	};

	struct InputData
	{
		InputType inputType;
		InputBindings key;
	};

	struct InputActionHash
	{
		std::size_t operator()(const InputAction& action) const
		{
			return std::hash<std::wstring>()(action.name);
		}
	};

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
	}

	void InputActionManager::CreateAction(const wchar_t* name)
	{
		actions.insert(std::make_pair(name, InputAction(name)));
	}

	InputAction& InputActionManager::GetAction(std::wstring name)
	{
		return actions.at(name);
	}

	void InputActionManager::AssignBindingToAction(std::wstring actionName, InputBindings key)
	{
		InputActionManager::actionBindings[key].insert(actions.at(actionName));
	}

	void InputActionManager::RemoveBindingFromAction(std::wstring actionName, InputBindings key)
	{
		actionBindings[key].erase(actions.at(actionName));
	}

	void InputActionManager::CheckActionBindings()
	{
		// Loop through all the bindings in actionBindings
		for (auto& binding : actionBindings)
		{
			InputBindings input = binding.first;
			InputBindingType type = bindings[input].first;

			// Loop through all the actions associated with this binding
			for (auto& action : binding.second)
			{
				
			}
		}
	}

	InputType InputActionManager::ProcessKey(uint16_t key)
	{
		if (InputManager::KeyPress)
			return InputType::Pressed;
		if (InputManager::KeyRelease(key))
			return InputType::Released;
		if (InputManager::KeyDown(key))
			return InputType::Down;
		if (InputManager::KeyUp(key))
			return InputType::Up;

		return InputType::Up;
	}

	enum InputBindings
	{
		// Keyboard keys
		KeyA = 0,
		KeyB = 1,
		KeyC = 2,
		KeyD = 3,
		KeyE = 4,
		KeyF = 5,
		KeyG = 6,
		KeyH = 7,
		KeyI = 8,
		KeyJ = 9,
		KeyK = 10,
		KeyL = 11,
		KeyM = 12,
		KeyN = 13,
		KeyO = 14,
		KeyP = 15,
		KeyQ = 16,
		KeyR = 17,
		KeyS = 18,
		KeyT = 19,
		KeyU = 20,
		KeyV = 21,
		KeyW = 22,
		KeyX = 23,
		KeyY = 24,
		KeyZ = 25,
		Key0 = 26,
		Key1 = 27,
		Key2 = 28,
		Key3 = 29,
		Key4 = 30,
		Key5 = 31,
		Key6 = 32,
		Key7 = 33,
		Key8 = 34,
		Key9 = 35,
		KeyEscape = 36,
		KeySpace = 37,
		KeyEnter = 38,
		KeyTab = 39,
		KeyBackspace = 40,
		KeyInsert = 41,
		KeyDelete = 42,
		KeyRight = 43,
		KeyLeft = 44,
		KeyDown = 45,
		KeyUp = 46,
		KeyPageUp = 47,
		KeyPageDown = 48,
		KeyHome = 49,
		KeyEnd = 50,
		KeyCapsLock = 51,
		KeyScrollLock = 52,
		KeyNumLock = 53,
		KeyPrintScreen = 54,
		KeyPause = 55,
		KeyF1 = 56,
		KeyF2 = 57,
		KeyF3 = 58,
		KeyF4 = 59,
		KeyF5 = 60,
		KeyF6 = 61,
		KeyF7 = 62,
		KeyF8 = 63,
		KeyF9 = 64,
		KeyF10 = 65,
		KeyF11 = 66,
		KeyF12 = 67,
		KeyLeftShift = 68,
		KeyRightShift = 69,
		KeyLeftControl = 70,
		KeyRightControl = 71,
		KeyLeftAlt = 72,
		KeyRightAlt = 73,

		// Mouse buttons
		MouseLeftButton = 74,
		MouseRightButton = 75,
		MouseMiddleButton = 76,

		// Mouse wheel
		MouseWheelUp = 77,
		MouseWheelDown = 78
	};

}