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
            showUpdateModal = true; // Trigger the modal pop-up!
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        lastUpdateAvailable = updateAvailable;

