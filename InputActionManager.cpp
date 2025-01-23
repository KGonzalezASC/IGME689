#include "InputActionManager.h"

namespace InputActionManager
{
	void CreateAction(const wchar_t* name)
	{
		//actions.insert(std::make_pair(name, InputAction(name)));
	}
	InputAction& GetAction(std::wstring name)
	{
		return actions.at(name);
	}
	void AssignKeyToAction(std::wstring actionName, InputBindings key)
	{
		actions.at(actionName).keys.push_back(key);
	}
	void RemoveKeyFromAction(std::wstring actionName, InputBindings key)
	{
		auto& action = actions.at(actionName);
		action.keys.erase(std::remove(action.keys.begin(), action.keys.end(), key), action.keys.end());
	}
}