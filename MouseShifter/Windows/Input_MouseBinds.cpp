            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                HANDLE device = raw->header.hDevice;
                bool match = false;

                if (g_selectedDevice == NULL)
                {
                    match = true; // All mice: accept every input
                }
                else
                {
                    // Specific device selected
                    for (auto& m : g_mouseDevices)
                    {
                        if (m.hDevice == g_selectedDevice)
                        {
                            // For touchpad, hDevice may be 0
                            if (m.isTouchpad ? device == 0 : device == g_selectedDevice)
                            {
                                match = true;
                                break;
                            }
                        }
                    }
                }

                if (!match)
                {
                    delete[] lpb;
                    return 0; // ignore input from other mice
                }
                // --- MOUSE BINDING CAPTURE ---
                RAWMOUSE& rm = raw->data.mouse;

                // Only process if user is setting an input from the panel
                if (!inputBeingSet.empty() && !showVJoyPicker)
                {
                    int idx = std::stoi(inputBeingSet); // which inputMap entry we're editing
                    inputMap[idx].type = MOUSE;

                    // Detect which mouse button was pressed
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        inputMap[idx].code = 1;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        inputMap[idx].code = 2;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        inputMap[idx].code = 3;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        inputMap[idx].code = 4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        inputMap[idx].code = 5;
                    else
                        return 0; // unknown button, ignore

                    inputBeingSet.clear(); // finished binding
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                if (toggleInputBeingSet && !togglePedalBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    toggleInputBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

                if (reverseUnlockBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    reverseUnlockBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

