#include "InputActionManager.h"

namespace InputActionManager
{
	//void Initialize()
	//{
	//	using BindingPair = std::pair<InputBindingType, uint16_t>;

	//	bindings.clear();

	//	// Helper lambdas to add keyboard/mouse entries more succinctly
	//	auto addKey = [&](InputBindings binding, uint16_t vkCode) {
	//		bindings.emplace(binding, BindingPair(Keyboard, vkCode));
	//		};
	//	auto addMouse = [&](InputBindings binding, uint16_t vkCode) {
	//		bindings.emplace(binding, BindingPair(Mouse, vkCode));
	//		};
	//}

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
		//actions.at(actionName).keys.push_back(key);
	}

	void RemoveKeyFromAction(std::wstring actionName, InputBindings key)
	{
		//auto& action = actions.at(actionName);
		//action.keys.erase(std::remove(action.keys.begin(), action.keys.end(), key), action.keys.end());
	}

	enum InputBindings
	{
		// Keyboard keys
		KeyA,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		KeyEscape,
		KeySpace,
		KeyEnter,
		KeyTab,
		KeyBackspace,
		KeyInsert,
		KeyDelete,
		KeyRight,
		KeyLeft,
		KeyDown,
		KeyUp,
		KeyPageUp,
		KeyPageDown,
		KeyHome,
		KeyEnd,
		KeyCapsLock,
		KeyScrollLock,
		KeyNumLock,
		KeyPrintScreen,
		KeyPause,
		KeyF1,
		KeyF2,
		KeyF3,
		KeyF4,
		KeyF5,
		KeyF6,
		KeyF7,
		KeyF8,
		KeyF9,
		KeyF10,
		KeyF11,
		KeyF12,
		KeyLeftShift,
		KeyRightShift,
		KeyLeftControl,
		KeyRightControl,
		KeyLeftAlt,
		KeyRightAlt,

		// Mouse buttons
		MouseLeftButton,
		MouseRightButton,
		MouseMiddleButton,

		// Mouse wheel
		MouseWheelUp,
		MouseWheelDown
	};
}