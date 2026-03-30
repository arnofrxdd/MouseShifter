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
                mouseDeviceDropdownOpen = true;
                steeringDeviceDropdownOpen = false;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
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
                steeringDeviceDropdownOpen = true;
                mouseDeviceDropdownOpen = false;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
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
