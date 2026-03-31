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
                return 0;
            }

            // Minimize button functionality (for Maximize/Restore button)
            if (PtInRect(&g_MaxButtonRect, pt))
            {
                ShowWindow(hwnd, SW_MINIMIZE);
                return 0;
            }




            // Drag window by title bar (excluding buttons)
            if (pt.y >= 0 && pt.y < 30 && pt.x < g_MaxButtonRect.left)
            {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }
        }
        if (showUpdateModal)
        {
            if (PtInRect(&g_modalActionRect, pt))
            {
                showUpdateModal = false;
                PerformUpdate();
                return 0;
            }
            if (PtInRect(&g_modalCancelRect, pt))
            {
                showUpdateModal = false;
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            return 0; // Block clicks while modal is open
        }

        // --- Handle subtle update button click ---
        if (updateAvailable && !isBorderless && PtInRect(&updateSubtleBtnRect, pt))
        {
            showUpdateModal = true;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
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
            pt.x >= panelRect.X && pt.x <= panelRect.X + panelRect.Width &&
            pt.y >= panelRect.Y && pt.y <= panelRect.Y + panelRect.Height)
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
            int index = (pt.y - yStart) / (rowHeight + rowSpacing);

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