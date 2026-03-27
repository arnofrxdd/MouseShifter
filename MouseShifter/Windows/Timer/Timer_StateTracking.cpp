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

