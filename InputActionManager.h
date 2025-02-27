#pragma once 

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include <string>
#include <memory>
#include <utility>
#include "InputManager.h"
#include "InputValue.h"

namespace InputActionManager
{
	// ===== | Enums | =====
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
		MouseWheelDown = 78,

		// Xinput controller
		XControllerA = 79,
		XControllerB = 80,
		XControllerX = 81,
		XControllerY = 82,
		XControllerDPadUp = 83,
		XControllerDPadDown = 84,
		XControllerDPadLeft = 85,
		XControllerDPadRight = 86,
		XControllerStart = 87,
		XControllerBack = 88,
		XControllerLeftThumb = 89,
		XControllerRightThumb = 90,
		XControllerLeftShoulder = 91,
		XControllerRightShoulder = 92,

		XControllerLeftStick = 93,
		XControllerRightStick = 94,

		XControllerLeftTrigger = 95,
		XControllerRightTrigger = 96,

		// Mouse Values
		MouseDelta = 97,
		MousePosition = 98,
		MouseWheelDelta = 99
	};

	enum InputBindingType
	{
		Keyboard,
		Mouse,
		XController,
	};

	enum InputType
	{
		Up,
		Down,
		Pressed,
		Released,
		Value
	};

	struct InputData
	{
		InputType inputType;
		InputBindings key;
		uint16_t controllerIndex;
		InputValue value = InputValue(nullptr);
	};

	typedef std::function<void(InputData)> ActionEvent;

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
			if (name != nullptr)
				delete name;
		}

		InputAction(InputAction&& other) noexcept
		{
			name = other.name;
			other.name = nullptr;
			
		}

		bool operator==(const InputAction& other) const
		{
			return wcscmp(name, other.name) == 0;
		}
	};

	// ===== | Methods | =====
	void Initialize();
	// Create a new action an add it to the actions map
	void CreateAction(const wchar_t* name);
	// Gets an action from the actions map by name
	InputAction& GetAction(std::wstring name);
	// Takes a inputBining and assosiates it with an action
	void AssignBindingToAction(std::wstring actionName, InputBindings key);
	// Disassociates a key from an action
	void RemoveBindingFromAction(std::wstring actionName, InputBindings key);
	// Checks if a key assosiated to a binding is has been iteracted with
	void CheckActionBindings();
	InputType ProcessKey(uint16_t key);
	InputType ProcessKey(bool current, bool prev);
	InputType ProcessMouse(InputBindings mouseInput);
	std::any GetMouseValue(InputBindings mouseInput);

	// ===== | Variables | =====
	extern std::unordered_map<std::wstring, InputAction> actions;
	extern std::unordered_map<InputBindings, std::pair<InputBindingType, uint16_t>> bindings;
	extern std::unordered_map<InputBindings, std::unordered_set<std::wstring>> actionBindings;
}