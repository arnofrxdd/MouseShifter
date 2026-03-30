        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };
        POINT itemAdjCursor = { mouseX, mouseY - settingsScrollOffset };

        // --- 1. Update Hover States for Right Side Panel ---
        std::string oldHoveredKeybindGear = hoveredKeybindGear;
        std::string oldHoveredResetGear = hoveredResetGear;
        int oldHoveredInputIndex = hoveredInputIndex;
        int oldHoveredInputResetIndex = hoveredInputResetIndex;
        int oldHoveredToggleIndex = hoveredToggleIndex;

        hoveredKeybindGear = "";
        hoveredResetGear = "";
        hoveredInputIndex = -1;
        hoveredInputResetIndex = -1;
        hoveredToggleIndex = -1;

        if (showKeybindPanel || showInputPanel || showTogglePanel)
        {
            if (pt.x >= (int)panelRect.X && pt.x <= (int)(panelRect.X + panelRect.Width))
            {
                // Reset buttons first (high priority hit)
                for (auto const& [gear, rect] : gearResetBtnRects) {
                    if (PtInRect(&rect, pt)) { hoveredResetGear = gear; break; }
                }
                for (auto const& [idx, rect] : inputResetBtnRects) {
                    if (PtInRect(&rect, pt)) { hoveredInputResetIndex = idx; break; }
                }

                // Toggle Panel Cards
                if (showTogglePanel) {
                    if (PtInRect(&g_toggleKeyRect, pt)) hoveredToggleIndex = 0;
                    else if (PtInRect(&g_assistButtonRect, pt)) hoveredToggleIndex = 1;
                    else if (PtInRect(&reverseUnlockKeyRect, pt)) hoveredToggleIndex = 2;
                }

                // Row hover if no button is hit
                if (hoveredResetGear.empty() && hoveredInputResetIndex == -1 && hoveredToggleIndex == -1)
                {
                    if (showKeybindPanel) {
                        int rowHeightS = 36; int rowSpacingS = 6;
                        int yStartS = panelRect.Y + 75 + rightPanelScrollOffset;
                        
                        std::vector<std::string> sortedKeys;
                        for (auto& kv : gearInputMap) sortedKeys.push_back(kv.first);
                        
                        auto isNum = [](const std::string& s) {
                            if (s.empty()) return false;
                            for (char c : s) if (!isdigit(c)) return false;
                            return true;
                        };
                        std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const std::string& a, const std::string& b) {
                            bool aIsN = isNum(a); bool bIsN = isNum(b);
                            if (aIsN && bIsN) return std::stoi(a) < std::stoi(b);
                            if (aIsN) return true;
                            if (bIsN) return false;
                            return a < b;
                        });

                        int index = (pt.y - yStartS) / (rowHeightS + rowSpacingS);
                        if (index >= 0 && index < (int)sortedKeys.size()) {
                            int rTop = yStartS + index * (rowHeightS + rowSpacingS);
                            if (pt.y >= rTop && pt.y < rTop + rowHeightS) hoveredKeybindGear = sortedKeys[index];
                        }
                    }

                    if (showInputPanel && hoveredKeybindGear.empty()) {
                        int rowHeightI = 36; int rowSpacingI = 6;
                        int yStartI = (int)inputPanelRectUnified.Y + 10 + 28;
                        int index = (pt.y - yStartI) / (rowHeightI + rowSpacingI);
                        if (index >= 0 && index < (int)inputMap.size()) {
                            int rTop = yStartI + index * (rowHeightI + rowSpacingI);
                            if (pt.y >= rTop && pt.y < rTop + rowHeightI) hoveredInputIndex = (int)index;
                        }
                    }
                }
            }
        }

        // --- 2. Redraw Trigger Condition ---
        bool hoverChanged = (oldHoveredKeybindGear != hoveredKeybindGear || 
                            oldHoveredResetGear != hoveredResetGear ||
                            oldHoveredInputIndex != hoveredInputIndex ||
                            oldHoveredInputResetIndex != hoveredInputResetIndex ||
                            oldHoveredToggleIndex != hoveredToggleIndex);

        bool inSettings = PtInRect(&settingsPanelRect, pt);
        bool inSidePanel = (pt.x >= (int)panelRect.X && pt.x <= (int)(panelRect.X + panelRect.Width));
        bool inNavBar = PtInRect(&topNavBarRect, pt);

        bool needRedraw = hoverChanged || inSettings || inSidePanel || inNavBar || 
                         showLayoutGallery || showVJoyPicker || creatingNewProfile || 
                         showResetConfirmation || gearLayoutDropdownOpen || profileDropdownOpen || 
                         hShifterLayoutDropdownOpen || mouseDeviceDropdownOpen || 
                         steeringDeviceDropdownOpen || processPickerModalOpen;

        if (needRedraw) {
            InvalidateRect(hwnd, NULL, FALSE);
        }

        // --- 3. Tooltip Display Logic ---
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen || showVJoyPicker || showLayoutGallery)
        {
            if (currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            g_showTooltip = false;
        }
        else
        {
            TooltipInfo* hoveredTooltip = nullptr;
            for (auto& tooltip : tooltips) {
                if (PtInRect(&tooltip.bounds, itemAdjCursor)) {  
                    hoveredTooltip = &tooltip;
                    break;
                }
            }
            if (hoveredTooltip && hoveredTooltip != currentTooltip) {
                if (currentTooltip) {
                    KillTimer(hwnd, currentTooltip->timerId);
                    currentTooltip->show = false;
                }
                currentTooltip = hoveredTooltip;
                currentTooltip->timerId = SetTimer(hwnd, 1001, 500, nullptr); 
            }
            else if (!hoveredTooltip && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Update tooltip bounds visibility
            g_showTooltip = (mouseX >= g_tooltipBounds.left && mouseX <= g_tooltipBounds.right &&
                mouseY >= g_tooltipBounds.top && mouseY <= g_tooltipBounds.bottom);
        }

