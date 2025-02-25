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
        // Set the previous state of the controller buttons
        // to the current state of the buttons
        prevConButtonStates[i] = controllerStates[i].Gamepad.wButtons;

        if (i == 0)
        //std::cout << "Prev: " << ((prevConButtonStates[i] & XINPUT_GAMEPAD_A) != 0);

        // Clear the memeory of the current state of the conState
        ZeroMemory(&controllerStates[i], sizeof(XINPUT_STATE));

        // Attempt to retrieve data from the controller
        DWORD result = XInputGetState(i, &controllerStates[i]);

        // Depending on the result, process accordingly
        if (result == ERROR_SUCCESS)
        {
            WORD wButtons = controllerStates[i].Gamepad.wButtons;

            //std::cout << "Now: " << ((wButtons & 0x1000) != 0) << std::endl;

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

        }
    }
}

InputType XInputManager::CheckButtonState(uint16_t button, int index)
{ 
    WORD wButtons = controllerStates[index].Gamepad.wButtons;
    bool isPressed = (wButtons & button) != 0;

    wButtons = prevConButtonStates[index];
    bool wasPressed = (wButtons & button) != 0;

    InputType type = CheckButtonState(isPressed, wasPressed);

    return type;
}

std::any XInputManager::GetValueFromController(InputBindings value, int index)
{

	// Check the value of the input binding
    switch (value)
    {
        case InputBindings::XControllerLeftTrigger:
            return controllerStates[index].Gamepad.bLeftTrigger;
        case InputBindings::XControllerRightTrigger:
            return controllerStates[index].Gamepad.bRightTrigger;
        case InputBindings::XControllerLeftStick:
            XMFLOAT2 leftValue({ (float)controllerStates[index].Gamepad.sThumbLX,
                (float)controllerStates[index].Gamepad.sThumbLY });
            return leftValue;
        case InputBindings::XControllerRightStick:
            XMFLOAT2 rightValue({ (float)controllerStates[index].Gamepad.sThumbRX,
                (float)controllerStates[index].Gamepad.sThumbRY });
			return rightValue;
    }
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
