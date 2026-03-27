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
                        hoveredGearLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }

        // Check for h-shifter layout dropdown hover
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
                        hoveredHShifterLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }
        if (profileDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = profileButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size();

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
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
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

        // ----- Knob Radius Slider -----
        if (draggingKnobSlider)
        {
            int width = knobSliderRect.right - knobSliderRect.left;
            float t = float(mouseX - knobSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            knobRadius = knobRadiusMin + int(t * (knobRadiusMax - knobRadiusMin));
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Gear Radius Slider -----
        if (isDraggingGearRadius)
        {
            int width = gearSliderRect.right - gearSliderRect.left;
            float t = float(mouseX - gearSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            gearRadius = gearRadiusMin + int(t * (gearRadiusMax - gearRadiusMin));

            // ✅ Add this line here:
            gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
            ComputeIntersections();

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // Max slider (0..255 range independent of minAlpha)
        if (draggingTransparencySlider)
        {
            int width = transparencySliderRect.right - transparencySliderRect.left;
            float t = float(mouseX - transparencySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            maxAlpha = BYTE(t * 255.0f);
            maxAlpha = max(maxAlpha, minAlpha + 1); // enforce > min
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (draggingMinTransparencySlider)
        {
            int width = minTransparencySliderRect.right - minTransparencySliderRect.left;
            float t = float(mouseX - minTransparencySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            minAlpha = BYTE(t * 255.0f);
            minAlpha = min(minAlpha, maxAlpha - 1); // enforce < max
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingTransparencyFadeDelaySlider)
        {
            int width = transparencyFadeDelaySliderRect.right - transparencyFadeDelaySliderRect.left;
            float t = float(mouseX - transparencyFadeDelaySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Round to nearest 10 for +10 increments
            transparencyFadeDelay = DWORD(round(t * 2000.0f / 10.0f) * 10.0f); // range: 0–2000 ms in +10 increments
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // Add this with your other slider update handlers:
        if (draggingAxisSmoothingFactorSlider)
        {
            int width = axisSmoothingFactorSlider.right - axisSmoothingFactorSlider.left;
            float t = float(mouseX - axisSmoothingFactorSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            axisSmoothingFactor = t; // 0.0 to 1.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Sensitivity Slider -----
        if (draggingSensSlider)
        {
            int width = sensSliderRect.right - sensSliderRect.left;
            float t = float(mouseX - sensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            knobSensitivity = knobSensitivityMin + t * (knobSensitivityMax - knobSensitivityMin);
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Diagonal Assist Slider -----
        if (draggingDiagSlider)
        {
            int width = diagSliderRect.right - diagSliderRect.left;
            float t = float(mouseX - diagSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            diagonalAssist = diagMin + t * (diagMax - diagMin);

            // Recompute thresholds
            enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
            for (auto& inter : intersections)
            {
                inter.radius = int(baseIntersectionRadius * diagonalAssist);
            }

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Snap-In Threshold Slider -----
        if (draggingSnapInSlider)
        {
            int width = snapInSliderRect.right - snapInSliderRect.left;
            float t = float(mouseX - snapInSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            gearSnapInMultiplier = snapInMin + t * (snapInMax - snapInMin);
            gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier); // update actual threshold
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSteeringSlider)
        {
            int width = steeringSensSliderRect.right - steeringSensSliderRect.left;
            float t = float(mouseX - steeringSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            steeringSensitivity = 0.1f + t * 4.9f; // example: 0.1 – 5.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSteeringDegreesSlider)
        {
            int width = steeringDegreesSliderRect.right - steeringDegreesSliderRect.left;
            float t = float(mouseX - steeringDegreesSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Map t (0–1) to 90–900°
            float rawDegrees = 90.0f + t * (900.0f - 90.0f);

            // --- Quantize to 10° steps ---
            maxSteeringDegrees = round(rawDegrees / 10.0f) * 10.0f;

            InvalidateRect(hwnd, nullptr, FALSE);
        }


        // Acc/Brake Sensitivity
        if (draggingAccBrakeSlider)
        {
            int width = accBrakeSensSliderRect.right - accBrakeSensSliderRect.left;
            float t = float(mouseX - accBrakeSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            accBrakeSensitivity = 0.1f + t * 19.9f; // 0.1 – 20.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingScrollSensSlider)
        {
            int width = scrollSensSliderRect.right - scrollSensSliderRect.left;
            float t = float(mouseX - scrollSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            scrollClutchSens = t * 10.0f; // 0–10
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // ----- Snap Speed Slider -----
        if (draggingSnapSpeedSlider)
        {
            int width = snapSpeedSliderRect.right - snapSpeedSliderRect.left;
            float t = float(mouseX - snapSpeedSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            snapSpeed = snapSpeedMin + t * (snapSpeedMax - snapSpeedMin);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSmoothScrollSlider)
        {
            int width = smoothScrollSlider.right - smoothScrollSlider.left;
            float t = float(mouseX - smoothScrollSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            smoothScrollSpeed = 1.0f + t * 19.0f; // 1.0-20.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingBrakeResistanceSlider)
        {
            int width = brakeresistanceSlider.right - brakeresistanceSlider.left;
            float t = float(mouseX - brakeresistanceSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            brakeresistanceFactor = t * 50.0f; // 0-50 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (draggingAccelerationResistanceSlider)
        {
            int width = accelerationresistanceSlider.right - accelerationresistanceSlider.left;
            float t = float(mouseX - accelerationresistanceSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            accelerationResistanceFactor = t * 50.0f; // 0-50 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (controllerDraggingSensSlider)
        {
            int width = controllerSensSliderRect.right - controllerSensSliderRect.left;
            float t = float(mouseX - controllerSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            controllerSensSliderValue = t;
            controllerSensMultiplier = 0.1f + t * 2.0f; // map 0-1 -> 0.1x to 2x
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingYBarAlphaSlider)
        {
            int width = yBarAlphaSlider.right - yBarAlphaSlider.left;
            float t = float(mouseX - yBarAlphaSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Convert percentage (0-100) back to 0-255 range
            int percentage = (int)(t * 100.0f);
            yBarAlpha = (int)((percentage / 100.0f) * 255.0f);

            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // ----- Layout Scale Slider -----
        if (draggingLayoutSlider)
        {
            int width = layoutScaleSliderRect.right - layoutScaleSliderRect.left;
            float t = float(mouseX - layoutScaleSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            layoutScale = 1.0f + t * (3.0f - 1.0f); // min: 1.0, max: 3.0
            ComputeLayout(hwnd);                    // recompute rails & positions

            ComputeIntersections();

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        break;
