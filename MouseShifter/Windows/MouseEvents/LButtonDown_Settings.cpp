        if (showSettingsPanel)
        {
            // --- Complex Custom Controls First (Dropdowns, specialized buttons) ---
            // These take priority over the generic registry elements
            
            // DLL Injection Controls
            r = GetScrolledRect(processComboRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                processPickerModalOpen = true;
                processPickerScrollTarget = 0;
                processPickerScrollOffset = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            r = GetScrolledRect(refreshButtonRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }

            r = GetScrolledRect(mouseBlockCheckboxRect);
            if (PtInRect(&r, pt)) {
                g_mouseBlockEnabled = !g_mouseBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }

            r = GetScrolledRect(xinputBlockCheckboxRect);
            if (PtInRect(&r, pt)) {
                g_xinputBlockEnabled = !g_xinputBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }

            // Dropdown Buttons
            r = GetScrolledRect(gearLayoutButtonRect);
            if (PtInRect(&r, pt)) { gearLayoutDropdownOpen = !gearLayoutDropdownOpen; hShifterLayoutDropdownOpen = false; profileDropdownOpen = false; InvalidateRect(hwnd, &settingsPanelRect, FALSE); return 0; }

            r = GetScrolledRect(hShifterLayoutButtonRect);
            if (PtInRect(&r, pt)) { hShifterLayoutDropdownOpen = !hShifterLayoutDropdownOpen; gearLayoutDropdownOpen = false; profileDropdownOpen = false; InvalidateRect(hwnd, &settingsPanelRect, FALSE); return 0; }

            r = GetScrolledRect(profileButtonRect);
            if (PtInRect(&r, pt)) { profileDropdownOpen = !profileDropdownOpen; gearLayoutDropdownOpen = false; hShifterLayoutDropdownOpen = false; InvalidateRect(hwnd, &settingsPanelRect, FALSE); return 0; }

            r = GetScrolledRect(createProfileButtonRect);
            if (PtInRect(&r, pt))
            {
                 creatingNewProfile = true;
                 newProfileName = "New Profile";
                 profileTextSelected = true;
                 profileTextSelectionStart = 0;
                 profileTextSelectionEnd = newProfileName.length();
                 InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                 return 0;
            }

            // --- Registry-based Hit Testing ---
            std::wstring section = L"";
            for (auto& element : g_settingsRegistry)
            {
                if (element.type == SettingType::HEADING) section = element.label;

                r = GetScrolledRect(element.rect);
                if (PtInRect(&r, pt))
                {
                    if (element.type == SettingType::HEADING)
                    {
                        g_collapsedSections[element.label] = !g_collapsedSections[element.label];
                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        return 0;
                    }

                    if (!section.empty() && g_collapsedSections[section]) continue;

                    if (element.type == SettingType::TOGGLE)
                    {
                        bool* val = (bool*)element.valuePtr;
                        *val = !(*val);

                        // Specialized toggle logic
                        if (element.label == L"Precision Knob Move") {
                            // original logic used inverted bool visually
                        }
                        if (element.label == L"Enable Neutral") {
                             if (!isNeutralEnabled && neutralHeld) {
                                GearInput gi = gearInputMap["N"];
                                if (gi.type == KEYBOARD) {
                                    INPUT input = {}; input.type = INPUT_KEYBOARD; input.ki.wVk = gi.code; input.ki.dwFlags = KEYEVENTF_KEYUP;
                                    SendInput(1, &input, sizeof(INPUT));
                                } else if (gi.type == MOUSE) {
                                    INPUT input = {}; input.type = INPUT_MOUSE; 
                                    input.mi.dwFlags = (gi.code == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTUP : gi.code == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_MIDDLEUP);
                                    SendInput(1, &input, sizeof(INPUT));
                                } else if (gi.type == VJOY_BUTTON) {
                                    SetBtn(false, (UINT)vjoyDeviceId, gi.code);
                                }
                                if (keybindAnimations.find("N") != keybindAnimations.end()) {
                                    keybindAnimations["N"].isHeld = false; keybindAnimations["N"].isActive = false; keybindAnimations["N"].activationTime = GetTickCount();
                                }
                                neutralHeld = false;
                            }
                        }
                        if (element.label == L"Hide High Gears" || element.label == L"Use Right Stick for Knob" || element.label == L"16-Gear Set") {
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                        }
                        if (element.label == L"Use Throttle/Brake (W/S)" && !useThrottleBrakeAxes) {
                            ResetThrottleBrake();
                        }

                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        if (element.label == L"Fancy Knob Mode") InvalidateRect(hwnd, NULL, TRUE);
                        return 0; // Handled
                    }
                    if (element.type == SettingType::SLIDER_INT || element.type == SettingType::SLIDER_FLOAT || element.type == SettingType::SLIDER_BYTE)
                    {
                        // Check Reset Button first
                        r = GetScrolledRect(element.resetRect);
                        if (PtInRect(&r, pt))
                        {
                            if (element.type == SettingType::SLIDER_INT) *(int*)element.valuePtr = (int)element.defaultValue;
                            else if (element.type == SettingType::SLIDER_BYTE) *(unsigned char*)element.valuePtr = (unsigned char)element.defaultValue;
                            else *(float*)element.valuePtr = element.defaultValue;

                            // Specialized reset logic
                            if (element.label == L"Gear Radius" || element.label == L"Snap Sensitivity") {
                                gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
                                ComputeIntersections();
                            }
                            if (element.label == L"Diagonal Assist Strength") {
                                enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
                                for (auto& inter : intersections) inter.radius = int(baseIntersectionRadius * diagonalAssist);
                            }
                            if (element.label == L"H-Shifter Size") {
                                ComputeLayout(hwnd);
                                ComputeIntersections();
                            }

                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            return 0;
                        }

                        g_draggingElement = &element;
                        SetCapture(hwnd);
                        return 0; // Handled
                    }
                }
            }
        }