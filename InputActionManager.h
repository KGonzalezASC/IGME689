#pragma once
#include <unordered_map>
#include <functional>
#include <string>


namespace InputActionManager
{
	enum InputBindings;

	enum InputType
	{
		Down,
		Pressed,
		Released
	};

	std::unordered_map<std::wstring, InputAction> actions;

	typedef std::function<void()> ActionEvent;

	// ===== | Structs | =====
	struct InputAction
	{
		wchar_t* name;
		std::vector<ActionEvent> OnTrigger;



		InputAction(const wchar_t* name)
		{
			this->name = new wchar_t[wcslen(name) + 1];
			wcscpy(this->name, name);
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
	void CreateAction(const wchar_t* name);
	InputAction& GetAction(std::wstring name);
	void AssignKeyToAction(std::wstring actionName, InputBindings key);
	void RemoveKeyFromAction(std::wstring actionName, InputBindings key);
}