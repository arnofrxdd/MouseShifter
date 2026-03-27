
    // --- Snap detection for ghost / targetKnob ---
// --- Snap detection for ghost / targetKnob ---
    auto& activeMap = lowerGearPositions;
    std::string snappedGear = "";

    if (layoutType == 11) // PRNDL layout
    {
        // For PRNDL, only consider vertical distance for snapping
        float closestDist = FLT_MAX;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;

            // For PRNDL, only vertical distance matters
            double dist = abs(targetKnob->y - g.y);
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                snappedGear = gear;
                closestDist = dist;
            }
        }
    }
    else
    {
        // Regular diamond layout snapping
        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
                (targetKnob->y - g.y) * (targetKnob->y - g.y));
            if (dist < gearSnapInThreshold)
            {
                snappedGear = gear;
                break;
            }
        }
    }

    bool isSnapped = !snappedGear.empty();

    if (isSnapped)
    {
        const int maxHorizontalOffset = 20;
        POINT g = activeMap[snappedGear];
        targetKnob->x += (int)roundf(dx * 0.2f);
        if (targetKnob->x < g.x - maxHorizontalOffset)
            targetKnob->x = g.x - maxHorizontalOffset;
        if (targetKnob->x > g.x + maxHorizontalOffset)
            targetKnob->x = g.x + maxHorizontalOffset;
        targetKnob->y += (int)roundf(dy);

        // --- Only update gear if targetKnob is the real knob ---
        // --- Only update gear if targetKnob is the real knob ---
        if (!useAssistPointer)
        {
            SetGearKey(snappedGear);
            activeGear = snappedGear;
            lockedInGear = true;

            // RELEASE NEUTRAL immediately
            if (neutralHeld)
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
                else if (gi.type == VJOY_BUTTON)
                {
                    SetBtn(false, vjoyDeviceId, gi.code);
                }
                neutralHeld = false;
                if (!isBorderless) {
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();

                        char debugBuf[256];
                        sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                            keybindAnimations["N"].isHeld);
                        OutputDebugStringA(debugBuf);

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                }
            }
        }
    }

    else
    {
        if (layoutType == 11) // PRNDL layout
        {
            // PRNDL: Vertical movement only with strong horizontal centering
            targetKnob->y += (int)roundf(dy);
            targetKnob->x += (int)roundf(dx * 0.1f); // Minimal horizontal movement
            targetKnob->x += (int)roundf((railX[0].x - targetKnob->x) * 0.3f); // Strong center pull

            // Reset rail state for PRNDL (no rail switching)
            currentRail = VERTICAL;
            currentVerticalIndex = 0;
        }
        else
        {
            // --- Not snapped: regular rail + lerp logic ---
            bool insideIntersection = IsInsideIntersection(targetKnob->x, targetKnob->y);
            if (currentRail == HORIZONTAL)
            {
                targetKnob->x += (int)roundf(dx);

                if (insideIntersection)
                {
                    targetKnob->y += (int)roundf(dy);

                    int railCount = is16GearSet ? 5 : 4;
                    for (int i = 0; i < railCount; ++i)
                    {
                        int railXPos = railX[i].x;
                        if (abs(targetKnob->x - railXPos) < enterVerticalThreshold && abs(dy) > 0)
                        {
                            bool movingDown = dy > 0;
                            bool movingUp = dy < 0;
                            bool canMoveVertically = false;

                            for (auto& kv : activeMap)
                            {
                                if (kv.second.x == railXPos)
                                {
                                    if (movingDown && kv.second.y == bottomY)
                                        canMoveVertically = true;
                                    if (movingUp && kv.second.y == topY)
                                        canMoveVertically = true;
                                }
                            }

                            if (canMoveVertically)
                            {
                                currentRail = VERTICAL;
                                currentVerticalIndex = i;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    targetKnob->y += (int)roundf(dy * 0.2f);
                    targetKnob->y += (int)roundf((centerY - targetKnob->y) * 0.05f);
                }
            }
            else if (currentRail == VERTICAL && currentVerticalIndex != -1)
            {
                targetKnob->y += (int)roundf(dy);
                int railCenter = railX[currentVerticalIndex].x;
                targetKnob->x += (int)roundf(dx * 0.2f);
                targetKnob->x += (int)roundf((railCenter - targetKnob->x) * 0.07f);

                if (IsInsideIntersection(targetKnob->x, targetKnob->y) && abs(dx) > 0)
                {
                    currentRail = HORIZONTAL;
                    currentVerticalIndex = -1;
                }
            }
        }
    }

    // --- Clamp knob ---
    if (targetKnob->x < knobMinX)
        targetKnob->x = knobMinX;
    if (targetKnob->x > knobMaxX)
        targetKnob->x = knobMaxX;
    if (targetKnob->y < knobMinY)
        targetKnob->y = knobMinY;
    if (targetKnob->y > knobMaxY)
        targetKnob->y = knobMaxY;

    // --- Snap knob ---
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
            (targetKnob->y - g.y) * (targetKnob->y - g.y));
        if (dist < gearSnapInThreshold)
        {
            snappedGear = gear;
            break;
        }
    }

    // --- Handle ghost teleport if assist released ---
    // --- Animate real knob toward ghost knob if assist released ---
    if (!useAssistPointer && !ghostSnappedGear.empty())
    {
        POINT target = lowerGearPositions[ghostSnappedGear];

        // Linear interpolation for smooth movement
        float lerpFactor = 0.50f; // tweak for speed
        knobPos.x = knobPos.x + (int)roundf((target.x - knobPos.x) * lerpFactor);
        knobPos.y = knobPos.y + (int)roundf((target.y - knobPos.y) * lerpFactor);

        // Check if close enough to snap to target
        if (abs(knobPos.x - target.x) < 2 && abs(knobPos.y - target.y) < 2)
        {
            knobPos = target;
            SetGearKey(ghostSnappedGear);
            activeGear = ghostSnappedGear;
            lockedInGear = true;

            // Release neutral key now that we're at the gear
            if (neutralHeld)
            {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = gearInputMap["N"].code;
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                neutralHeld = false;
                if (!isBorderless) {
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();

                        char debugBuf[256];
                        sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                            keybindAnimations["N"].isHeld);
                        OutputDebugStringA(debugBuf);

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                }
            }

            ghostSnappedGear = "";
        }
    }

    if (useAssistPointer)
    {
        // Handle snapping for ghost knob with hysteresis (like real knob)
        std::string gearToSnap = "";
        double closestDist = 1e9;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((ghostKnobPos.x - g.x) * (ghostKnobPos.x - g.x) +
                (ghostKnobPos.y - g.y) * (ghostKnobPos.y - g.y));
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                closestDist = dist;
                gearToSnap = gear;
            }
        }

        if (!ghostKnobSnapped && !gearToSnap.empty())
        {
            ghostKnobPos = activeMap[gearToSnap];
            ghostSnappedGear = gearToSnap;
            ghostKnobSnapped = true;       // you need to define this boolean similar to realKnobSnapped
// Replace: PulseVibrationAsync(0, 65535);
            SafeRumble(0, 65535, 100); // ghost knob
        }
        else if (ghostKnobSnapped && gearToSnap.empty())
        {
            ghostSnappedGear = "";
            ghostKnobSnapped = false;
        }
    }

    // --- Handle snapping for real knob with hysteresis ---
    std::string gearToSnap = "";
    double closestDist = 1e9;

    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold && dist < closestDist)
        {
            closestDist = dist;
            gearToSnap = gear;
        }
    }

    if (!realKnobSnapped && !gearToSnap.empty())
    {
        knobPos = activeMap[gearToSnap];
        SetGearKey(gearToSnap);
        activeGear = gearToSnap;
        lockedInGear = true;
        realKnobSnapped = true;
        // Replace: PulseVibrationAsync(65535, 0);
        SafeRumble(65535, 0, 100); // real knob

        if (neutralHeld)
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
            else if (gi.type == VJOY_BUTTON)
            {
                SetBtn(false, vjoyDeviceId, gi.code);
            }
            neutralHeld = false;
            if (!isBorderless) {
                if (keybindAnimations.find("N") != keybindAnimations.end()) {
                    keybindAnimations["N"].isHeld = false;
                    keybindAnimations["N"].isActive = false;
                    keybindAnimations["N"].activationTime = GetTickCount();

                    char debugBuf[256];
                    sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                        keybindAnimations["N"].isHeld);
                    OutputDebugStringA(debugBuf);

                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
        }
    }

    else if (realKnobSnapped && gearToSnap.empty())
    {
        ReleaseGearKey();
        activeGear = "";
        lockedInGear = false;
        realKnobSnapped = false;
    }