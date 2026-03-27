
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

