                if (!useXInput)
                {
                    // --- Compute movement ---
                    int dx = raw->data.mouse.lLastX;
                    int dy = raw->data.mouse.lLastY;

                    // NEW: shave low-end sensitivity
                    float effectiveSens = powf(knobSensitivity, 1.5f);  // exponent >1 compresses low values gently
                    if (effectiveSens < 0.0f) effectiveSens = 0.0f; // clamp to 0
                    bool rightClickHeld = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

                    // Only disable knob movement if the selected device is the same as the steering device
                    // Only apply special knob/mouse logic if mouse steering is enabled
                    UpdateKnobMovement(hwnd);
                    // --- XInput knob movement ---

                    if (!knobMovementEnabled || rightClickHeld)
                    {
                        delete[] lpb;
                        return 0;
                    }
                    auto& activeMap = lowerGearPositions;

                    if (layoutType == 11) // PRNDL layout
                    {
                        // Vertical movement only for PRNDL
                        knobPos.y += int(dy * effectiveSens);

                        // Minimal horizontal movement with strong centering
                        knobPos.x += int(dx * effectiveSens * 0.1f);
                        knobPos.x += int((railX[0].x - knobPos.x) * 0.3f); // Strong center pull

                        // --- Snap detection for PRNDL ---
                        std::string snappedGear = "";
                        float closestDist = FLT_MAX;

                        for (auto& kv : activeMap)
                        {
                            POINT g = kv.second;
                            std::string gear = kv.first;

                            // For PRNDL, only consider vertical distance for snapping
                            double dist = abs(knobPos.y - g.y);
                            if (dist < gearSnapInThreshold && dist < closestDist)
                            {
                                snappedGear = gear;
                                closestDist = dist;
                            }
                        }

                        bool isSnapped = !snappedGear.empty();

                        // --- Handle snapped state for PRNDL ---
                        if (isSnapped)
                        {
                            // When snapped, allow free vertical movement but keep gear engaged
                            // Only apply minimal smoothing toward the gear position
                            POINT targetPos = activeMap[snappedGear];
                            knobPos.y = int(knobPos.y + (targetPos.y - knobPos.y) * 0.1f); // Very light pull
                            knobPos.x = int(knobPos.x + (targetPos.x - knobPos.x) * 0.3f); // Keep centered

                            // Set gear key
                            SetGearKey(snappedGear);

                            // Update active gear state
                            activeGear = snappedGear;
                            lockedInGear = true;

                            // Release neutral key if held
                            if (neutralHeld)
                            {
                                INPUT input = {};
                                input.type = INPUT_KEYBOARD;
                                input.ki.wVk = gearInputMap["N"].code;
                                input.ki.dwFlags = KEYEVENTF_KEYUP;
                                SendInput(1, &input, sizeof(INPUT));
                                neutralHeld = false;
                            }
                        }
                        else
                        {
                            // Not snapped - check if we left a gear
                            if (lockedInGear && !activeGear.empty())
                            {
                                // Check if we moved far enough from the previously active gear
                                POINT prevGearPos = activeMap[activeGear];
                                double dist = abs(knobPos.y - prevGearPos.y);

                                if (dist > gearSnapOutThreshold)
                                {
                                    // We've moved far enough, release the gear
                                    activeGear = "";
                                    lockedInGear = false;
                                    ReleaseGearKey();
                                }
                                else
                                {
                                    // Still close to previous gear, light pull back toward it
                                    knobPos.y = int(knobPos.y + (prevGearPos.y - knobPos.y) * 0.1f);
                                }
                            }
                        }

                        // Reset rail state for PRNDL (no rail switching)
                        currentRail = VERTICAL;
                        currentVerticalIndex = 0;
                    }
                    else
                    {
                        // --- Check diamond intersection ---
                        bool insideIntersection = IsInsideIntersection(knobPos.x, knobPos.y);

                        // Lerp factor for smooth movement into rails (only applies when not snapped)
                        const float lerpSpeed = 0.2f;
                        const float lerpSpeedHorizontal = 0.05f;
                        const float lerpSpeedVertical = 0.07f;
                        int railCount = is16GearSet ? 5 : 4;

                        // --- Snap detection ---
                        std::string snappedGear = "";
                        for (auto& kv : activeMap)
                        {
                            POINT g = kv.second;
                            std::string gear = kv.first;

                            double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) + (knobPos.y - g.y) * (knobPos.y - g.y));
                            if (dist < gearSnapInThreshold)
                            {
                                snappedGear = gear;
                                break;
                            }
                        }

                        bool isSnapped = !snappedGear.empty();



                        // --- CLUTCH STATE CALCULATION (Global for this frame) ---
                        // "Engaged" means pedal pressed (disengaged engine) for shifting
                        // We calculate this regardless of clutchLockGear enabled state for robust access
                        bool isClutchEngaged = false;
                        if (useScrollClutch || true) // Always check if we have joyRx data
                        {
                            float cNorm = 0.0f;
                            if (useHalfClutch)
                            {
                                LONG mid = (axisMin + axisMax) / 2;
                                cNorm = (float)(joyRx - mid) / (float)(axisMax - mid);
                            }
                            else
                            {
                                cNorm = (float)(joyRx - axisMin) / (float)(axisMax - axisMin);
                            }
                            cNorm = max(0.0f, min(1.0f, cNorm));
                            
                            isClutchEngaged = (cNorm > 0.2f);
                        }

                        // --- CLUTCH LOCK LOGIC: Check State ---
                        if (clutchLockGear && useScrollClutch)
                        {
                            if (!isClutchEngaged)
                            {
                                if (lockedInGear && !activeGear.empty())
                                {
                                    // Force stay in gear
                                    snappedGear = activeGear;
                                    isSnapped = true;
                                }
                                else
                                {
                                    // Force stay in neutral
                                    snappedGear = "";
                                    isSnapped = false;
                                }
                            }
                        }

                        // --- Handle snapped state ---
                        // --- Handle snapped state ---
                        if (isSnapped)
                        {
                            const int maxHorizontalOffset = 20;            // max pixels allowed from snapped X
                            knobPos.x += int(dx * effectiveSens * 0.2f);

                            // Clamp around snapped gear X
                            int snappedX = activeMap[snappedGear].x;
                            if (knobPos.x < snappedX - maxHorizontalOffset)
                                knobPos.x = snappedX - maxHorizontalOffset;
                            if (knobPos.x > snappedX + maxHorizontalOffset)
                                knobPos.x = snappedX + maxHorizontalOffset;

                            // Allow full vertical movement UNLESS clutch locked
                            if (clutchLockGear && useScrollClutch && !isClutchEngaged && lockedInGear)
                            {
                                // Restrict Y movement to gear radius
                                int targetY = activeMap[snappedGear].y;
                                int newY = knobPos.y + int(dy * effectiveSens);
                                if (abs(newY - targetY) > gearRadius)
                                {
                                    if (newY > targetY) newY = targetY + gearRadius;
                                    else newY = targetY - gearRadius;
                                }
                                knobPos.y = newY;
                            }
                            else
                            {
                                int newY = knobPos.y + int(dy * effectiveSens);
                                ClampMovementForReverseLock(newY, knobPos.x);
                                knobPos.y = newY;
                            }

                            // --- DEBUG: Log snappedGear ---
                            char buf[256];
                            //sprintf_s(buf, "Snapped detected: snappedGear='%s'\n", snappedGear.c_str());
                            //OutputDebugStringA(buf);

                            // Always set gear key when snapped
                            SetGearKey(snappedGear);

                            // Update activeGear and lock state
                            activeGear = snappedGear;
                            lockedInGear = true;

                            // Release neutral key if held
                            if (neutralHeld)
                            {
                                INPUT input = {};
                                input.type = INPUT_KEYBOARD;
                                input.ki.wVk = gearInputMap["N"].code;
                                input.ki.dwFlags = KEYEVENTF_KEYUP;
                                SendInput(1, &input, sizeof(INPUT));
                                neutralHeld = false;
                            }
                        }

                        else
                        {
                            // --- Not snapped: regular rail movement with lerp corrections ---

                            if (currentRail == HORIZONTAL)
                            {
                                LONG proposedX = knobPos.x + ApplyKnobMotion(dx, effectiveSens);
                                ClampHorizontalMovementForReverseLock(proposedX, knobPos.y);
                                knobPos.x = proposedX;


                                if (insideIntersection)
                                {
                                    int dY = ApplyKnobMotion(dy, effectiveSens);
                                    
                                    // Safeguard: if in neutral, trying to move vertically, and clutch locked -> block it early
                                    if (clutchLockGear && useScrollClutch && !isClutchEngaged && abs(dY) > 0)
                                    {
                                         dY = 0;
                                    }
                                    int newY = knobPos.y + dY;
                                    ClampMovementForReverseLock(newY, knobPos.x);
                                    knobPos.y = newY;


                                    for (int i = 0; i < railCount; ++i)
                                    {
                                        int railXPos = railX[i].x;
                                        if (abs(knobPos.x - railXPos) < enterVerticalThreshold && abs(dy) > 0)
                                        {
                                            bool movingDown = dy > 0;
                                            bool movingUp = dy < 0;
                                            bool canMoveVertically = false;

                                            for (auto& kv : activeMap)
                                            {
                                                if (kv.second.x == railXPos)
                                                {
                                                    bool isReverse = (kv.first == "R");
                                                    bool reverseLocked = reverseLockEnabled && !IsReverseUnlockActive_Fix();

                                                    if (movingDown && kv.second.y == bottomY) {
                                                        if (!isReverse || !reverseLocked)
                                                            canMoveVertically = true;
                                                    }
                                                    if (movingUp && kv.second.y == topY) {
                                                        if (!isReverse || !reverseLocked)
                                                            canMoveVertically = true;
                                                    }
                                                }
                                            }

                                            if (canMoveVertically)
                                            {
                                                // Prevent vertical entry if clutch is locked and not engaged
                                                if (clutchLockGear && useScrollClutch && !isClutchEngaged)
                                                    canMoveVertically = false;
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
                                    // Soft horizontal lock: blend mouse input with center pull
                                    int newY = knobPos.y + int(dy * effectiveSens * 0.2f);
                                    newY += int((centerY - newY) * 0.05f);

                                    // FIX: Also clamp vertical movement when outside intersection ("Wide" or "Diagonal" state)
                                    // This prevents bypassing the lock by strictly moving diagonally from the side
                                    ClampMovementForReverseLock_Fix(newY, knobPos.x);

                                    knobPos.y = newY;
                                }
                            }
                            else if (currentRail == VERTICAL && currentVerticalIndex != -1)
                            {
                                int newY = knobPos.y + int(dy * effectiveSens);

                                // --- REVERSE LOCK CLAMPING (Unified) ---
                                ClampMovementForReverseLock_Fix(newY, knobPos.x);

                                knobPos.y = newY;

                                // Define railCenter for current vertical rail
                                int railCenter = railX[currentVerticalIndex].x;

                                // Calculate proposed X with motion and centering
                                LONG proposedX = knobPos.x + int(ApplyKnobMotion(dx, effectiveSens) * 0.2f);
                                proposedX += int((railCenter - proposedX) * 0.07f);

                                // Clamp lateral movement to prevent bypassing Reverse Lock from the adjacent rail
                                ClampHorizontalMovementForReverseLock_Fix(proposedX, knobPos.x, knobPos.y);
                                
                                knobPos.x = proposedX;

                                if (IsInsideIntersection(knobPos.x, knobPos.y) && abs(dx) > 0)
                                {
                                    currentRail = HORIZONTAL;
                                    currentVerticalIndex = -1;
                                }
                            }

                            // --- Sticky Authorization Reset ---
                            if (g_reverseAuthorizedOnRail)
                            {
                                // Reset if any gear OTHER than Reverse is active
                                if (!activeGear.empty() && activeGear != "R")
                                {
                                    g_reverseAuthorizedOnRail = false;
                                }
                                else
                                {
                                    for (auto& kv : activeMap)
                                    {
                                        if (kv.first == "R")
                                        {
                                            int reverseX = kv.second.x;
                                            int reverseY = kv.second.y;
                                            
                                            // Horizontal Reset: Knob moved too far from R column
                                            if (abs(knobPos.x - reverseX) > 45)
                                                g_reverseAuthorizedOnRail = false;
                                                
                                            // Vertical Reset (ONLY for Shared Rails)
                                            bool isSharedRail = false;
                                            for (auto& other : activeMap)
                                            {
                                                if (other.first != "R" && other.second.x == reverseX)
                                                {
                                                    isSharedRail = true;
                                                    break;
                                                }
                                            }

                                            if (isSharedRail)
                                            {
                                                // If R is at top, and we move past center towards the bottom gear
                                                if (reverseY == topY && knobPos.y > centerY + 10)
                                                    g_reverseAuthorizedOnRail = false;
                                                // If R is at bottom, and we move past center towards the top gear
                                                if (reverseY == bottomY && knobPos.y < centerY - 10)
                                                    g_reverseAuthorizedOnRail = false;
                                            }
                                                
                                            break;
                                        }
                                    }
                                }
                            }

                            // --- Rigid Boundary Check ---
                            // This ensures that stateless "jumps" (e.g. fast diagonal moves) 
                            // cannot bypass the component-based clamping.
                            EnforceReverseLockBoundary(knobPos);

                            // --- Snap to gear if close ---
                            for (auto& kv : activeMap)
                            {
                                POINT g = kv.second;
                                std::string gear = kv.first;

                                double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) + (knobPos.y - g.y) * (knobPos.y - g.y));
                                bool inside = dist < gearSnapInThreshold;

                                if (gearInsideRadius.find(gear) == gearInsideRadius.end())
                                    gearInsideRadius[gear] = false;

                                if (inside)
                                {
                                    if (!gearInsideRadius[gear])
                                    {
                                        activeGear = gear;
                                        lockedInGear = true;
                                        SetGearKey(gear); // ADD THIS LINE

                                    }

                                    // Smoothly move knob toward gear
                                    if (lockedInGear)
                                    {
                                        knobPos.x = int(knobPos.x + (g.x - knobPos.x) * snapSpeed);
                                        knobPos.y = int(knobPos.y + (g.y - knobPos.y) * snapSpeed);
                                    }

                                    gearInsideRadius[gear] = true;
                                }
                                else
                                {
                                    gearInsideRadius[gear] = false;
                                    if (activeGear == gear)
                                    {
                                        activeGear = "";
                                        lockedInGear = false;
                                        ReleaseGearKey();
                                    }
                                }
                            }
                        }
                    }
                    // --- Clamp to window bounds ---
                    if (knobPos.x < knobMinX)
                        knobPos.x = knobMinX;
                    if (knobPos.x > knobMaxX)
                        knobPos.x = knobMaxX;
                    if (knobPos.y < knobMinY)
                        knobPos.y = knobMinY;
                    if (knobPos.y > knobMaxY)
                        knobPos.y = knobMaxY;

                    // --- Handle Neutral ---
                    // --- Handle Neutral ---
                    bool inNeutral = true;
                    for (auto& kv : activeMap)
                    {
                        POINT g = kv.second;
                        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) + (knobPos.y - g.y) * (knobPos.y - g.y));
                        if (dist < gearSnapInThreshold)
                        {
                            inNeutral = false;
                            break;
                        }
                    }

                    if (inNeutral && isNeutralEnabled)
                    {
                        activeGear = "";
                        lockedInGear = false;
                        ReleaseGearKey();

                        if (!neutralHeld)
                        {
                            // Check the neutral input type
                            GearInput gi = gearInputMap["N"];
                            if (gi.type == KEYBOARD)
                            {
                                INPUT input = {};
                                input.type = INPUT_KEYBOARD;
                                input.ki.wVk = gi.code;
                                input.ki.dwFlags = 0; // key down
                                SendInput(1, &input, sizeof(INPUT));
                            }
                            else if (gi.type == MOUSE)
                            {
                                INPUT input = {};
                                input.type = INPUT_MOUSE;
                                input.mi.dwFlags = (gi.code == RI_MOUSE_LEFT_BUTTON_DOWN ? MOUSEEVENTF_LEFTDOWN : gi.code == RI_MOUSE_RIGHT_BUTTON_DOWN ? MOUSEEVENTF_RIGHTDOWN
                                    : MOUSEEVENTF_MIDDLEDOWN);
                                SendInput(1, &input, sizeof(INPUT));
                            }
                            else if (gi.type == VJOY_BUTTON)
                            {
                                SetBtn(true, vjoyDeviceId, gi.code); // press vJoy button
                            }

                            // === ADD THIS: Trigger glow animation for Neutral ===
                            if (!isBorderless) {
                                if (keybindAnimations.find("N") == keybindAnimations.end()) {
                                    keybindAnimations["N"] = KeybindAnimation();
                                }
                                keybindAnimations["N"].isHeld = true;
                                keybindAnimations["N"].isActive = false;
                                keybindAnimations["N"].glowAlpha = MAX_GLOW_ALPHA;
                                keybindAnimations["N"].activationTime = GetTickCount();
                            }

                            neutralHeld = true;
                        }
                    }
                    else if (!inNeutral && neutralHeld)
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
                            SetBtn(false, vjoyDeviceId, gi.code); // release vJoy button
                        }

                        // === STOP NEUTRAL GLOW WHEN LEAVING NEUTRAL ===
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
                        neutralHeld = false;
                    }
                    // --- Dynamic alpha transparency ---
                    // --- Dynamic alpha transparency ---

                    static POINT lastMouseKnobPos = { 0, 0 };
                    bool mouseKnobChanged = (knobPos.x != lastMouseKnobPos.x || knobPos.y != lastMouseKnobPos.y);

                    // Only process mouse knob changes in windowed mode
                    if (!isBorderless && mouseKnobChanged) {
                        InvalidateRect(hwnd, nullptr, FALSE);
                        lastMouseKnobPos = knobPos;
                    }
                }
            }
        }
        delete[] lpb;
        break;
    }
