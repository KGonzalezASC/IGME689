#pragma once
#include <unordered_map>
#include <functional>
#include <string>


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
		Down,
		Pressed,
		Released
	};

	typedef std::function<void()> ActionEvent;

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
	};

	struct InputData
	{
		InputType inputType;
		InputBindings key;
	};

	// ===== | Methods | =====
	void Initialize();
	void CreateAction(const wchar_t* name);
	InputAction& GetAction(std::wstring name);
	void AssignKeyToAction(std::wstring actionName, InputBindings key);
	void RemoveKeyFromAction(std::wstring actionName, InputBindings key);

	// ===== | Variables | =====
	std::unordered_map<std::wstring, InputAction> actions;
	std::unordered_map<InputBindings, std::pair<InputBindingType, uint16_t>> bindings;
}