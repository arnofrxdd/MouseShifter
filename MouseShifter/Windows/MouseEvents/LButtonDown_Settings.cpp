        if (showSettingsPanel)
        {
            RECT r; // temporary rect for PtInRect

            r = GetScrolledRect(knobSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingKnobSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(gearSliderRect);
            if (PtInRect(&r, pt))
            {
                isDraggingGearRadius = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(sensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSensSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(diagSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingDiagSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapInSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapInSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(noReverseToggleRect);
            if (PtInRect(&r, pt))
            {
                noReverseLayout = !noReverseLayout;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                ComputeLayout(hwnd);
                ComputeIntersections();
            }
            r = GetScrolledRect(hideHighGearsToggleRect);
            if (PtInRect(&r, pt))
            {
                hideHighGears = !hideHighGears;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw

                ComputeLayout(hwnd); // if needed
            }
            r = GetScrolledRect(useScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useScrollClutch = !useScrollClutch;           // toggle the flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            r = GetScrolledRect(reverseLockToggleRect);
            if (PtInRect(&r, pt))
            {
                reverseLockEnabled = !reverseLockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // DLL INJECTION CLICK 

            // --- DLL Injection Controls ---
            r = GetScrolledRect(processComboRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                if (!g_processList.empty()) {
                    static size_t currentIndex = 0;
                    currentIndex = (currentIndex + 1) % g_processList.size();
                    g_selectedProcessId = g_processList[currentIndex];
                }
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(refreshButtonRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(autoInjectButtonRect);
            if (PtInRect(&r, pt))
            {
                g_autoInjectEnabled = !g_autoInjectEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Toggle Mouse Block checkbox
            r = GetScrolledRect(mouseBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_mouseBlockEnabled = !g_mouseBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Toggle XInput Block checkbox
            r = GetScrolledRect(xinputBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_xinputBlockEnabled = !g_xinputBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(smoothScrollSlider);
            if (PtInRect(&r, pt))
            {
                draggingSmoothScrollSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(brakeresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingBrakeResistanceSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(accelerationresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingAccelerationResistanceSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapSpeedSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapSpeedSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(layoutScaleSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingLayoutSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(useAxisSmoothingToggle);
            if (PtInRect(&r, pt))
            {
                useAxisSmoothing = !useAxisSmoothing;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Add this with your other slider dragging handlers:
            r = GetScrolledRect(axisSmoothingFactorSlider);
            if (PtInRect(&r, pt))
            {
                draggingAxisSmoothingFactorSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(neutralToggleRect);
            if (PtInRect(&r, pt))
            {
                bool wasEnabled = isNeutralEnabled;
                isNeutralEnabled = !isNeutralEnabled;

                // If Neutral was just disabled, release the Neutral key
                if (wasEnabled && !isNeutralEnabled && neutralHeld)
                {
                    GearInput gi = gearInputMap["N"];
                    if (gi.type == KEYBOARD)
                    {
                        INPUT input = {};
                        input.type = INPUT_KEYBOARD;
                        input.ki.wVk = gi.code;
                        input.ki.dwFlags = KEYEVENTF_KEYUP;
                        SendInput(1, &input, sizeof(INPUT));
                    }
                    else if (gi.type == MOUSE)
                    {
                        INPUT input = {};
                        input.type = INPUT_MOUSE;
                        input.mi.dwFlags = (gi.code == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTUP : gi.code == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTUP
                            : MOUSEEVENTF_MIDDLEUP);
                        SendInput(1, &input, sizeof(INPUT));
                    }
                    else if (gi.type == VJOY_BUTTON)
                    {
                        SetBtn(false, vjoyDeviceId, gi.code);
                    }

                    // Stop Neutral glow animation
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();
                    }

                    neutralHeld = false;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(showYBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showYBar = !showYBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(invertScrollToggleRect);
            if (PtInRect(&r, pt))
            {
                invertScrollClutchAxis = !invertScrollClutchAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Binding Mode For R-Axis toggle
            r = GetScrolledRect(bindingModeForAxisToggle);
            if (PtInRect(&r, pt))
            {
                bindingModeForRAxis = !bindingModeForRAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(realisticKnobToggleRect);
            if (PtInRect(&r, pt))
            {
                realisticKnob = !realisticKnob;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                InvalidateRect(hwnd, NULL, TRUE); // redraw main window to update knob appearance
            }
            // Clutch Lock Gear Toggle
            r = GetScrolledRect(clutchLockGearToggleRect);
            if (PtInRect(&r, pt))
            {
                clutchLockGear = !clutchLockGear;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Invert Assist Axes toggle
            r = GetScrolledRect(invertAssistToggleRect);
            if (PtInRect(&r, pt))
            {
                invertAssistAxes = !invertAssistAxes;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(yBarAlphaSlider);
            if (PtInRect(&r, pt))
            {
                draggingYBarAlphaSlider = true;
                SetCapture(hwnd);
            }

            // Add this after the showYBar toggle click handling:
            r = GetScrolledRect(yBarFixedTransToggle);
            if (PtInRect(&r, pt))
            {
                useYbarFixedTransparency = !useYbarFixedTransparency;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Use LT as Clutch toggle
            r = GetScrolledRect(useLTAsClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useLTAsClutch = !useLTAsClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showXBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showXBar = !showXBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(knobAccelToggleRect);
            if (PtInRect(&r, pt))
            {
                knobAccelerationEnabled = !knobAccelerationEnabled; // toggle boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);    // redraw
            }
            r = GetScrolledRect(useXInputToggleRect);
            if (PtInRect(&r, pt))
            {
                useXInput = !useXInput;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(halfScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useHalfClutch = !useHalfClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(optimizationToggleRect);
            if (PtInRect(&r, pt))
            {
                disableSmartRedraws = !disableSmartRedraws; // Toggle the actual flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(controllerSensSliderRect);
            if (PtInRect(&r, pt))
            {
                controllerDraggingSensSlider = true;
                SetCapture(hwnd);
            }
            // --- Use Right Stick for Knob Toggle ---
            r = GetScrolledRect(useRightStickToggleRect);
            if (PtInRect(&r, pt))
            {
                useRightStick = !useRightStick; // toggle the boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(disableRealKnobMovementToggleRect);
            if (PtInRect(&r, pt))
            {
                disableRealKnobMovement = !disableRealKnobMovement;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Add this with your other toggle click handlers:
            r = GetScrolledRect(useThrottleBrakeToggleRect);
            if (PtInRect(&r, pt))
            {
                useThrottleBrakeAxes = !useThrottleBrakeAxes;

                // If disabling, reset throttle/brake to prevent stuck inputs
                if (!useThrottleBrakeAxes)
                {
                    ResetThrottleBrake();
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(dynamicTransparencyToggleRect);
            if (PtInRect(&r, pt))
            {
                dynamicTransparencyEnabled = !dynamicTransparencyEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(minTransparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingMinTransparencySlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(transparencyFadeDelaySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencyFadeDelaySlider = true;
                SetCapture(hwnd);

            }

            r = GetScrolledRect(transparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencySlider = true;
                SetCapture(hwnd);
            }