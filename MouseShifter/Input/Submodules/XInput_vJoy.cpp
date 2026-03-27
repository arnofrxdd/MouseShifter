#include <chrono>

// Globals
std::chrono::steady_clock::time_point rbReleaseTime;
bool rbWasPressed = false;
const int RB_DELAY_MS = 40;
void UpdateVJoyFromXInput()
{
    if (g_selectedGamepadIndex < 0 || g_selectedGamepadIndex >= g_gamepads.size())
        return;

    Gamepad* g = &g_gamepads[g_selectedGamepadIndex];
    if (!g->controller)
        return;

    SDL_GameControllerUpdate();

    // --- LT as clutch ---
    if (useLTAsClutch && !useThrottleBrakeAxes)
    {
        // SDL axis range is -32768 → 32767 for triggers too, but SDL triggers usually 0–32767
        int ltValue = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);

        LONG zAxisValue;
        if (bindingModeForRAxis && ltValue > 1000) // Little bit of movement detection for clutch
        {
            zAxisValue = axisMax; // Full value when binding mode is active
        }
        else
        {
            zAxisValue = axisMin + (ltValue / 32767.0) * (axisMax - axisMin);
        }
        SetAxis(zAxisValue, vjoyDeviceId, HID_USAGE_Z);
    }

    if (!disableRealKnobMovement)
        return;

    // --- Assist button ---
    bool rbPressed = false;
    if (assistButton >= 0 && assistButton < SDL_CONTROLLER_BUTTON_MAX)
        rbPressed = SDL_GameControllerGetButton(g->controller, (SDL_GameControllerButton)assistButton) != 0;

    // --- Inverted assist axis logic ---
    if (invertAssistAxes)
    {
        if (!rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            return;
        }
        // else → assist pressed → continue normal stick processing
    }
    else
    {
        if (rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            rbWasPressed = true;
            return;
        }
        else if (rbWasPressed)
        {
            rbReleaseTime = std::chrono::steady_clock::now();
            rbWasPressed = false;
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if ((now - rbReleaseTime) < std::chrono::milliseconds(RB_DELAY_MS))
            return;
    }

    // --- Process right stick normally ---
    int sThumbRX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTX);
    int sThumbRY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTY);

    double normRX = 0.0, normRY = 0.0;
    const int DEADZONE = 8000; // adjust as needed

    if (bindingModeForRAxis)
    {
        // Binding mode: go to full value with little movement
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = (sThumbRX > 0) ? 1.0 : -1.0; // Full value
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = (sThumbRY > 0) ? 1.0 : -1.0; // Full value
    }
    else
    {
        // Normal mode
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = sThumbRX / 32767.0;
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = sThumbRY / 32767.0;
    }

    joyRX = (LONG)(axisMin + (normRX + 1.0) * 0.5 * (axisMax - axisMin));
    joyRY = (LONG)(axisMin + (normRY + 1.0) * 0.5 * (axisMax - axisMin));

    SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
    SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
}