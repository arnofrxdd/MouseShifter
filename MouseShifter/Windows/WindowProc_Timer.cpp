        // ADD THESE TWO FLAGS AT THE VERY TOP
        if (!isBorderless) {
            if (wParam == 1001 && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = true;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            if ((gearLayoutDropdownOpen || hShifterLayoutDropdownOpen) && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }
        UpdateKnobMovement(hwnd);
        UpdateAutoInjection();
        UpdateSmoothScroll();
        if (knobDisabledByF9)
        {
            knobMovementEnabled = IsKnobToggleActive();
        }

        // Update controller positions
        static POINT lastControllerKnobPos = knobPos;
        static POINT lastControllerGhostKnobPos = ghostKnobPos;

        POINT prevKnobPos = knobPos;
        POINT prevGhostKnobPos = ghostKnobPos;
        UpdateKnobFromXInput(hwnd);
        UpdateVJoyFromXInput();

        bool controllerKnobMoved = (knobPos.x != prevKnobPos.x || knobPos.y != prevKnobPos.y);
        bool controllerGhostMoved = (ghostKnobPos.x != prevGhostKnobPos.x || ghostKnobPos.y != prevGhostKnobPos.y);

        PollDirectInput();
        PollPedalBinding(hwnd);

        // === UPDATE KEYBIND GLOW ANIMATIONS (ALWAYS PROCESS) ===
        bool anyGlowAnimationChanged = false;
        DWORD currentTime = GetTickCount();

        for (auto& kv : keybindAnimations) {
            auto& animation = kv.second;
            float oldAlpha = animation.glowAlpha;

            if (animation.isHeld) {
                // Key is held down - immediately go to full brightness and stay there
                animation.glowAlpha = MAX_GLOW_ALPHA;
                animation.isActive = false;
            }
            else {
                // Key is not held - handle fade in/out
                if (animation.isActive) {
                    // Fade in
                    float elapsed = (float)(currentTime - animation.activationTime);
                    animation.glowAlpha = min(MAX_GLOW_ALPHA, elapsed / GLOW_FADE_IN_TIME);

                    if (animation.glowAlpha >= MAX_GLOW_ALPHA) {
                        animation.glowAlpha = MAX_GLOW_ALPHA;
                        animation.isActive = false;
                        animation.activationTime = currentTime;
                    }
                }
                else {
                    // Fade out
                    float elapsed = (float)(currentTime - animation.activationTime);
                    if (elapsed < GLOW_FADE_OUT_TIME) {
                        animation.glowAlpha = max(0.0f, MAX_GLOW_ALPHA - (elapsed / GLOW_FADE_OUT_TIME));
                    }
                    else {
                        animation.glowAlpha = 0.0f;
                    }
                }
            }

            if (abs(animation.glowAlpha - oldAlpha) > 0.01f) {
                anyGlowAnimationChanged = true;
            }
        }

        // Clean up completed animations
        std::vector<std::string> toRemove;
        for (auto& kv : keybindAnimations) {
            if (!kv.second.isHeld && !kv.second.isActive && kv.second.glowAlpha <= 0.0f) {
                toRemove.push_back(kv.first);
            }
        }
        for (auto& key : toRemove) {
            keybindAnimations.erase(key);
        }

        // --- TRACK ALL VISUAL STATES FOR SMART REDRAW ---
        static float lastKnobFlash = knobFlash;
        static bool lastGreyOutState = !knobMovementEnabled;
        static bool lastKnobMovementEnabled = knobMovementEnabled;
        static float lastClutchNorm = 0.0f;
        static bool lastVJoyMouseEnabled = vJoyMouseEnabled;
        static std::string lastActiveGear = activeGear;

        // Ghost knob tracking
        static POINT lastGhostKnobPos = ghostKnobPos;
        static std::string lastGhostSnappedGear = ghostSnappedGear;
        static bool lastUseAssistPointer = useAssistPointer;

        // Indicator bar tracking
        static bool lastMouseSteeringEnabled = mouseSteeringEnabled;
        static bool lastShowYBar = showYBar;
        static bool lastShowXBar = showXBar;
        static bool lastShowClutchIndicator = showClutchIndicator;
        static int lastJoyY = joyY;
        static int lastJoyX = joyX;
        static int lastJoyRx = joyRx;
        static bool lastAssistButtonState = false;
        static bool assistButtonChanged = false;
        // X-bar specific tracking
        static POINT lastKnobPosForXBar = knobPos;
        static int lastDrawRailCount = 0;

        // Calculate current rail count for X-bar positioning
        int currentDrawRailCount = 0;
        switch (layoutType)
        {
        case 1: // Normal Layout
        case 3: // Reverse Bottom Layout (First rail)
        case 4: // Reverse Bottom Layout (Last rail)
        case 10: // Reverse Top Last Layout
            currentDrawRailCount = is16GearSet ? 5 : 4;
            break;
        case 2: // No Reverse Layout
            currentDrawRailCount = is16GearSet ? 4 : 3;
            break;
        case 5: // 5-Gear Only Layout
        case 6: // 5-Gear Reverse First Layout
        case 7: // 4-Gear Reverse Top Layout
        case 8: // 4-Gear Reverse Bottom Layout
        case 9: // 4-Gear Reverse Mixed Layout
            currentDrawRailCount = 3; // Always 3 rails for these layouts
            break;
        case 11: // PRNDL Layout
            currentDrawRailCount = 1; // Only one rail for PRNDL
            break;
        }

        // Calculate current clutch normalization
        float currentClutchNorm = 0.0f;
        if (useScrollClutch)
        {
            if (useHalfClutch)
            {
                LONG mid = (axisMin + axisMax) / 2;
                currentClutchNorm = (float)(joyRx - mid) / (float)(axisMax - mid);
            }
            else
            {
                currentClutchNorm = (float)(joyRx - axisMin) / (float)(axisMax - axisMin);
            }
            currentClutchNorm = max(0.0f, min(1.0f, currentClutchNorm));
        }

        // Update flash state
        bool anyPressed = false;
        for (auto& kv : buttonPressedState)
        {
            if (kv.second) {
                anyPressed = true;
                break;
            }
        }

        if (anyPressed)
            knobFlash = 0.5f;
        else
            knobFlash = max(0.0f, knobFlash - knobFlashFade);

        // --- CHECK ALL VISUAL CHANGES THAT REQUIRE REDRAW ---
        bool flashChanged = (abs(knobFlash - lastKnobFlash) > 0.01f);
        bool greyOutChanged = (!knobMovementEnabled != lastGreyOutState);
        bool knobEnabledChanged = (knobMovementEnabled != lastKnobMovementEnabled);
        bool clutchEffectChanged = (abs(currentClutchNorm - lastClutchNorm) > 0.01f);
        bool vJoyStateChanged = (vJoyMouseEnabled != lastVJoyMouseEnabled);
        bool activeGearChanged = (activeGear != lastActiveGear);
        static POINT lastBorderlessKnobPos = knobPos;
        bool borderlessKnobMoved = (knobPos.x != lastBorderlessKnobPos.x || knobPos.y != lastBorderlessKnobPos.y);
        lastBorderlessKnobPos = knobPos;
        // Ghost knob states
        bool ghostMoved = (ghostKnobPos.x != lastGhostKnobPos.x || ghostKnobPos.y != lastGhostKnobPos.y);
        bool ghostGearChanged = (ghostSnappedGear != lastGhostSnappedGear);
        bool ghostVisibilityChanged = (useAssistPointer != lastUseAssistPointer);

        // Indicator bar states
        bool mouseSteeringEnabledChanged = (mouseSteeringEnabled != lastMouseSteeringEnabled);
        bool showYBarChanged = (showYBar != lastShowYBar);
        bool showXBarChanged = (showXBar != lastShowXBar);
        bool showClutchIndicatorChanged = (showClutchIndicator != lastShowClutchIndicator);

        // Input value changes for indicator bars
        bool joyYChanged = (joyY != lastJoyY);
        bool joyXChanged = (joyX != lastJoyX);
        bool joyRxChanged = (joyRx != lastJoyRx);

        // X-bar specific redraw check
        bool xBarNeedsRedraw = joyXChanged || showXBarChanged || mouseSteeringEnabledChanged ||
            (knobPos.x != lastKnobPosForXBar.x || knobPos.y != lastKnobPosForXBar.y) ||
            (currentDrawRailCount != lastDrawRailCount);

        // Check if any visual state changed enough to warrant redraw
        bool needsFullRedraw = flashChanged || greyOutChanged || knobEnabledChanged ||
            clutchEffectChanged || vJoyStateChanged || activeGearChanged ||
            ghostMoved || ghostGearChanged || ghostVisibilityChanged ||
            mouseSteeringEnabledChanged || showYBarChanged || showClutchIndicatorChanged ||
            joyYChanged || joyRxChanged || controllerKnobMoved ||
            controllerGhostMoved || anyGlowAnimationChanged;

        // === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
// === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
        // In your WM_TIMER case, replace the borderless section with this:

        if (isBorderless)
        {
            if (disableSmartRedraws)
            {
                // SIMPLE CONSTANT REDRAW - NO SMART LOGIC
                InvalidateRect(hwnd, nullptr, FALSE);

            }
            else
            {
                // YOUR EXISTING SMART REDRAW LOGIC GOES HERE (everything below)
                if (knobMovementEnabled)
                {
                    // --- KNOB MOVEMENT ENABLED ---
            // ADD THIS FLAG
                    bool disableSmartFPS = true;

                    DWORD currentTime = GetTickCount();
                    float moveDistance = CalculateKnobMovementIntensity();

                    // SYNC ALL SYSTEMS TO SAME FPS TIMING (EXCEPT FLASH)
                    bool shouldRedrawThisFrame = false;
                    int syncFPS = currentRedrawFPS;

                    // 1. FLASH EFFECT - ALWAYS UNLIMITED FPS, OUTSIDE SMART SYSTEM
                    if (flashChanged && knobFlash > 0.0f && !realisticKnob) // ADD: && !realisticKnob
                    {
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }
                    }
                    if (vJoyStateChanged)
                    {
                        // When vJoy state changes, redraw knob immediately for visual feedback
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        // Also redraw any active gear text on the knob
                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }



                        // Update tracking immediately
                        lastVJoyMouseEnabled = vJoyMouseEnabled;
                    }
                    // 2. GHOST KNOB MOVEMENT - SEPARATE SMART FPS SYSTEM
                    // 2. GHOST KNOB MOVEMENT - SEPARATE SMART FPS SYSTEM
                    if ((ghostMoved || ghostGearChanged) && useAssistPointer)
                    {
                        // === UNLIMITED FPS - NO THROTTLING ===
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);

                        // Optional: Still track for debugging if needed
                        ghostKnobMoveDistance += 1.0f;
                        lastGhostKnobMoveTime = GetTickCount();
                    }

                    // 3. REAL KNOB MOVEMENT FPS control (your existing code)
                     // 3. REAL KNOB MOVEMENT FPS control (your existing code)
                    if (moveDistance > 0.1f)
                    {
                        knobMoveDistance += moveDistance;

                        // MODIFY THIS PART
                        if (disableSmartFPS)
                        {
                            shouldRedrawThisFrame = true;
                            syncFPS = 0;
                        }
                        else
                        {
                            if (knobMoveDistance < 1.0f) syncFPS = 5;
                            else if (knobMoveDistance < 3.0f) syncFPS = 10;
                            else syncFPS = 0;

                            if (syncFPS > 0)
                            {
                                DWORD minRedrawInterval = 1000 / syncFPS;
                                shouldRedrawThisFrame = (currentTime - lastKnobMoveTime) >= minRedrawInterval;
                            }
                            else
                            {
                                shouldRedrawThisFrame = true;
                            }
                        }

                        if (shouldRedrawThisFrame)
                        {
                            knobMoveDistance = 0.0f;
                            lastKnobMoveTime = currentTime;

                            RECT knobRedrawRect = CalculateKnobRedrawArea();
                            InvalidateRect(hwnd, &knobRedrawRect, FALSE);
                        }

                        lastKnobPos = knobPos;
                    }
                    // Update assist button tracking
                    assistButtonChanged = (assistButtonHeld != lastAssistButtonState);
                    lastAssistButtonState = assistButtonHeld;
                    if (assistButtonChanged && !assistButtonHeld)
                    {
                        // Assist button was just released - hide ghost knob immediately
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);
                        char debugMsg[256];
                        sprintf_s(debugMsg, "ASSIST RELEASED 1: assistButtonHeld=%d, useAssistPointer=%d",
                            assistButtonHeld, useAssistPointer);
                        OutputDebugStringA(debugMsg);
                    }
                    // 4. Other events use the SAME FPS timing (EXCLUDING FLASH AND GHOST)
                    bool hasCriticalEvent = activeGearChanged || greyOutChanged || knobEnabledChanged || assistButtonChanged;
                    if (hasCriticalEvent)
                    {
                        // Gear changes - keep FPS throttling
                        if (activeGearChanged && shouldRedrawThisFrame)
                        {
                            if (!lastActiveGearState.empty() && lastActiveGearState != "N")
                            {
                                RECT oldGearRect = CalculateSingleGearRedrawArea(lastActiveGearState);
                                InvalidateRect(hwnd, &oldGearRect, FALSE);
                            }

                            if (!activeGear.empty() && activeGear != "N")
                            {
                                RECT newGearRect = CalculateSingleGearRedrawArea(activeGear);
                                InvalidateRect(hwnd, &newGearRect, FALSE);
                            }

                            lastActiveGearState = activeGear;
                        }

                        // Greyout changes - IMMEDIATE REDRAW (no FPS throttling)
                        if (greyOutChanged || knobEnabledChanged)
                        {
                            RECT allGearsRect = CalculateAllGearsRedrawArea();
                            InvalidateRect(hwnd, &allGearsRect, FALSE);
                            lastKnobMovementEnabledState = knobMovementEnabled;
                        }
                    }
                }
                else
                {
                    // --- KNOB MOVEMENT DISABLED ---
                    // But ghost knob might still be active!

                    // GHOST KNOB MOVEMENT WHEN DISABLED - SEPARATE SMART FPS SYSTEM
                    // GHOST KNOB MOVEMENT WHEN DISABLED - SEPARATE SMART FPS SYSTEM
                    assistButtonChanged = (assistButtonHeld != lastAssistButtonState);
                    lastAssistButtonState = assistButtonHeld;
                    if (vJoyStateChanged)
                    {
                        // When vJoy state changes, redraw knob immediately for visual feedback
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        // Also redraw any active gear text on the knob
                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }

                        // Update tracking immediately
                        lastVJoyMouseEnabled = vJoyMouseEnabled;
                    }

                    // GHOST KNOB MOVEMENT WHEN DISABLED - UNLIMITED FPS
                    if ((ghostMoved || ghostGearChanged) && useAssistPointer)
                    {
                        // === UNLIMITED FPS - NO THROTTLING ===
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);

                        // Optional: Still track for debugging if needed
                        ghostKnobMoveDistance += 1.0f;
                        lastGhostKnobMoveTime = GetTickCount();
                    }

                    if (assistButtonChanged && !assistButtonHeld)
                    {
                        // Assist button was just released - hide ghost knob immediately
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);
                        char debugMsg[256];
                        sprintf_s(debugMsg, "ASSIST RELEASED: assistButtonHeld=%d, useAssistPointer=%d",
                            assistButtonHeld, useAssistPointer);
                        OutputDebugStringA(debugMsg);
                    }

                    // 1. Handle gear greying out when knob gets disabled
                    if (greyOutChanged || knobEnabledChanged)
                    {
                        // Redraw ALL gears when knob gets disabled/enabled
                        RECT allGearsRect = CalculateAllGearsRedrawArea();
                        if (allGearsRect.right > allGearsRect.left) // Valid rect
                            InvalidateRect(hwnd, &allGearsRect, FALSE);

                        lastKnobMovementEnabledState = knobMovementEnabled;
                    }


                }
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
        }
        else
        {
            // WINDOWED MODE: ALWAYS FULL REDRAW
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // Update all tracked states
        lastKnobFlash = knobFlash;
        lastGreyOutState = !knobMovementEnabled;
        lastKnobMovementEnabled = knobMovementEnabled;
        lastClutchNorm = currentClutchNorm;
        lastVJoyMouseEnabled = vJoyMouseEnabled;
        lastActiveGear = activeGear;

        lastGhostKnobPos = ghostKnobPos;
        lastGhostSnappedGear = ghostSnappedGear;
        lastUseAssistPointer = useAssistPointer;

        lastMouseSteeringEnabled = mouseSteeringEnabled;
        lastShowYBar = showYBar;
        lastShowXBar = showXBar;
        lastShowClutchIndicator = showClutchIndicator;
        lastJoyY = joyY;
        lastJoyX = joyX;
        lastJoyRx = joyRx;

        lastKnobPosForXBar = knobPos;
        lastDrawRailCount = currentDrawRailCount;

        // Dynamic transparency (only for borderless mode)
// Dynamic transparency (only for borderless mode)
        if (isBorderless && !useXInput && isTransparent && dynamicTransparencyEnabled)
        {
            static DWORD lastActiveTime = 0;
            static POINT lastKnobPosTrans = knobPos;
            static DWORD lastAlphaUpdate = 0;
            static DWORD lastBrightnessCheck = 0;
            static BYTE dynamicMinAlpha = minAlpha; // Separate variable for dynamic adjustment
            static BYTE lastAppliedMinAlpha = minAlpha; // Track what's currently applied to window
            static BYTE lastUserMinAlpha = minAlpha; // Track user's slider value

            DWORD now = GetTickCount();

            // Update if user changed the slider
            if (lastUserMinAlpha != minAlpha) {
                lastUserMinAlpha = minAlpha;
                dynamicMinAlpha = minAlpha; // Reset to user's value
                lastAppliedMinAlpha = minAlpha;
                lastBrightnessCheck = 0; // Force recalc
                lastAlphaUpdate = 0; // Force redraw
            }

            // Check background brightness at 2 FPS (every 500ms) - OPTIMIZED
// Check background brightness at 2 FPS (every 500ms) - OPTIMIZED
            if (now - lastBrightnessCheck > 1500)
            {
                float brightness = GetHShifterBackgroundBrightnessDebug(hwnd);

                // Linear mapping: brightness 0-255 maps to brightnessFactor 0.2-1.0
                float brightnessFactor = 0.2f + (brightness / 255.0f) * 0.8f;
                brightnessFactor = max(0.2f, min(1.0f, brightnessFactor));

                // Calculate new dynamic minAlpha - will always be between 20% and 100% of minAlpha
                BYTE newDynamicMinAlpha = (BYTE)(minAlpha * brightnessFactor);
                newDynamicMinAlpha = max(10, min(150, newDynamicMinAlpha));

                // Add to history
                minAlphaHistory.push_back(newDynamicMinAlpha);
                if (minAlphaHistory.size() > HISTORY_SIZE)
                    minAlphaHistory.erase(minAlphaHistory.begin());

                // Check if we should update (only if significant change or history is stable)
                bool shouldUpdate = false;

                if (minAlphaHistory.size() == HISTORY_SIZE)
                {
                    // Calculate average of recent values
                    int sum = 0;
                    for (BYTE val : minAlphaHistory) sum += val;
                    BYTE recentAverage = sum / minAlphaHistory.size();

                    // Check if the new value is significantly different from recent average
                    if (abs((int)newDynamicMinAlpha - (int)recentAverage) >= CHANGE_THRESHOLD)
                    {
                        shouldUpdate = true;
                    }
                    // Also check if all recent values are similar (stable state)
                    else
                    {
                        bool allSimilar = true;
                        for (BYTE val : minAlphaHistory)
                        {
                            if (abs((int)val - (int)newDynamicMinAlpha) > (CHANGE_THRESHOLD / 2))
                            {
                                allSimilar = false;
                                break;
                            }
                        }
                        // If values are stable and different from current dynamicMinAlpha, update
                        if (allSimilar && abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD)
                        {
                            shouldUpdate = true;
                        }
                    }
                }
                else
                {
                    // Not enough history yet, use simple threshold
                    shouldUpdate = (abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD);
                }

                if (shouldUpdate)
                {
                    dynamicMinAlpha = newDynamicMinAlpha;

                    char alphaMsg[256];
                    sprintf_s(alphaMsg, "DYNAMIC ALPHA: Brightness=%.1f, UserMinAlpha=%d, DynamicMinAlpha=%d (Factor=%.1f)\n",
                        brightness, minAlpha, dynamicMinAlpha, brightnessFactor);
                    OutputDebugStringA(alphaMsg);

                    // Force alpha update on next frame since dynamicMinAlpha changed
                    lastAlphaUpdate = 0;
                }

                lastBrightnessCheck = now;
            }

            bool shiftHeld = IsKnobToggleActive();
            float deadzone = 2.0f;

            bool knobMoved = (abs(knobPos.x - lastKnobPosTrans.x) > deadzone) ||
                (abs(knobPos.y - lastKnobPosTrans.y) > deadzone);

            if (knobMoved || shiftHeld)
            {
                currentAlpha = maxAlpha;
                lastActiveTime = now;
                SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                lastAppliedMinAlpha = dynamicMinAlpha; // Track what's applied
            }
            else
            {
                if (now - lastActiveTime >= transparencyFadeDelay)
                {
                    // Limit to 15 FPS (66ms between updates) - OPTIMIZED
                    if (now - lastAlphaUpdate > 60)
                    {
                        // Use dynamicMinAlpha for the fade calculation, NOT the global minAlpha
                        BYTE newAlpha = BYTE(dynamicMinAlpha + (currentAlpha - dynamicMinAlpha) * alphaDecay);

                        // Only redraw if alpha actually changed OR if dynamicMinAlpha changed
                        if (newAlpha != currentAlpha || dynamicMinAlpha != lastAppliedMinAlpha)
                        {
                            currentAlpha = newAlpha;
                            lastAppliedMinAlpha = dynamicMinAlpha; // Track what we're applying
                            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                        }
                        lastAlphaUpdate = now;
                    }
                }
            }
            lastKnobPosTrans = knobPos;
        }

        // Settings panel scroll
        float oldOffset = settingsScrollOffsetF;
        settingsScrollOffsetF += (settingsScrollTarget - settingsScrollOffsetF) * settingsScrollSpeed;

        if (abs(settingsScrollOffsetF - oldOffset) > 0.1f)
        {
            settingsScrollOffset = (int)settingsScrollOffsetF;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }

        // Right panel scroll (Keybind, Input, Toggle)
        float oldRightOffset = rightPanelScrollOffsetF;
        rightPanelScrollOffsetF += (rightPanelScrollTarget - rightPanelScrollOffsetF) * rightPanelScrollSpeed;

        if (abs(rightPanelScrollOffsetF - oldRightOffset) > 0.1f)
        {
            rightPanelScrollOffset = (int)rightPanelScrollOffsetF;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        // Redraw update button if available
// Redraw update button if available (ONE-TIME REDRAW)
        static bool lastUpdateAvailable = false;
        if (updateAvailable && !lastUpdateAvailable)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        lastUpdateAvailable = updateAvailable;

