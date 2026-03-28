            // Steering Sensitivity
            r = GetScrolledRect(steeringSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(steeringDegreesSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringDegreesSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Acc/Brake Sensitivity
            r = GetScrolledRect(accBrakeSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingAccBrakeSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(scrollSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingScrollSensSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                showClutchIndicator = !showClutchIndicator;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Steering toggle ---
            r = GetScrolledRect(mouseSteeringToggleRect);
            if (PtInRect(&r, pt))
            {
                mouseSteeringEnabled = !mouseSteeringEnabled;

                if (IsKnobToggleActive())
                {
                    mouseTrackingEnabled = false;
                    knobMovementEnabled = true;
                }
                else
                {
                    mouseTrackingEnabled = mouseSteeringEnabled;
                    knobMovementEnabled = !mouseSteeringEnabled;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Device Selector ---
            r = GetScrolledRect(deviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t idx = 0;
                if (g_selectedDevice)
                {
                    for (size_t i = 0; i < g_mouseDevices.size(); ++i)
                    {
                        if (g_mouseDevices[i].hDevice == g_selectedDevice)
                        {
                            idx = i + 1;
                            break;
                        }
                    }
                }
                if (idx >= g_mouseDevices.size())
                {
                    g_selectedDevice = NULL; // All Mice
                }
                else
                {
                    g_selectedDevice = g_mouseDevices[idx].hDevice;
                }

                InvalidateRect(hwnd, &settingsPanelRect, TRUE); // redraw
            }
            r = GetScrolledRect(gamepadComboRect);
            if (PtInRect(&r, pt)) // `pt` is mouse click position
            {
                if (!g_gamepads.empty()) {
                    // cycle through only real gamepads
                    g_selectedGamepadIndex = (g_selectedGamepadIndex + 1) % g_gamepads.size();
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }


            // --- Steering Device Selector ---
            r = GetScrolledRect(steeringDeviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t index = 0;
                if (g_selectedSteeringDevice)
                {
                    for (index = 0; index < g_mouseDevices.size(); index++)
                    {
                        if (g_mouseDevices[index].hDevice == g_selectedSteeringDevice)
                        {
                            break;
                        }
                    }
                    index = (index + 1) % (g_mouseDevices.size() + 1); // +1 for "All Mice"
                }
                else
                {
                    index = 0; // start at first device if currently on "All Mice"
                }

                if (index < g_mouseDevices.size())
                {
                    g_selectedSteeringDevice = g_mouseDevices[index].hDevice;
                }
                else
                {
                    g_selectedSteeringDevice = NULL; // "All Mice"
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            r = GetScrolledRect(gearLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                gearLayoutDropdownOpen = !gearLayoutDropdownOpen;
                hShifterLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(hShifterLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                hShifterLayoutDropdownOpen = !hShifterLayoutDropdownOpen;
                gearLayoutDropdownOpen = false; // Close other dropdown
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
