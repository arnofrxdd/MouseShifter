                // --- CLUTCH KNOB GLOW (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (useScrollClutch && !realisticKnob) // ADD: && !realisticKnob
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastClutchKnobRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Check if knob needs redrawing due to clutch or vJoy changes
                        if (ShouldRedrawClutchKnob())
                        {
                            RECT clutchKnobRedrawArea = CalculateClutchKnobRedrawArea();
                            if (clutchKnobRedrawArea.right > clutchKnobRedrawArea.left)
                                InvalidateRect(hwnd, &clutchKnobRedrawArea, FALSE);

                            lastClutchKnobRedrawTime = currentTime; // Update timer only when actually redrawing
                        }
                    }
                }
                // --- RAIL COLOR CHANGES (CLUTCH DEPENDENT) ---
                if (useScrollClutch)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastRailRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Check if rails need redrawing due to clutch changes
                        if (ShouldRedrawRails())
                        {
                            RECT railRedrawArea = CalculateRailRedrawArea();
                            if (railRedrawArea.right > railRedrawArea.left)
                                InvalidateRect(hwnd, &railRedrawArea, FALSE);

                            lastRailRedrawTime = currentTime; // Update timer only when actually redrawing
                        }
                    }
                }
                // --- X-BAR MOVEMENT (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (showXBar && mouseSteeringEnabled)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastXBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
                        int drawRailCount = 0;
                        switch (layoutType)
                        {
                        case 1: // Normal Layout
                        case 3: // Reverse Bottom Layout (First rail)
                        case 4: // Reverse Bottom Layout (Last rail)
                        case 10: // Reverse Top Last Layout
                            drawRailCount = is16GearSet ? 5 : 4;
                            break;
                        case 2: // No Reverse Layout
                            drawRailCount = is16GearSet ? 4 : 3;
                            break;
                        case 5: // 5-Gear Only Layout
                        case 6: // 5-Gear Reverse First Layout
                        case 7: // 4-Gear Reverse Top Layout
                        case 8: // 4-Gear Reverse Bottom Layout
                        case 9: // 4-Gear Reverse Mixed Layout
                            drawRailCount = 3; // Always 3 rails for these layouts
                            break;
                        case 11: // PRNDL Layout
                            drawRailCount = 1; // Only one rail for PRNDL
                            break;
                        }

                        int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
                        int xBarWidth = static_cast<int>(fullWidth * 0.7f);
                        int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
                        int xBarYOffset = 30;
                        int currentXBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
                        int xBarLeft = xCenter - xBarWidth / 2;

                        float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
                        normalizedX = max(0.0f, min(1.0f, normalizedX));
                        int currentIndicatorX = xBarLeft + (int)(normalizedX * xBarWidth);

                        bool xBarPositionChanged = (currentXBarY != lastXBarY);
                        bool indicatorMoved = (currentIndicatorX != lastIndicatorX);

                        // If anything changed, redraw the combined area to prevent trails
                        if (xBarPositionChanged || indicatorMoved || xBarNeedsRedraw)
                        {
                            RECT xBarCompleteArea = CalculateXBarCompleteRedrawArea();
                            if (xBarCompleteArea.right > xBarCompleteArea.left)
                                InvalidateRect(hwnd, &xBarCompleteArea, FALSE);

                            lastXBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastXBarY = currentXBarY;
                        lastIndicatorX = currentIndicatorX;
                    }
                }


                // --- Y-BAR MOVEMENT if fixed transparency disabled ---
                if (showYBar && mouseSteeringEnabled && !useYbarFixedTransparency)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastYBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
                        int drawRailCount = 0;
                        switch (layoutType)
                        {
                        case 1: // Normal Layout
                        case 3: // Reverse Bottom Layout (First rail)
                        case 4: // Reverse Bottom Layout (Last rail)
                        case 10: // Reverse Top Last Layout
                            drawRailCount = is16GearSet ? 5 : 4;
                            break;
                        case 2: // No Reverse Layout
                            drawRailCount = is16GearSet ? 4 : 3;
                            break;
                        case 5: // 5-Gear Only Layout
                        case 6: // 5-Gear Reverse First Layout
                        case 7: // 4-Gear Reverse Top Layout
                        case 8: // 4-Gear Reverse Bottom Layout
                        case 9: // 4-Gear Reverse Mixed Layout
                            drawRailCount = 3; // Always 3 rails for these layouts
                            break;
                        case 11: // PRNDL Layout
                            drawRailCount = 1; // Only one rail for PRNDL
                            break;
                        }

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 20;
                        int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

                        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
                        normalizedY = max(0.0f, min(1.0f, normalizedY));
                        int currentFillHeight = (int)(barHeight * normalizedY);

                        bool yBarPositionChanged = (currentYBarX != lastYBarX);
                        bool fillHeightChanged = (currentFillHeight != lastYBarFillHeight);

                        // If anything changed, redraw the combined area to prevent trails
                        if (yBarPositionChanged || fillHeightChanged)
                        {
                            RECT yBarCompleteArea = CalculateYBarCompleteRedrawArea();
                            if (yBarCompleteArea.right > yBarCompleteArea.left)
                                InvalidateRect(hwnd, &yBarCompleteArea, FALSE);

                            lastYBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastYBarX = currentYBarX;
                        lastYBarFillHeight = currentFillHeight;
                    }
                }
                // if fixed transparency enabled
                if (showYBar && mouseSteeringEnabled && useYbarFixedTransparency)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastYBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
                        int drawRailCount = 0;
                        switch (layoutType)
                        {
                        case 1: // Normal Layout
                        case 3: // Reverse Bottom Layout (First rail)
                        case 4: // Reverse Bottom Layout (Last rail)
                        case 10: // Reverse Top Last Layout
                            drawRailCount = is16GearSet ? 5 : 4;
                            break;
                        case 2: // No Reverse Layout
                            drawRailCount = is16GearSet ? 4 : 3;
                            break;
                        case 5: // 5-Gear Only Layout
                        case 6: // 5-Gear Reverse First Layout
                        case 7: // 4-Gear Reverse Top Layout
                        case 8: // 4-Gear Reverse Bottom Layout
                        case 9: // 4-Gear Reverse Mixed Layout
                            drawRailCount = 3; // Always 3 rails for these layouts
                            break;
                        case 11: // PRNDL Layout
                            drawRailCount = 1; // Only one rail for PRNDL
                            break;
                        }

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 20;
                        int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

                        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
                        normalizedY = max(0.0f, min(1.0f, normalizedY));
                        int currentFillHeight = (int)(barHeight * normalizedY);

                        bool yBarPositionChanged = (currentYBarX != lastYBarX);
                        bool fillHeightChanged = (currentFillHeight != lastYBarFillHeight);

                        // If anything changed, redraw the Y-bar window
                        if (yBarPositionChanged || fillHeightChanged || joyYChanged || showYBarChanged)
                        {
                            if (isBorderless && g_yBarHwnd)
                            {
                                // REDRAW BOTH CURRENT AND PREVIOUS AREAS TO PREVENT TRAILING
                                RECT currentYBarArea = CalculateYBarRedrawArea();
                                RECT previousYBarArea;

                                // Calculate previous area based on last position
                                previousYBarArea.left = lastYBarX - 5;
                                previousYBarArea.top = centerY - barHeight / 2 - 5;
                                previousYBarArea.right = lastYBarX + barWidth + 5;
                                previousYBarArea.bottom = centerY + barHeight / 2 + 5;

                                // Combine both areas
                                RECT combinedArea;
                                combinedArea.left = min(currentYBarArea.left, previousYBarArea.left);
                                combinedArea.top = min(currentYBarArea.top, previousYBarArea.top);
                                combinedArea.right = max(currentYBarArea.right, previousYBarArea.right);
                                combinedArea.bottom = max(currentYBarArea.bottom, previousYBarArea.bottom);

                                // Redraw the combined area
                                InvalidateRect(g_yBarHwnd, &combinedArea, FALSE);
                            }
                            else
                            {
                                // Fallback: redraw in main window (windowed mode)
                                RECT yBarCompleteArea = CalculateYBarCompleteRedrawArea();
                                if (yBarCompleteArea.right > yBarCompleteArea.left)
                                    InvalidateRect(hwnd, &yBarCompleteArea, FALSE);
                            }

                            lastYBarRedrawTime = currentTime;
                        }

                        // Update tracking variables
                        lastYBarX = currentYBarX;
                        lastYBarFillHeight = currentFillHeight;
                    }
                }
                // --- CLUTCH BAR MOVEMENT (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (showClutchIndicator && useScrollClutch)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastClutchBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
                        int drawRailCount = 0;
                        switch (layoutType)
                        {
                        case 1: // Normal Layout
                        case 3: // Reverse Bottom Layout (First rail)
                        case 4: // Reverse Bottom Layout (Last rail)
                        case 10: // Reverse Top Last Layout
                            drawRailCount = is16GearSet ? 5 : 4;
                            break;
                        case 2: // No Reverse Layout
                            drawRailCount = is16GearSet ? 4 : 3;
                            break;
                        case 5: // 5-Gear Only Layout
                        case 6: // 5-Gear Reverse First Layout
                        case 7: // 4-Gear Reverse Top Layout
                        case 8: // 4-Gear Reverse Bottom Layout
                        case 9: // 4-Gear Reverse Mixed Layout
                            drawRailCount = 3; // Always 3 rails for these layouts
                            break;
                        case 11: // PRNDL Layout
                            drawRailCount = 1; // Only one rail for PRNDL
                            break;
                        }

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 40;
                        int currentClutchBarX = lastRailX + 50;
                        currentClutchBarX = max(currentClutchBarX, knobPos.x + knobRadius + xBarGap);
                        if (showYBar && mouseSteeringEnabled)
                            currentClutchBarX = max(currentClutchBarX, lastRailX + 50 + xBarGap);

                        // Calculate current fill position
                        float normalizedRx;
                        if (useHalfClutch)
                        {
                            LONG mid = (axisMin + axisMax) / 2;
                            normalizedRx = 1.0f - ((float)(joyRx - mid) / (float)(axisMax - mid));
                        }
                        else
                        {
                            normalizedRx = 1.0f - ((float)(joyRx - axisMin) / (float)(axisMax - axisMin));
                        }
                        normalizedRx = max(0.0f, min(1.0f, normalizedRx));
                        int currentFillHeight = (int)(barHeight * normalizedRx);

                        bool clutchBarPositionChanged = (currentClutchBarX != lastClutchBarX);
                        bool fillHeightChanged = (currentFillHeight != lastClutchBarFillHeight);

                        // If anything changed, redraw the combined area to prevent trails
                        if (clutchBarPositionChanged || fillHeightChanged)
                        {
                            RECT clutchBarCompleteArea = CalculateClutchBarCompleteRedrawArea();
                            if (clutchBarCompleteArea.right > clutchBarCompleteArea.left)
                                InvalidateRect(hwnd, &clutchBarCompleteArea, FALSE);

                            lastClutchBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastClutchBarX = currentClutchBarX;
                        lastClutchBarFillHeight = currentFillHeight;
                    }
                }
            }