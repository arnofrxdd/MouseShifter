            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                RAWMOUSE& rm = raw->data.mouse;

                // Update right button state
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                {
                    isRightButtonHeld = true;
                }
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                {
                    isRightButtonHeld = false;
                }

                // Only call UpdateVJoyFromMouse if right button is not held
                if (!isRightButtonHeld)
                {
                    UpdateVJoyFromMouse(rm, raw->header.hDevice);
                    HandleScrollInput(rm);
                }
            }
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

                RAWMOUSE& rm = raw->data.mouse;

                // <<< ADD HERE >>>
                lastDx += rm.lLastX;
                lastDy += rm.lLastY;

                // delete this lol
            }
