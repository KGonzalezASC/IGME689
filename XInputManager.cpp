#include "XInputManager.h"
#include <iostream>

XInputManager * XInputManager::Instance = nullptr;

XInputManager::XInputManager()
{

}

XInputManager::~XInputManager()
{

}

void XInputManager::Initialize()
{
    Instance = new XInputManager();
}

void XInputManager::UpdateControllerStates()
{
    // Iterate over all of the controllers (There can be max, up to 4
    for (int i = 0; i < XUSER_MAX_COUNT; i++)
    {
        // Get the current state of the controller
        XINPUT_STATE conState = controllerStates[i];

        // Set the previous state of the controller buttons
        // to the current state of the buttons
        prevConButtonStates[i] = conState.Gamepad.wButtons;

        // Clear the memeory of the current state of the conState
        ZeroMemory(&conState, sizeof(XINPUT_STATE));

        // Attempt to retrieve data from the controller
        DWORD result = XInputGetState(i, &conState);

        // Depending on the result, process accordingly
        if (result == ERROR_SUCCESS)
        {
            WORD wButtons = conState.Gamepad.wButtons;

            bool aButton = (wButtons & XINPUT_GAMEPAD_A) != 0;
            bool bButton = (wButtons & XINPUT_GAMEPAD_B) != 0;

            /*if (aButton || bButton)
            {
                std::cout << "Controller " << i << " connected.\n"
                    << "A: " << aButton << " B: " << bButton << std::endl;
            }*/
        }
        else
        {
            // If it didn't succeed, make sure that XInput_State
            // is set to a null_ptr
            conState = {};
        }
    }
}

InputType XInputManager::CheckButtonState(uint16_t button, int index)
{ 
    if (controllerStates[index].Gamepad.wButtons != prevConButtonStates[index])
    {

    }

    WORD wButtons = controllerStates[index].Gamepad.wButtons;
    bool isPressed = (wButtons & button) != 0;

    wButtons = prevConButtonStates[index];
    bool wasPressed = (wButtons & button) != 0;

    InputType type = CheckButtonState(isPressed, wasPressed);

    std::cout << " State: " << type << std::endl;

    return type;
}

InputType XInputManager::CheckButtonState(bool currentInput, bool prevInput)
{
    if (currentInput && !prevInput)
	{
		return InputType::Pressed;
	}
	else if (!currentInput && prevInput)
	{
		return InputType::Released;
	}
	else if (currentInput && prevInput)
	{
		return InputType::Down;
	}
	else
	{
		return InputType::Up;
	}
}
