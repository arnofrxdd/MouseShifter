
    // --- Handle Neutral for real knob ---
    bool inNeutral = true;
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold)
        {
            inNeutral = false;
            break;
        }
    }

    if (inNeutral && isNeutralEnabled && !neutralHeld)
    {
        GearInput gi = gearInputMap["N"];
        if (gi.type == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = gi.code;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(true, vjoyDeviceId, gi.code);
        }
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
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(false, vjoyDeviceId, gi.code);
        }
        // === STOP NEUTRAL GLOW ===
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
        else {
            OutputDebugStringA("=== NEUTRAL GLOW NOT FOUND IN ANIMATIONS ===\n");
        }


        neutralHeld = false;
    }

    // --- DEBUG ---

    // --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
    if (isBorderless && isTransparent && dynamicTransparencyEnabled)
    {
        static DWORD lastActiveTime = 0;
        static DWORD lastAlphaUpdate = 0;
        static DWORD lastBrightnessCheck = 0;
        static BYTE dynamicMinAlpha = minAlpha;
        static BYTE lastAppliedMinAlpha = minAlpha;
        static BYTE lastUserMinAlpha = minAlpha;

        // === ADD HISTORY TRACKING FOR CONTROLLER ===
        static std::vector<BYTE> minAlphaHistory;
        static const int HISTORY_SIZE = 3;
        static const BYTE CHANGE_THRESHOLD = 8;

        DWORD now = GetTickCount();

        // Update if user changed the slider
        if (lastUserMinAlpha != minAlpha) {
            lastUserMinAlpha = minAlpha;
            dynamicMinAlpha = minAlpha;
            lastAppliedMinAlpha = minAlpha;
            minAlphaHistory.clear(); // Clear history on user change
            lastBrightnessCheck = 0;
            lastAlphaUpdate = 0;
        }

        // Check background brightness at 2 FPS (every 500ms) - SAME AS MOUSE VERSION
        if (now - lastBrightnessCheck > 1500)
        {
            float brightness = GetHShifterBackgroundBrightnessDebug(hwnd);

            // Linear mapping: brightness 0-255 maps to brightnessFactor 0.2-1.0
            float brightnessFactor = 0.2f + (brightness / 255.0f) * 0.8f;
            brightnessFactor = max(0.2f, min(1.0f, brightnessFactor));

            BYTE newDynamicMinAlpha = (BYTE)(minAlpha * brightnessFactor);
            newDynamicMinAlpha = max(10, min(150, newDynamicMinAlpha));

            // Add to history
            minAlphaHistory.push_back(newDynamicMinAlpha);
            if (minAlphaHistory.size() > HISTORY_SIZE)
                minAlphaHistory.erase(minAlphaHistory.begin());

            // Smart update check using history (SAME LOGIC AS MOUSE)
            bool shouldUpdate = false;

            if (minAlphaHistory.size() == HISTORY_SIZE)
            {
                int sum = 0;
                for (BYTE val : minAlphaHistory) sum += val;
                BYTE recentAverage = sum / minAlphaHistory.size();

                // Only update if significant deviation from recent trend
                if (abs((int)newDynamicMinAlpha - (int)recentAverage) >= CHANGE_THRESHOLD)
                {
                    shouldUpdate = true;
                }
                else
                {
                    // Check if all recent values are similar (stable state)
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
                // Not enough history, use simple threshold
                shouldUpdate = (abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD);
            }

            if (shouldUpdate)
            {
                dynamicMinAlpha = newDynamicMinAlpha;
                lastAlphaUpdate = 0; // Force redraw on next frame
            }

            lastBrightnessCheck = now;
        }

        // AGGRESSIVE 15 FPS LIMIT (66ms between updates)
        if (now - lastAlphaUpdate >= 66)
        {
            // Detect activity: LB/RB held OR slight knob movement
            bool inputActive = (abs(dx) > 0.1f || abs(dy) > 0.1f);
            if (!invertAssistAxes)
                inputActive = inputActive || assistButtonHeld;

            if (inputActive)
            {
                currentAlpha = maxAlpha;
                lastActiveTime = now; // reset timer
                SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                lastAppliedMinAlpha = dynamicMinAlpha; // Track what's applied
            }
            else if (now - lastActiveTime >= transparencyFadeDelay)
            {
                // Fade back to DYNAMIC minAlpha (not static minAlpha) smoothly
                BYTE newAlpha = BYTE(dynamicMinAlpha + (currentAlpha - dynamicMinAlpha) * alphaDecay);

                // Only update if alpha actually changed OR if dynamicMinAlpha changed
                if (newAlpha != currentAlpha || dynamicMinAlpha != lastAppliedMinAlpha)
                {
                    currentAlpha = newAlpha;
                    lastAppliedMinAlpha = dynamicMinAlpha; // Track what we're applying
                    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                }
            }

            lastAlphaUpdate = now; // Update FPS timer
        }
    }


}