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
        hoveredProfileIndex = -1; // Reset hover state
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
                        break;
                    }
                }
            }
        }
        // Redraw if hover state changed
        bool needRedraw = (prevGearHover != hoveredGearLayoutIndex) ||
            (prevHShifterHover != hoveredHShifterLayoutIndex) ||
            (prevProfileHover != hoveredProfileIndex); // Add this comparison

        if (needRedraw) {
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
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

