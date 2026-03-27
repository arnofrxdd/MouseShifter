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