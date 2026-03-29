        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };
        POINT adjustedPt = { mouseX, mouseY - settingsScrollOffset };

        // --- DISABLE TOOLTIP WHEN DROPDOWNS ARE OPEN ---
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            // Cancel any active tooltip timers and hide current tooltip
            if (currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            g_showTooltip = false;

            // Don't break - continue to process dropdown hovers and sliders
        }
        else
        {
            // Only process tooltips when dropdowns are closed
            // Check hover for all tooltips
            TooltipInfo* hoveredTooltip = nullptr;

            for (auto& tooltip : tooltips) {
                if (PtInRect(&tooltip.bounds, adjustedPt)) {  // ← Use adjustedPt here
                    hoveredTooltip = &tooltip;
                    break;
                }
            }

            // Start timer for new hover, cancel for others
            if (hoveredTooltip && hoveredTooltip != currentTooltip) {
                // Cancel previous timer
                if (currentTooltip) {
                    KillTimer(hwnd, currentTooltip->timerId);
                    currentTooltip->show = false;
                }

                // Start new timer
                currentTooltip = hoveredTooltip;
                currentTooltip->timerId = SetTimer(hwnd, 1001, 500, nullptr); // 500ms delay
            }
            else if (!hoveredTooltip && currentTooltip) {
                // No hover, hide current tooltip
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Check if mouse is over the special tooltip (g_tooltipBounds)
            g_showTooltip = (mouseX >= g_tooltipBounds.left && mouseX <= g_tooltipBounds.right &&
                mouseY >= g_tooltipBounds.top && mouseY <= g_tooltipBounds.bottom);

            // Only redraw if tooltip state changed
            static bool lastTooltipState = false;
            if (g_showTooltip != lastTooltipState) {
                lastTooltipState = g_showTooltip;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }

        // Reset hover states (this should happen regardless of dropdown state)
        int prevGearHover = hoveredGearLayoutIndex;
        int prevHShifterHover = hoveredHShifterLayoutIndex;
        hoveredGearLayoutIndex = -1;
        hoveredHShifterLayoutIndex = -1;
        int prevProfileHover = hoveredProfileIndex; // Track previous hover state
        int prevCloneHover = hoveredCloneIndex;
        int prevDeleteHover = hoveredDeleteIndex;
        hoveredProfileIndex = -1; // Reset hover state
        hoveredCloneIndex = -1;
        hoveredDeleteIndex = -1;
        // Check for gear layout dropdown hover
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 30;
            int itemGap = 2;
            int listHeightPadding = 8;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size() + listHeightPadding;

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
                        dropdownRect.left + 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right - 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredGearLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }

        // Check for h-shifter layout dropdown hover
        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 30;
            int itemGap = 2;
            int listHeightPadding = 8;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size() + listHeightPadding;

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
                        dropdownRect.left + 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right - 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredHShifterLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }
        if (profileDropdownOpen)
        {
            int listItemHeight = 30;
            int itemGap = 2;
            int listHeightPadding = 8;
            int listY = profileButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size() + listHeightPadding;

            RECT dropdownRect = {
                profileButtonRect.left,
                listY,
                profileButtonRect.right,
                listY + totalHeight
            };

            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < profileNames.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left + 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right - 4,
                        listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredProfileIndex = (int)i;
                        
                        // Check icons
                        RECT cloneBtn = { itemRect.right - 50, itemRect.top, itemRect.right - 30, itemRect.bottom };
                        RECT deleteBtn = { itemRect.right - 25, itemRect.top, itemRect.right - 5, itemRect.bottom };

                        if (PtInRect(&cloneBtn, pt)) hoveredCloneIndex = (int)i;
                        else if (PtInRect(&deleteBtn, pt)) hoveredDeleteIndex = (int)i;
                        
                        break;
                    }
                }
            }
        }

        // --- Right Panel Hover Detection ---
        std::string prevHoverGear = hoveredKeybindGear;
        std::string prevResetHover = hoveredResetGear;
        int prevInputResetHover = hoveredInputResetIndex;
        int prevHoverInput = hoveredInputIndex;
        int prevHoverToggle = hoveredToggleIndex;
        
        hoveredKeybindGear = "";
        hoveredResetGear = "";
        hoveredInputResetIndex = -1;
        hoveredInputIndex = -1;
        hoveredToggleIndex = -1;
        
        if (showKeybindPanel)
        {
            RECT prHit = { (LONG)panelRect.X, (LONG)panelRect.Y, (LONG)(panelRect.X + panelRect.Width), (LONG)(panelRect.Y + panelRect.Height) };
            if (PtInRect(&prHit, pt))
            {
                int rowHeight = 36;
                int rowSpacing = 6;
                int yStart = panelRect.Y + 70 + rightPanelScrollOffset;

                std::vector<std::string> sortedKeys;
                for (auto& kv : gearInputMap) sortedKeys.push_back(kv.first);
                std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const std::string& a, const std::string& b) {
                    auto isNumber = [](const std::string& s) { if (s.empty()) return false; for (char c : s) if (!isdigit(c)) return false; return true; };
                    bool aIsNum = isNumber(a); bool bIsNum = isNumber(b);
                    if (aIsNum && bIsNum) return std::stoi(a) < std::stoi(b);
                    if (aIsNum) return true; if (bIsNum) return false; return a < b;
                });

                int index = (pt.y - yStart) / (rowHeight + rowSpacing);
                if (index >= 0 && index < (int)sortedKeys.size()) {
                    std::string gear = sortedKeys[index];
                    hoveredKeybindGear = gear;
                    
                    // Specific check for row reset button
                    if (gearResetBtnRects.count(gear) && PtInRect(&gearResetBtnRects[gear], pt)) {
                        hoveredResetGear = gear;
                    }
                }
            }
        }

        if (showInputPanel)
        {
            int rowHeight = 36;
            int rowSpacing = 6;
            int yStart = (int)inputPanelRectUnified.Y + 10 + 28;
            int index = (pt.y - yStart) / (rowHeight + rowSpacing);
            if (index >= 0 && index < (int)inputMap.size() && pt.x >= inputPanelRectUnified.X && pt.x <= inputPanelRectUnified.X + inputPanelRectUnified.Width)
            {
                hoveredInputIndex = index;
                if (inputResetBtnRects.count(index) && PtInRect(&inputResetBtnRects[index], pt)) {
                    hoveredInputResetIndex = index;
                }
            }
        }

        if (showTogglePanel)
        {
            if (PtInRect(&g_toggleKeyRect, pt)) hoveredToggleIndex = 0;
            else if (PtInRect(&g_assistButtonRect, pt)) hoveredToggleIndex = 1;
            else if (PtInRect(&reverseUnlockKeyRect, pt)) hoveredToggleIndex = 2;
        }

        // Redraw if hover state changed
        bool needRedraw = (prevGearHover != hoveredGearLayoutIndex) ||
            (prevHShifterHover != hoveredHShifterLayoutIndex) ||
            (prevProfileHover != hoveredProfileIndex) ||
            (prevCloneHover != hoveredCloneIndex) ||
            (prevDeleteHover != hoveredDeleteIndex) ||
            (prevHoverGear != hoveredKeybindGear) ||
            (prevResetHover != hoveredResetGear) ||
            (prevInputResetHover != hoveredInputResetIndex) ||
            (prevHoverInput != hoveredInputIndex) ||
            (prevHoverToggle != hoveredToggleIndex) ||
            PtInRect(&resetAllButtonRect, pt); // Also redraw for Reset All highlight

        if (creatingNewProfile || showVJoyPicker || showResetConfirmation) {
            // Modal overlays redraw on every move for responsive hover
            needRedraw = true; 
        }

        if (needRedraw) {
            InvalidateRect(hwnd, NULL, FALSE);
        }

        // --- Registry-based Dragging ---
        if (g_draggingElement)
        {
            int width = g_draggingElement->rect.right - g_draggingElement->rect.left;
            float t = float(mouseX - g_draggingElement->rect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            if (g_draggingElement->type == SettingType::SLIDER_INT)
            {
                int val = int(g_draggingElement->minVal + t * (g_draggingElement->maxVal - g_draggingElement->minVal));
                *(int*)g_draggingElement->valuePtr = val;
            }
            else if (g_draggingElement->type == SettingType::SLIDER_BYTE)
            {
                unsigned char val = (unsigned char)(g_draggingElement->minVal + t * (g_draggingElement->maxVal - g_draggingElement->minVal));
                *(unsigned char*)g_draggingElement->valuePtr = val;
            }
            else if (g_draggingElement->type == SettingType::SLIDER_FLOAT)
            {
                float val = g_draggingElement->minVal + t * (g_draggingElement->maxVal - g_draggingElement->minVal);
                *(float*)g_draggingElement->valuePtr = val;
            }

            // Specialized slider logic
            if (g_draggingElement->label == L"Gear Radius" || g_draggingElement->label == L"Snap Sensitivity") {
                gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
                ComputeIntersections();
            }
            if (g_draggingElement->label == L"Diagonal Assist Strength") {
                enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
                for (auto& inter : intersections) inter.radius = int(baseIntersectionRadius * diagonalAssist);
            }
            if (g_draggingElement->label == L"H-Shifter Size") {
                ComputeLayout(hwnd);
                ComputeIntersections();
            }

            InvalidateRect(hwnd, nullptr, FALSE);
        }

