        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };

        // Handle ALL dropdown clicks first and block any other processing when dropdowns are open
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            bool handledInDropdown = false;

            // Handle profile dropdown clicks
            if (profileDropdownOpen) {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = profileButtonRect.top + 25; // Base Y position without scroll offset

                RECT dropdownRect = {
                    profileButtonRect.left,
                    listY + settingsScrollOffset,
                    profileButtonRect.right,
                    listY + settingsScrollOffset + (int)((listItemHeight + itemGap) * profileNames.size())
                };

                if (PtInRect(&dropdownRect, pt)) {
                    // Check if clicking on existing profiles
                    for (size_t i = 0; i < profileNames.size(); ++i) {
                        RECT itemRect = {
                            dropdownRect.left,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };

                        if (PtInRect(&itemRect, pt)) {
                            SwitchProfile((int)i, hwnd);
                            profileDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                // If we clicked anywhere (inside or outside dropdown), close it and block further processing
                if (!handledInDropdown) {
                    profileDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when profile dropdown was open
            }

            // Handle gear layout dropdown clicks
            if (gearLayoutDropdownOpen)
            {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
                int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();

                RECT dropdownRect = {
                    gearLayoutButtonRect.left,
                    listY,
                    gearLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < gearLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left,
                            listY + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentGearLayout = (int)i;
                            gearLabelOverride = gearLayouts[i];
                            gearLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    gearLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when gear layout dropdown was open
            }

            // Handle h-shifter layout dropdown clicks
            if (hShifterLayoutDropdownOpen)
            {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
                int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();

                RECT dropdownRect = {
                    hShifterLayoutButtonRect.left,
                    listY,
                    hShifterLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left,
                            listY + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentHShifterLayout = hShifterLayouts[i].id;
                            layoutType = currentHShifterLayout;
                            hShifterLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    hShifterLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when h-shifter dropdown was open
            }

            return 0; // Safety return
        }
        if (creatingNewProfile) {
            RECT adjustedProfileButtonRect = {
                profileButtonRect.left,
                profileButtonRect.top + settingsScrollOffset,
                profileButtonRect.right,
                profileButtonRect.bottom + settingsScrollOffset
            };
            RECT adjustedCreateProfileButtonRect = {
                createProfileButtonRect.left,
                createProfileButtonRect.top + settingsScrollOffset,
                createProfileButtonRect.right,
                createProfileButtonRect.bottom + settingsScrollOffset
            };

            // Check if click is NOT in the profile creation area
            if (!PtInRect(&adjustedProfileButtonRect, pt) && !PtInRect(&adjustedCreateProfileButtonRect, pt)) {
                // Clicked outside profile creation area - create the profile
                if (!newProfileName.empty()) {
                    CreateNewProfile(hwnd);
                }
                creatingNewProfile = false;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0; // Block further processing
            }
            // If click is inside profile creation area, continue with normal processing
        }

        // Only process these if NO dropdowns are open
        RECT adjustedProfileButtonRect = {
            profileButtonRect.left,
            profileButtonRect.top + settingsScrollOffset,
            profileButtonRect.right,
            profileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedProfileButtonRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }

        RECT adjustedCreateProfileButtonRect = {
            createProfileButtonRect.left,
            createProfileButtonRect.top + settingsScrollOffset,
            createProfileButtonRect.right,
            createProfileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedCreateProfileButtonRect, pt)) {
            creatingNewProfile = true;
            newProfileName = "New Profile";
            profileTextSelected = true;
            profileTextSelectionStart = 0;
            profileTextSelectionEnd = newProfileName.length();
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }


        // --- Block clicks through dropdown ---
        RECT togglePanelRectHit = {
            (LONG)togglePanelRectUnified.X,
            (LONG)togglePanelRectUnified.Y,
            (LONG)(togglePanelRectUnified.X + togglePanelRectUnified.Width),
            (LONG)(togglePanelRectUnified.Y + togglePanelRectUnified.Height) };
        RECT inputPanelRectHit = {
            (LONG)inputPanelRectUnified.X,
            (LONG)inputPanelRectUnified.Y,
            (LONG)(inputPanelRectUnified.X + inputPanelRectUnified.Width),
            (LONG)(inputPanelRectUnified.Y + inputPanelRectUnified.Height) };

        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();

            RECT dropdownRect = {
                gearLayoutButtonRect.left,
                listY,
                gearLayoutButtonRect.right,   // <-- use button right, not +150
                listY + totalHeight
            };


            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < gearLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        currentGearLayout = (int)i;
                        gearLabelOverride = gearLayouts[i];
                        gearLayoutDropdownOpen = false;
                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        ComputeLayout(hwnd);
                        ComputeIntersections();
                        break;
                    }
                }
                return 0;
            }
            else
            {
                gearLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();

            RECT dropdownRect = {
                hShifterLayoutButtonRect.left,
                listY,
                hShifterLayoutButtonRect.right, // <-- match button width
                listY + totalHeight
            };


            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        currentHShifterLayout = hShifterLayouts[i].id;
                        layoutType = currentHShifterLayout;
                        hShifterLayoutDropdownOpen = false;
                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        ComputeLayout(hwnd);
                        ComputeIntersections();
                        break;
                    }
                }
                return 0;
            }
            else
            {
                hShifterLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }

        if (showTogglePanel && PtInRect(&g_toggleKeyRect, pt))
        {
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            {
                // Shift+Click = enter pedal binding mode
                togglePedalBeingSet = true;
                toggleInputBeingSet = false;
                OutputDebugString(_T("Shift+Click -> Listening for pedal...\n"));
            }
            else
            {
                // Normal click = keyboard binding
                toggleInputBeingSet = true;
                togglePedalBeingSet = false;
                OutputDebugString(_T("Click -> Listening for keyboard key...\n"));
            }

            InvalidateRect(hwnd, nullptr, TRUE);
        }
        if (showTogglePanel && PtInRect(&g_assistButtonRect, pt))
        {
            assistButtonBeingSet = true;
            OutputDebugString(_T("Assist Button Clicked -> Listening for controller input...\n"));
            InvalidateRect(hwnd, nullptr, TRUE);
        }

        if (showTogglePanel && PtInRect(&reverseUnlockKeyRect, pt))
        {
            reverseUnlockBeingSet = true;
            OutputDebugString(_T("Reverse Unlock Clicked -> Listening for input...\n"));
            InvalidateRect(hwnd, nullptr, TRUE);
        }



        if (!isBorderless)
        {
            // Close button
            if (PtInRect(&g_CloseButtonRect, pt))
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            }

            // Minimize button functionality (for Maximize/Restore button)
            if (PtInRect(&g_MaxButtonRect, pt))
            {
                ShowWindow(hwnd, SW_MINIMIZE);
                break;
            }




            // Drag window by title bar (excluding buttons)
            if (pt.y >= 0 && pt.y < 30 && pt.x < g_MaxButtonRect.left)
            {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                break;
            }
        }
        // Update button click (add debug output)
        if (updateAvailable && !isBorderless && PtInRect(&updateButtonRect, pt))
        {
            char debugBuf[256];
            sprintf_s(debugBuf, "[UPDATE CLICK] Button clicked! Mouse: (%d,%d), Button: (%d,%d)-(%d,%d)\n",
                pt.x, pt.y,
                updateButtonRect.left, updateButtonRect.top,
                updateButtonRect.right, updateButtonRect.bottom);
            OutputDebugStringA(debugBuf);

            std::wstring currentVersionW(currentVersion.begin(), currentVersion.end());
            std::wstring latestVersionW(latestVersion.begin(), latestVersion.end());

            std::wstring message =
                L"A new version is available! Would you like to update now?\n\n"
                L"Current version: " + currentVersionW + L"\n" +
                L"Latest version: " + latestVersionW + L"\n\n" +
                L"Your settings will be preserved.";

            int result = MessageBoxW(hwnd, message.c_str(), L"Update Available", MB_YESNO | MB_ICONINFORMATION);
            if (result == IDYES)
            {
                OutputDebugStringA("[UPDATE] User chose to update - starting update process...\n");
                PerformUpdate();
            }
            else
            {
                OutputDebugStringA("[UPDATE] User chose to skip update\n");
            }
            return 0; // Important: return here to prevent click from passing through
        }
        if (showInputPanel && PtInRect(&inputPanelRectHit, pt))
        {
            int rowHeight = 36; // same as drawing
            int rowSpacing = 6;
            int yStart = (int)inputPanelRectUnified.Y + 10 + 28; // top padding + subtitle

            int index = (pt.y - yStart) / (rowHeight + rowSpacing);
            if (index >= 0 && index < (int)inputMap.size())
            {
                inputBeingSet = std::to_string(index);

                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    showVJoyPicker = true;
                    vJoyPickerInput = inputBeingSet;
                    InvalidateRect(hwnd, nullptr, TRUE);
                    UpdateWindow(hwnd);
                }
            }
        }

        // --- Keybind panel click ---
        if (showKeybindPanel &&
            mouseX >= panelRect.X && mouseX <= panelRect.X + panelRect.Width &&
            mouseY >= panelRect.Y && mouseY <= panelRect.Y + panelRect.Height)
        {
            // Match drawing offsets
            int rowHeight = 36;            // same as in draw
            int rowSpacing = 6;            // same as in draw
            int yStart = panelRect.Y + 70 + rightPanelScrollOffset; // matches drawing start

            // Prepare sorted key list same as in draw
            std::vector<std::string> sortedKeys;
            for (auto& kv : gearInputMap)
                sortedKeys.push_back(kv.first);

            auto isNumber = [](const std::string& s)
            {
                if (s.empty())
                    return false;
                for (char c : s)
                    if (!isdigit(c))
                        return false;
                return true;
            };

            std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const std::string& a, const std::string& b)
                {
                    bool aIsNum = isNumber(a);
                    bool bIsNum = isNumber(b);

                    if (aIsNum && bIsNum) return std::stoi(a) < std::stoi(b);
                    if (aIsNum) return true;
                    if (bIsNum) return false;
                    return a < b; });

            // Compute clicked row dynamically including spacing
            int index = (mouseY - yStart) / (rowHeight + rowSpacing);

            if (index >= 0 && index < (int)sortedKeys.size())
            {
                keybindBeingSet = sortedKeys[index];

                // Shift+Click → show vJoy picker
                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    showVJoyPicker = true;
                    vJoyPickerGear = keybindBeingSet;
                    InvalidateRect(hwnd, nullptr, TRUE);
                    UpdateWindow(hwnd);
                }
            }
        }

        // --- vJoy Picker click ---
        // --- vJoy Picker click ---
        if (showVJoyPicker && PtInRect(&g_vJoyPickerRect, pt))
        {
            for (int i = 0; i < g_vJoyButtonRects.size(); ++i)
            {
                if (PtInRect(&g_vJoyButtonRects[i], pt))
                {
                    char buf[256];

                    // --- Only update gearInputMap if vJoyPickerGear is set ---
                    if (!vJoyPickerGear.empty())
                    {
                        gearInputMap[vJoyPickerGear] = { VJOY_BUTTON, (WORD)(i + 1) };
                        sprintf_s(buf, "gearInputMap: Bound vJoy button %d to gear '%s'\n", i + 1, vJoyPickerGear.c_str());
                        OutputDebugStringA(buf);
                        vJoyPickerGear.clear(); // reset after use
                    }

                    // --- Only update inputMap secondary if vJoyPickerInput is set ---
                    if (!vJoyPickerInput.empty())
                    {
                        int idx = std::stoi(vJoyPickerInput);

                        // Update only secondary vJoy button, keep primary as-is
                        inputMap[idx].vjoyButton = i + 1;

                        sprintf_s(buf, "inputMap (secondary): Bound vJoy button %d to inputMap index %d\n", i + 1, idx);
                        OutputDebugStringA(buf);

                        vJoyPickerInput.clear(); // reset after use
                        inputBeingSet.clear();   // also reset inputBeingSet
                    }

                    showVJoyPicker = false;  // hide picker
                    keybindBeingSet.clear(); // reset old gear keybind
                    InvalidateRect(hwnd, nullptr, FALSE);
                    break;
                }
            }
        }

        else
        {
            // showVJoyPicker = false; // click outside picker closes it
        }

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
        }

        break;
