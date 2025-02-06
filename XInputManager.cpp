#include "XInputManager.h"
#include <iostream>

XInputManager::XInputManager()
{
}

XInputManager::~XInputManager()
{
}

void XInputManager::CheckControllerState(DWORD dwUserIndex)
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    // Retrieve the state of the controller from XInput
    DWORD dwResult = XInputGetState(dwUserIndex, &state);

    if (dwResult == ERROR_SUCCESS)
    {
        // Controller is connected
        // state.Gamepad will have the input data
        // For instance:
        SHORT lx = state.Gamepad.sThumbLX;
        SHORT ly = state.Gamepad.sThumbLY;
        SHORT rx = state.Gamepad.sThumbRX;
        SHORT ry = state.Gamepad.sThumbRY;

        BYTE leftTrigger = state.Gamepad.bLeftTrigger;
        BYTE rightTrigger = state.Gamepad.bRightTrigger;

        WORD wButtons = state.Gamepad.wButtons;

        bool aButton = (wButtons & XINPUT_GAMEPAD_A) != 0;
        bool bButton = (wButtons & XINPUT_GAMEPAD_B) != 0;

        std::cout << "Controller " << dwUserIndex << " connected.\n"
            << "LX: " << lx << " LY: " << ly
            << " RX: " << rx << " RY: " << ry << "\n"
            << "LT: " << (int)leftTrigger
            << " RT: " << (int)rightTrigger << "\n"
            << "A: " << aButton << " B: " << bButton << std::endl;
    }
    else
    {
        // Controller not connected
        //std::cout << "Controller " << dwUserIndex << " not connected.\n";
    }
}
