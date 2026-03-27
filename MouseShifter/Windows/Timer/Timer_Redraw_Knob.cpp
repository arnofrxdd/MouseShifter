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