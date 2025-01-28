#pragma once
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

#include "InputManager.h"

namespace InputActionManager
{
	// ===== | Enums | =====
	enum InputBindings;
	enum InputBindingType
	{
		Keyboard,
		Mouse,
	};
	enum InputType
	{
		Up,
		Down,
		Pressed,
		Released,
	};

	typedef std::function<void()> ActionEvent;

	// ===== | Structs | =====
	struct InputAction;

	struct InputData;

	struct InputActionHash;

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

	// ===== | Variables | =====

	extern std::unordered_map<std::wstring, InputAction> actions;
	extern std::unordered_map<InputBindings, std::pair<InputBindingType, uint16_t>> bindings;
	extern std::unordered_map<InputBindings, std::unordered_set<InputAction, InputActionHash>> actionBindings;
}