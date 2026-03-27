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
        #include "Windows/Input/Input_Mouse_PRNDL.cpp"
                    else
                    {
                        // --- Check diamond intersection ---
        #include "Windows/Input/Input_Mouse_HShifter.cpp"
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

