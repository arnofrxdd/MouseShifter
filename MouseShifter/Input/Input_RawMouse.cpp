void ProcessRawInput(RAWINPUT* raw)
{
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        RAWKEYBOARD& kb = raw->data.keyboard;

        // F11 toggle using raw input
        static bool wasF11Down = false;
        if (kb.VKey == VK_F11)
        {
            bool isDown = (kb.Message == WM_KEYDOWN || kb.Message == WM_SYSKEYDOWN);
            if (isDown && !wasF11Down)
            {
                vJoyMouseEnabled = !vJoyMouseEnabled; // toggle on press
            }
            wasF11Down = isDown;
        }
    }

    for (auto& mapping : inputMap)
    {
        if (mapping.type == KEYBOARD && raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            RAWKEYBOARD& kb = raw->data.keyboard;

            if (kb.Message == WM_KEYDOWN && kb.VKey == mapping.code)
            {
                SetBtn(true, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = true;  // <--- add this
            }
            else if (kb.Message == WM_KEYUP && kb.VKey == mapping.code)
            {
                SetBtn(false, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = false; // <--- add this
            }

        }
        else if (mapping.type == MOUSE && raw->header.dwType == RIM_TYPEMOUSE)
        {
            RAWMOUSE& rm = raw->data.mouse;
            bool pressed = false;
            bool released = false;

            if (mapping.code == 1)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP;
            }
            else if (mapping.code == 2)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP;
            }
            else if (mapping.code == 3)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP;
            }
            else if (mapping.code == 4) {
                pressed = rm.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_BUTTON_4_UP;
            }
            else if (mapping.code == 5) {
                pressed = rm.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_BUTTON_5_UP;
            }


            if (pressed)
            {
                SetBtn(true, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = true; // mark pressed
            }
            if (released)
            {
                SetBtn(false, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = false; // mark released
            }

        }
    }
}

void UpdateKnobMovement(HWND hwnd)
{
    if (!mouseSteeringEnabled)
        return;

    bool sameDevice = (g_selectedDevice != NULL && g_selectedSteeringDevice != NULL &&
        g_selectedDevice == g_selectedSteeringDevice);
    if (!sameDevice)
    {
        if (knobDisabledByF9)
        {
            // When disabled by F9, only allow temporary enable with toggle
            knobMovementEnabled = IsKnobToggleActive();
            mouseTrackingEnabled = !IsKnobToggleActive();
        }
        else
        {
            // Normal case: knob always enabled + mouse tracking
            knobMovementEnabled = true;
            mouseTrackingEnabled = true;
        }
    }

    else
    {
        // Devices are the same: use Shift key logic
        if (IsKnobToggleActive())
        {
            knobMovementEnabled = true;
            mouseTrackingEnabled = false;
        }
        else
        {
            knobMovementEnabled = false;
            mouseTrackingEnabled = true;
        }
    }
}
// Variables now defined at top of file


