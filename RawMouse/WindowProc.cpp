// ---------------- Window Proc ----------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        ComputeLayout(hwnd);

        ComputeIntersections();
        for (auto& inter : intersections)
        {
            char dbg[128];
            sprintf_s(dbg, "[DEBUG] diamond intersection center=(%d,%d) radius=%d\n",
                inter.x, inter.y, inter.radius);
            OutputDebugStringA(dbg);
        }

        if (!rawInputInitialized)
            InitRawInput(hwnd);
        if (!InitVJoy())
        {
            PostQuitMessage(1);
            return 0;
        }
        {
            int vJoyButtonCount = GetVJoyButtonCount(vjoyDeviceId);
            char dbg[128];
            sprintf_s(dbg, "[vJoy] Device %d has %d buttons\n", vjoyDeviceId, vJoyButtonCount);
            OutputDebugStringA(dbg);
        }
        // ----- ADD DIRECTINPUT INITIALIZATION HERE -----
        if (!InitDirectInput(hwnd))
        {
            OutputDebugStringA("[DirectInput] Failed to initialize\n");
        }
        else
        {
            if (!SetupJoystick(hwnd))
            {
                OutputDebugStringA("[DirectInput] No joystick/pedals detected\n");
            }
        }
        InitGamepads();
        SDL_InitSubSystem(SDL_INIT_EVENTS); // make sure events are enabled
        StartThrottleBrakeThread();

        SetTimer(hwnd, 1, 10, nullptr); // 10 ms = ~100 FPS max achievable
        RefreshProcessList();  // <-- ADD THIS LINE HERE
        tooltips.resize(60);
        // Update check thread - now safely scoped
        std::thread updateThread([]() {
            CheckForUpdates();
            });
        updateThread.detach();

        // SetTimer(hwnd, 1, 16, nullptr); // 60FPS
        break;
    }

    case WM_INPUT:
    {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

        BYTE* lpb = new BYTE[dwSize];
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
        {
            RAWINPUT* raw = (RAWINPUT*)lpb;
            ProcessRawInput(raw); // <-- handles keyboard/mouse → vJoy mapping

            // --- Handle RAW KEYBOARD F1 toggle ---
            if (raw->header.dwType == RIM_TYPEKEYBOARD)
            {
                RAWKEYBOARD& kb = raw->data.keyboard;

                if (kb.Message == WM_KEYDOWN && kb.VKey == VK_F10)
                {
                    is16GearSet = !is16GearSet; // toggle gear set
                    ComputeLayout(hwnd);        // recompute rails & gears

                    ComputeIntersections(); // recompute intersections after layout changes

                    InvalidateRect(hwnd, nullptr, FALSE);
                }

                // Detect key down, ignore key up/repeat
                if (kb.Message == WM_KEYDOWN && kb.VKey == VK_F12)
                {
                    knobDisabledByF9 = !knobDisabledByF9; // toggle disable
                    if (!knobDisabledByF9)
                    {
                        // Enable knob movement permanently if F9 re-enables
                        knobMovementEnabled = true;
                    }

                    char buf[256];
                    sprintf_s(buf, "F9 pressed: knobDisabledByF9 = %s\n", knobDisabledByF9 ? "true" : "false");
                    OutputDebugStringA(buf);
                }



                // Keyboard
            }

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                RAWMOUSE& rm = raw->data.mouse;

                // Update right button state
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                {
                    isRightButtonHeld = true;
                }
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                {
                    isRightButtonHeld = false;
                }

                // Only call UpdateVJoyFromMouse if right button is not held
                if (!isRightButtonHeld)
                {
                    UpdateVJoyFromMouse(rm, raw->header.hDevice);
                    HandleScrollInput(rm);
                }
            }
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                HANDLE device = raw->header.hDevice;
                bool match = false;

                if (g_selectedDevice == NULL)
                {
                    match = true; // All mice: accept every input
                }
                else
                {
                    // Specific device selected
                    for (auto& m : g_mouseDevices)
                    {
                        if (m.hDevice == g_selectedDevice)
                        {
                            // For touchpad, hDevice may be 0
                            if (m.isTouchpad ? device == 0 : device == g_selectedDevice)
                            {
                                match = true;
                                break;
                            }
                        }
                    }
                }

                if (!match)
                {
                    delete[] lpb;
                    return 0; // ignore input from other mice
                }

                RAWMOUSE& rm = raw->data.mouse;

                // <<< ADD HERE >>>
                lastDx += rm.lLastX;
                lastDy += rm.lLastY;

                // delete this lol
            }
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                HANDLE device = raw->header.hDevice;
                bool match = false;

                if (g_selectedDevice == NULL)
                {
                    match = true; // All mice: accept every input
                }
                else
                {
                    // Specific device selected
                    for (auto& m : g_mouseDevices)
                    {
                        if (m.hDevice == g_selectedDevice)
                        {
                            // For touchpad, hDevice may be 0
                            if (m.isTouchpad ? device == 0 : device == g_selectedDevice)
                            {
                                match = true;
                                break;
                            }
                        }
                    }
                }

                if (!match)
                {
                    delete[] lpb;
                    return 0; // ignore input from other mice
                }
                // --- MOUSE BINDING CAPTURE ---
                RAWMOUSE& rm = raw->data.mouse;

                // Only process if user is setting an input from the panel
                if (!inputBeingSet.empty() && !showVJoyPicker)
                {
                    int idx = std::stoi(inputBeingSet); // which inputMap entry we're editing
                    inputMap[idx].type = MOUSE;

                    // Detect which mouse button was pressed
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        inputMap[idx].code = 1;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        inputMap[idx].code = 2;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        inputMap[idx].code = 3;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        inputMap[idx].code = 4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        inputMap[idx].code = 5;
                    else
                        return 0; // unknown button, ignore

                    inputBeingSet.clear(); // finished binding
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                if (toggleInputBeingSet && !togglePedalBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    toggleInputBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

                if (reverseUnlockBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    reverseUnlockBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

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
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        // Handle gear keybind
        if (!keybindBeingSet.empty() && !showVJoyPicker)
        {
            // Set new binding
            gearInputMap[keybindBeingSet] = { KEYBOARD, (WORD)wParam };
            keybindBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // Handle input panel keybind
        if (!inputBeingSet.empty() && !showVJoyPicker)
        {
            int idx = std::stoi(inputBeingSet);
            inputMap[idx].type = KEYBOARD;
            inputMap[idx].code = (WORD)wParam;
            inputBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (toggleInputBeingSet)
        {
            g_knobToggleKey = (int)wParam;
            g_knobToggleType = TOGGLE_KEYBOARD;
            toggleInputBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        if (reverseUnlockBeingSet)
        {
            g_reverseUnlockKey = (int)wParam;
            g_reverseUnlockType = TOGGLE_KEYBOARD;
            reverseUnlockBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }


        else if (wParam == VK_TAB)
        {
            ToggleBorderless(hwnd);
        }
        if (creatingNewProfile) {
            switch (wParam) {
            case VK_LEFT:
                if (profileTextSelectionStart > 0) {
                    profileTextSelectionStart--;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_RIGHT:
                if (profileTextSelectionStart < newProfileName.length()) {
                    profileTextSelectionStart++;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_HOME:
                profileTextSelectionStart = 0;
                profileTextSelectionEnd = 0;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_END:
                profileTextSelectionStart = newProfileName.length();
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_ESCAPE:
                creatingNewProfile = false;
                profileTextSelected = false;
                newProfileName = "New Profile";
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }
        break;
    case WM_CHAR:
    {
        if (creatingNewProfile) {
            if (wParam == VK_RETURN) {
                // Enter pressed - create the profile
                if (!newProfileName.empty()) { // Removed the "New Profile" check
                    CreateNewProfile(hwnd);
                    creatingNewProfile = false;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam == VK_BACK) {
                // Backspace - delete selected text or character before cursor
                if (!newProfileName.empty()) {
                    if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                        // Delete selected text
                        newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    else if (profileTextSelectionStart > 0) {
                        // Delete character before cursor
                        newProfileName.erase(profileTextSelectionStart - 1, 1);
                        profileTextSelectionStart--;
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam >= 32 && wParam <= 126) {
                // Printable characters - replace selection or insert
                char newChar = (char)wParam;

                if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                    // Replace selected text
                    newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                    profileTextSelectionEnd = profileTextSelectionStart;
                }

                // Insert new character
                newProfileName.insert(profileTextSelectionStart, 1, newChar);
                profileTextSelectionStart++;
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            return 0;
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };

        // Handle ALL dropdown clicks first and block any other processing when dropdowns are open
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            bool handledInDropdown = false;

            // Handle profile dropdown clicks
            if (profileDropdownOpen) {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = profileButtonRect.top + 25; // Base Y position without scroll offset

                RECT dropdownRect = {
                    profileButtonRect.left,
                    listY + settingsScrollOffset,
                    profileButtonRect.right,
                    listY + settingsScrollOffset + (int)((listItemHeight + itemGap) * profileNames.size())
                };

                if (PtInRect(&dropdownRect, pt)) {
                    // Check if clicking on existing profiles
                    for (size_t i = 0; i < profileNames.size(); ++i) {
                        RECT itemRect = {
                            dropdownRect.left,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };

                        if (PtInRect(&itemRect, pt)) {
                            SwitchProfile((int)i, hwnd);
                            profileDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                // If we clicked anywhere (inside or outside dropdown), close it and block further processing
                if (!handledInDropdown) {
                    profileDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when profile dropdown was open
            }

            // Handle gear layout dropdown clicks
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
                            currentGearLayout = (int)i;
                            gearLabelOverride = gearLayouts[i];
                            gearLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    gearLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when gear layout dropdown was open
            }

            // Handle h-shifter layout dropdown clicks
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
                            currentHShifterLayout = hShifterLayouts[i].id;
                            layoutType = currentHShifterLayout;
                            hShifterLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    hShifterLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when h-shifter dropdown was open
            }

            return 0; // Safety return
        }
        if (creatingNewProfile) {
            RECT adjustedProfileButtonRect = {
                profileButtonRect.left,
                profileButtonRect.top + settingsScrollOffset,
                profileButtonRect.right,
                profileButtonRect.bottom + settingsScrollOffset
            };
            RECT adjustedCreateProfileButtonRect = {
                createProfileButtonRect.left,
                createProfileButtonRect.top + settingsScrollOffset,
                createProfileButtonRect.right,
                createProfileButtonRect.bottom + settingsScrollOffset
            };

            // Check if click is NOT in the profile creation area
            if (!PtInRect(&adjustedProfileButtonRect, pt) && !PtInRect(&adjustedCreateProfileButtonRect, pt)) {
                // Clicked outside profile creation area - create the profile
                if (!newProfileName.empty()) {
                    CreateNewProfile(hwnd);
                }
                creatingNewProfile = false;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0; // Block further processing
            }
            // If click is inside profile creation area, continue with normal processing
        }

        // Only process these if NO dropdowns are open
        RECT adjustedProfileButtonRect = {
            profileButtonRect.left,
            profileButtonRect.top + settingsScrollOffset,
            profileButtonRect.right,
            profileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedProfileButtonRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }

        RECT adjustedCreateProfileButtonRect = {
            createProfileButtonRect.left,
            createProfileButtonRect.top + settingsScrollOffset,
            createProfileButtonRect.right,
            createProfileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedCreateProfileButtonRect, pt)) {
            creatingNewProfile = true;
            newProfileName = "New Profile";
            profileTextSelected = true;
            profileTextSelectionStart = 0;
            profileTextSelectionEnd = newProfileName.length();
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }


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
                break;
            }

            // Minimize button functionality (for Maximize/Restore button)
            if (PtInRect(&g_MaxButtonRect, pt))
            {
                ShowWindow(hwnd, SW_MINIMIZE);
                break;
            }




            // Drag window by title bar (excluding buttons)
            if (pt.y >= 0 && pt.y < 30 && pt.x < g_MaxButtonRect.left)
            {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                break;
            }
        }
        // Update button click (add debug output)
        if (updateAvailable && !isBorderless && PtInRect(&updateButtonRect, pt))
        {
            char debugBuf[256];
            sprintf_s(debugBuf, "[UPDATE CLICK] Button clicked! Mouse: (%d,%d), Button: (%d,%d)-(%d,%d)\n",
                pt.x, pt.y,
                updateButtonRect.left, updateButtonRect.top,
                updateButtonRect.right, updateButtonRect.bottom);
            OutputDebugStringA(debugBuf);

            std::wstring currentVersionW(currentVersion.begin(), currentVersion.end());
            std::wstring latestVersionW(latestVersion.begin(), latestVersion.end());

            std::wstring message =
                L"A new version is available! Would you like to update now?\n\n"
                L"Current version: " + currentVersionW + L"\n" +
                L"Latest version: " + latestVersionW + L"\n\n" +
                L"Your settings will be preserved.";

            int result = MessageBoxW(hwnd, message.c_str(), L"Update Available", MB_YESNO | MB_ICONINFORMATION);
            if (result == IDYES)
            {
                OutputDebugStringA("[UPDATE] User chose to update - starting update process...\n");
                PerformUpdate();
            }
            else
            {
                OutputDebugStringA("[UPDATE] User chose to skip update\n");
            }
            return 0; // Important: return here to prevent click from passing through
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
            mouseX >= panelRect.X && mouseX <= panelRect.X + panelRect.Width &&
            mouseY >= panelRect.Y && mouseY <= panelRect.Y + panelRect.Height)
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
            int index = (mouseY - yStart) / (rowHeight + rowSpacing);

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

        if (showSettingsPanel)
        {
            RECT r; // temporary rect for PtInRect

            r = GetScrolledRect(knobSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingKnobSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(gearSliderRect);
            if (PtInRect(&r, pt))
            {
                isDraggingGearRadius = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(sensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSensSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(diagSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingDiagSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapInSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapInSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(noReverseToggleRect);
            if (PtInRect(&r, pt))
            {
                noReverseLayout = !noReverseLayout;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                ComputeLayout(hwnd);
                ComputeIntersections();
            }
            r = GetScrolledRect(hideHighGearsToggleRect);
            if (PtInRect(&r, pt))
            {
                hideHighGears = !hideHighGears;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw

                ComputeLayout(hwnd); // if needed
            }
            r = GetScrolledRect(useScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useScrollClutch = !useScrollClutch;           // toggle the flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            r = GetScrolledRect(reverseLockToggleRect);
            if (PtInRect(&r, pt))
            {
                reverseLockEnabled = !reverseLockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // DLL INJECTION CLICK 

            // --- DLL Injection Controls ---
            r = GetScrolledRect(processComboRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                if (!g_processList.empty()) {
                    static size_t currentIndex = 0;
                    currentIndex = (currentIndex + 1) % g_processList.size();
                    g_selectedProcessId = g_processList[currentIndex];
                }
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(refreshButtonRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(autoInjectButtonRect);
            if (PtInRect(&r, pt))
            {
                g_autoInjectEnabled = !g_autoInjectEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Toggle Mouse Block checkbox
            r = GetScrolledRect(mouseBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_mouseBlockEnabled = !g_mouseBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Toggle XInput Block checkbox
            r = GetScrolledRect(xinputBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_xinputBlockEnabled = !g_xinputBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(smoothScrollSlider);
            if (PtInRect(&r, pt))
            {
                draggingSmoothScrollSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(brakeresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingBrakeResistanceSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(accelerationresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingAccelerationResistanceSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapSpeedSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapSpeedSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(layoutScaleSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingLayoutSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(useAxisSmoothingToggle);
            if (PtInRect(&r, pt))
            {
                useAxisSmoothing = !useAxisSmoothing;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Add this with your other slider dragging handlers:
            r = GetScrolledRect(axisSmoothingFactorSlider);
            if (PtInRect(&r, pt))
            {
                draggingAxisSmoothingFactorSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(neutralToggleRect);
            if (PtInRect(&r, pt))
            {
                bool wasEnabled = isNeutralEnabled;
                isNeutralEnabled = !isNeutralEnabled;

                // If Neutral was just disabled, release the Neutral key
                if (wasEnabled && !isNeutralEnabled && neutralHeld)
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
                        SetBtn(false, vjoyDeviceId, gi.code);
                    }

                    // Stop Neutral glow animation
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();
                    }

                    neutralHeld = false;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(showYBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showYBar = !showYBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(invertScrollToggleRect);
            if (PtInRect(&r, pt))
            {
                invertScrollClutchAxis = !invertScrollClutchAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Binding Mode For R-Axis toggle
            r = GetScrolledRect(bindingModeForAxisToggle);
            if (PtInRect(&r, pt))
            {
                bindingModeForRAxis = !bindingModeForRAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(realisticKnobToggleRect);
            if (PtInRect(&r, pt))
            {
                realisticKnob = !realisticKnob;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                InvalidateRect(hwnd, NULL, TRUE); // redraw main window to update knob appearance
            }
            // Clutch Lock Gear Toggle
            r = GetScrolledRect(clutchLockGearToggleRect);
            if (PtInRect(&r, pt))
            {
                clutchLockGear = !clutchLockGear;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Invert Assist Axes toggle
            r = GetScrolledRect(invertAssistToggleRect);
            if (PtInRect(&r, pt))
            {
                invertAssistAxes = !invertAssistAxes;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(yBarAlphaSlider);
            if (PtInRect(&r, pt))
            {
                draggingYBarAlphaSlider = true;
                SetCapture(hwnd);
            }

            // Add this after the showYBar toggle click handling:
            r = GetScrolledRect(yBarFixedTransToggle);
            if (PtInRect(&r, pt))
            {
                useYbarFixedTransparency = !useYbarFixedTransparency;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Use LT as Clutch toggle
            r = GetScrolledRect(useLTAsClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useLTAsClutch = !useLTAsClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showXBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showXBar = !showXBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(knobAccelToggleRect);
            if (PtInRect(&r, pt))
            {
                knobAccelerationEnabled = !knobAccelerationEnabled; // toggle boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);    // redraw
            }
            r = GetScrolledRect(useXInputToggleRect);
            if (PtInRect(&r, pt))
            {
                useXInput = !useXInput;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(halfScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useHalfClutch = !useHalfClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(optimizationToggleRect);
            if (PtInRect(&r, pt))
            {
                disableSmartRedraws = !disableSmartRedraws; // Toggle the actual flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(controllerSensSliderRect);
            if (PtInRect(&r, pt))
            {
                controllerDraggingSensSlider = true;
                SetCapture(hwnd);
            }
            // --- Use Right Stick for Knob Toggle ---
            r = GetScrolledRect(useRightStickToggleRect);
            if (PtInRect(&r, pt))
            {
                useRightStick = !useRightStick; // toggle the boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(disableRealKnobMovementToggleRect);
            if (PtInRect(&r, pt))
            {
                disableRealKnobMovement = !disableRealKnobMovement;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Add this with your other toggle click handlers:
            r = GetScrolledRect(useThrottleBrakeToggleRect);
            if (PtInRect(&r, pt))
            {
                useThrottleBrakeAxes = !useThrottleBrakeAxes;

                // If disabling, reset throttle/brake to prevent stuck inputs
                if (!useThrottleBrakeAxes)
                {
                    ResetThrottleBrake();
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(dynamicTransparencyToggleRect);
            if (PtInRect(&r, pt))
            {
                dynamicTransparencyEnabled = !dynamicTransparencyEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(minTransparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingMinTransparencySlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(transparencyFadeDelaySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencyFadeDelaySlider = true;
                SetCapture(hwnd);

            }

            r = GetScrolledRect(transparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencySlider = true;
                SetCapture(hwnd);
            }
            // Steering Sensitivity
            r = GetScrolledRect(steeringSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(steeringDegreesSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringDegreesSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Acc/Brake Sensitivity
            r = GetScrolledRect(accBrakeSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingAccBrakeSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(scrollSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingScrollSensSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                showClutchIndicator = !showClutchIndicator;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Steering toggle ---
            r = GetScrolledRect(mouseSteeringToggleRect);
            if (PtInRect(&r, pt))
            {
                mouseSteeringEnabled = !mouseSteeringEnabled;

                if (IsKnobToggleActive())
                {
                    mouseTrackingEnabled = false;
                    knobMovementEnabled = true;
                }
                else
                {
                    mouseTrackingEnabled = mouseSteeringEnabled;
                    knobMovementEnabled = !mouseSteeringEnabled;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Device Selector ---
            r = GetScrolledRect(deviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t idx = 0;
                if (g_selectedDevice)
                {
                    for (size_t i = 0; i < g_mouseDevices.size(); ++i)
                    {
                        if (g_mouseDevices[i].hDevice == g_selectedDevice)
                        {
                            idx = i + 1;
                            break;
                        }
                    }
                }
                if (idx >= g_mouseDevices.size())
                {
                    g_selectedDevice = NULL; // All Mice
                }
                else
                {
                    g_selectedDevice = g_mouseDevices[idx].hDevice;
                }

                InvalidateRect(hwnd, &settingsPanelRect, TRUE); // redraw
            }
            r = GetScrolledRect(gamepadComboRect);
            if (PtInRect(&r, pt)) // `pt` is mouse click position
            {
                if (!g_gamepads.empty()) {
                    // cycle through only real gamepads
                    g_selectedGamepadIndex = (g_selectedGamepadIndex + 1) % g_gamepads.size();
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }


            // --- Steering Device Selector ---
            r = GetScrolledRect(steeringDeviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t index = 0;
                if (g_selectedSteeringDevice)
                {
                    for (index = 0; index < g_mouseDevices.size(); index++)
                    {
                        if (g_mouseDevices[index].hDevice == g_selectedSteeringDevice)
                        {
                            break;
                        }
                    }
                    index = (index + 1) % (g_mouseDevices.size() + 1); // +1 for "All Mice"
                }
                else
                {
                    index = 0; // start at first device if currently on "All Mice"
                }

                if (index < g_mouseDevices.size())
                {
                    g_selectedSteeringDevice = g_mouseDevices[index].hDevice;
                }
                else
                {
                    g_selectedSteeringDevice = NULL; // "All Mice"
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            r = GetScrolledRect(gearLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                gearLayoutDropdownOpen = !gearLayoutDropdownOpen;
                hShifterLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(hShifterLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                hShifterLayoutDropdownOpen = !hShifterLayoutDropdownOpen;
                gearLayoutDropdownOpen = false; // Close other dropdown
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }

        break;
    }

    case WM_MOUSEMOVE:
    {
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
    }

    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        draggingKnobSlider = false;
        draggingSensSlider = false;
        draggingDiagSlider = false;
        draggingSnapInSlider = false;
        draggingSnapSpeedSlider = false;
        draggingLayoutSlider = false;
        isDraggingGearRadius = false;
        draggingTransparencySlider = false;
        controllerDraggingSensSlider = false;
        draggingSteeringSlider = false;
        draggingAccBrakeSlider = false;
        draggingMinTransparencySlider = false;
        draggingScrollSensSlider = false;
        draggingSteeringDegreesSlider = false;
        draggingTransparencyFadeDelaySlider = false;
        draggingAxisSmoothingFactorSlider = false;
        draggingYBarAlphaSlider = false;  // Add this
        draggingSmoothScrollSlider = false;
        draggingBrakeResistanceSlider = false;
        draggingAccelerationResistanceSlider = false;

        break;
    }
    case WM_CAPTURECHANGED:
    {
        // If capture was lost for any reason, stop all dragging
        draggingKnobSlider = false;
        draggingSensSlider = false;
        draggingDiagSlider = false;
        draggingSnapInSlider = false;
        draggingSnapSpeedSlider = false;
        draggingLayoutSlider = false;
        isDraggingGearRadius = false;
        draggingTransparencySlider = false;
        controllerDraggingSensSlider = false;
        draggingSteeringSlider = false;
        draggingAccBrakeSlider = false;
        draggingMinTransparencySlider = false;
        draggingScrollSensSlider = false;
        draggingSteeringDegreesSlider = false;
        draggingTransparencyFadeDelaySlider = false;
        draggingAxisSmoothingFactorSlider = false;
        draggingYBarAlphaSlider = false;  // Add this

        break;
    }
    case WM_TIMER:
    {
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

        break;
    }
    case WM_SIZE: // <-- new case added here
        ComputeLayout(hwnd);

        ComputeIntersections();

        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right;
        int height = rc.bottom;

        // Always use double buffering
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        if (isBorderless)
        {
            // BORDERLESS MODE: Draw elements without any background
            // No background clearing - completely transparent
            DrawBorderless(memDC, width, height);
        }
        else
        {
            // WINDOWED MODE: Draw full interface
            DrawShifterGDIPlus(hwnd, memDC);
        }

        // Blit to screen
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_NCHITTEST:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int border = 8; // resize border thickness
        const int titleHeight = 30;

        // Top-left corner
        if (pt.x < border && pt.y < border)
            return HTTOPLEFT;
        // Top-right corner
        if (pt.x >= rc.right - border && pt.y < border)
            return HTTOPRIGHT;
        // Bottom-left corner
        if (pt.x < border && pt.y >= rc.bottom - border)
            return HTBOTTOMLEFT;
        // Bottom-right corner
        if (pt.x >= rc.right - border && pt.y >= rc.bottom - border)
            return HTBOTTOMRIGHT;

        // Top edge
        if (pt.y < border)
            return HTTOP;
        // Bottom edge
        if (pt.y >= rc.bottom - border)
            return HTBOTTOM;
        // Left edge
        if (pt.x < border)
            return HTLEFT;
        // Right edge
        if (pt.x >= rc.right - border)
            return HTRIGHT;

        // Custom title bar area (excluding buttons)
        if (!isBorderless && pt.y < titleHeight && pt.x < g_MaxButtonRect.left)
            return HTCAPTION;

        // ADD THIS: Bottom drag area (bottom 30 pixels)
        if (!isBorderless && pt.y >= rc.bottom - 30)
            return HTCAPTION;

        return HTCLIENT;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 1250; // minimum width
        mmi->ptMinTrackSize.y = 830;  // minimum height
        // optional: maximum size
        // mmi->ptMaxTrackSize.x = 1920;
        // mmi->ptMaxTrackSize.y = 1080;
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        POINT pt;
        GetCursorPos(&pt);           // get mouse position in screen coords
        ScreenToClient(hwnd, &pt);   // convert to client coords

        // Only scroll if mouse is inside the settings panel
        if (PtInRect(&settingsPanelRect, pt))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam); // ±120
            settingsScrollTarget += delta / 2.0f;       // adjust scroll speed

            // Clamp target
            if (settingsScrollTarget < -settingsScrollMax)
                settingsScrollTarget = -settingsScrollMax;
            if (settingsScrollTarget > 0)
                settingsScrollTarget = 0;

            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }
        // Handle scrolling for the right panel (Keybind, Input, Toggle)
        else if (pt.x >= (int)panelRect.X && pt.x <= (int)(panelRect.X + panelRect.Width) &&
                 pt.y >= (int)panelRect.Y && pt.y <= (int)(panelRect.Y + panelRect.Height))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            rightPanelScrollTarget += delta; // More aggressive: full delta instead of delta / 2.0f

            // Clamp target
            if (rightPanelScrollTarget < -rightPanelScrollMax)
                rightPanelScrollTarget = -rightPanelScrollMax;
            if (rightPanelScrollTarget > 0)
                rightPanelScrollTarget = 0;

            InvalidateRect(hwnd, NULL, FALSE); // Redraw the whole right side area
        }
    }
    break;


    case WM_DESTROY:
        // ============================================================================
        // UNINJECT ALL DLLs FROM TARGET PROCESSES BEFORE EXITING
        // ============================================================================

        // Uninject from currently selected process
        if (g_selectedProcessId != 0) {
            if (g_lastInjectedMouseProcessId != 0) {
                UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
                OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected\n");
            }
            if (g_lastInjectedXinputProcessId != 0) {
                UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
                OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected\n");
            }
        }

        // Also clean up from any previously injected processes
        if (g_lastInjectedMouseProcessId != 0 && g_lastInjectedMouseProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
            OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected from previous process\n");
        }
        if (g_lastInjectedXinputProcessId != 0 && g_lastInjectedXinputProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
            OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected from previous process\n");
        }

        // Reset injection tracking
        g_lastInjectedMouseProcessId = 0;
        g_lastInjectedXinputProcessId = 0;
        DestroyYBarWindow();

        // If neutral key was held, release it
        if (neutralHeld)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = neutralKey;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }

        // --- RELEASE ALL HELD BUTTONS BEFORE EXITING ---
        for (int btn = 1; btn <= vJoyButtonCount; ++btn)
        {
            SetBtn(FALSE, vjoyDeviceId, btn); // Release button
        }
        RelinquishVJD(vjoyDeviceId); // Release vJoy device

        OutputDebugString(L"[vJoy] All buttons released and device relinquished.\n");

        // ----- RELEASE DIRECTINPUT RESOURCES -----
        if (g_pJoystick)
        {
            g_pJoystick->Unacquire();
            g_pJoystick->Release();
            g_pJoystick = nullptr;
        }
        if (g_pDI)
        {
            g_pDI->Release();
            g_pDI = nullptr;
        }
        // ----------------------------------------

        // Save configuration if needed
        SaveConfig();
        StopThrottleBrakeThread();

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
