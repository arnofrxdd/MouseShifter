int hoveredHShifterLayoutIndex = -1;
void ComputeLayout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    centerX = (rc.right - rc.left) / 2;
    centerY = (rc.bottom - rc.top) / 2;

    int* offsets;
    if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9) {
        // Force 12-gear offsets for layouts with less than 6 gears
        offsets = railOffsets12;
    }
    else {
        // Use normal selection for other layouts
        offsets = is16GearSet ? railOffsets16 : railOffsets12;
    }
    int railCount = 0;

    // Determine rail count based on layout type
    switch (layoutType)
    {
    case 1: // Normal Layout
    case 3: // Reverse Bottom Layout (First rail)
    case 4: // Reverse Bottom Layout (Last rail)
    case 10: // Reverse Top Last Layout
        railCount = is16GearSet ? 5 : 4;
        break;
    case 2: // No Reverse Layout
        railCount = is16GearSet ? 4 : 3;
        break;
    case 5: // 5-Gear Only Layout
    case 6: // 5-Gear Reverse First Layout
    case 7: // 4-Gear Reverse Top Layout
    case 8: // 4-Gear Reverse Bottom Layout  
    case 9: // 4-Gear Reverse Mixed Layout
        railCount = 3; // Always 3 rails for these layouts
        break;
    case 11: // PRNDL Layout
        railCount = 1; // Only one vertical rail for PRNDL
        break;
    }

    // --- Compute center offset if No Reverse Layout to keep rails centered ---

    int centerOffset = 0;
    if (layoutType == 2 || layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9 || layoutType == 11)
    {
        // Use appropriate full rail count based on layout type
        int fullRailCount;
        if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9) {
            fullRailCount = 4; // 12-gear full layout for <6 gear layouts
        }
        else {
            fullRailCount = is16GearSet ? 5 : 4; // Normal logic for other layouts
        }

        int usedRailCount = railCount;                                 // rails we actually use
        int fullWidth = offsets[fullRailCount - 1] - offsets[0];       // width of full layout
        int usedWidth = offsets[usedRailCount - 1] - offsets[0];       // width of used rails
        centerOffset = int((fullWidth - usedWidth) / 2 * layoutScale); // shift rails to center
    }

    // --- Assign rail positions ---
    for (int i = 0; i < railCount; ++i)
    {
        railX[i].x = centerX + int(offsets[i] * layoutScale) + centerOffset;
        railX[i].y = centerY;
    }

    // --- Adjust vertical range based on layout type ---
    if (layoutType == 11) // PRNDL Layout - larger vertical range
    {
        topY = centerY - int(140 * layoutScale);    // Increased from 60 to 80
        bottomY = centerY + int(140 * layoutScale); // Increased from 60 to 80
    }
    else // All other layouts
    {
        topY = centerY - int(60 * layoutScale);
        bottomY = centerY + int(60 * layoutScale);
    }

    knobMinX = railX[0].x;
    knobMaxX = railX[railCount - 1].x;
    knobMinY = topY;
    knobMaxY = bottomY;

    lowerGearPositions.clear();
    highGearPositions.clear();

    // --- Layout 1: Normal Layout ---
    if (layoutType == 1)
    {
        if (!is16GearSet)
        {
            // 12-gear normal layout
            lowerGearPositions["R"] = { railX[0].x, topY };
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };

            highGearPositions["R"] = { railX[0].x, topY };
            highGearPositions["7"] = { railX[1].x, topY };
            highGearPositions["8"] = { railX[1].x, bottomY };
            highGearPositions["9"] = { railX[2].x, topY };
            highGearPositions["10"] = { railX[2].x, bottomY };
            highGearPositions["11"] = { railX[3].x, topY };
            highGearPositions["12"] = { railX[3].x, bottomY };
        }
        else
        {
            // 16-gear normal layout
            lowerGearPositions["R"] = { railX[0].x, topY };
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["7"] = { railX[4].x, topY };
            lowerGearPositions["8"] = { railX[4].x, bottomY };

            highGearPositions["R"] = { railX[0].x, topY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["13"] = { railX[3].x, topY };
            highGearPositions["14"] = { railX[3].x, bottomY };
            highGearPositions["15"] = { railX[4].x, topY };
            highGearPositions["16"] = { railX[4].x, bottomY };
        }
    }
    // --- Layout 2: No Reverse Layout ---
    else if (layoutType == 2)
    {
        if (!is16GearSet)
        {
            // 12-gear, 3 vertical rails (no reverse)
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
        }
        else
        {
            // 16-gear, 4 vertical rails (no reverse)
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
        }
    }
    // --- Layout 3: Reverse Bottom Layout ---
    else if (layoutType == 3)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at bottom
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom

            highGearPositions["7"] = { railX[1].x, topY };
            highGearPositions["8"] = { railX[1].x, bottomY };
            highGearPositions["9"] = { railX[2].x, topY };
            highGearPositions["10"] = { railX[2].x, bottomY };
            highGearPositions["11"] = { railX[3].x, topY };
            highGearPositions["12"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom
        }
        else
        {
            // 16-gear with reverse at bottom
            lowerGearPositions["1"] = { railX[1].x, topY };
            lowerGearPositions["2"] = { railX[1].x, bottomY };
            lowerGearPositions["3"] = { railX[2].x, topY };
            lowerGearPositions["4"] = { railX[2].x, bottomY };
            lowerGearPositions["5"] = { railX[3].x, topY };
            lowerGearPositions["6"] = { railX[3].x, bottomY };
            lowerGearPositions["7"] = { railX[4].x, topY };
            lowerGearPositions["8"] = { railX[4].x, bottomY };
            lowerGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom

            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["13"] = { railX[3].x, topY };
            highGearPositions["14"] = { railX[3].x, bottomY };
            highGearPositions["15"] = { railX[4].x, topY };
            highGearPositions["16"] = { railX[4].x, bottomY };
            highGearPositions["R"] = { railX[0].x, bottomY }; // Reverse at bottom
        }
    }

    // --- Layout 4: Reverse Bottom Last Layout ---
    else if (layoutType == 4)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at bottom on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["R"] = { railX[3].x, bottomY }; // Reverse at bottom on last rail

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["R"] = { railX[3].x, bottomY }; // Reverse at bottom on last rail
        }
        else
        {
            // 16-gear with reverse at bottom on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[4].x, bottomY }; // Reverse at bottom on last rail

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[4].x, bottomY }; // Reverse at bottom on last rail
        }
    }
    // --- Layout 5: 5-Gear Only Layout ---
    else if (layoutType == 5)
    {
        // 5-gear layout only (no 16-gear support)
        // First rail: 1 top, 2 bottom
        lowerGearPositions["1"] = { railX[0].x, topY };
        lowerGearPositions["2"] = { railX[0].x, bottomY };

        // Second rail: 3 top, 4 bottom  
        lowerGearPositions["3"] = { railX[1].x, topY };
        lowerGearPositions["4"] = { railX[1].x, bottomY };

        // Third rail: 5 top, R bottom
        lowerGearPositions["5"] = { railX[2].x, topY };
        lowerGearPositions["R"] = { railX[2].x, bottomY };

        // High range uses same positions (no separate high range for 5-gear)
        highGearPositions["1"] = { railX[0].x, topY };
        highGearPositions["2"] = { railX[0].x, bottomY };
        highGearPositions["3"] = { railX[1].x, topY };
        highGearPositions["4"] = { railX[1].x, bottomY };
        highGearPositions["5"] = { railX[2].x, topY };
        highGearPositions["R"] = { railX[2].x, bottomY };
    }
    // --- Layout 6: 5-Gear Reverse First Layout ---
    else if (layoutType == 6)
    {
        // 5-gear with reverse on first rail top
        // First rail: R top, 1 bottom
        lowerGearPositions["R"] = { railX[0].x, topY };
        lowerGearPositions["1"] = { railX[0].x, bottomY };

        // Second rail: 2 top, 3 bottom  
        lowerGearPositions["2"] = { railX[1].x, topY };
        lowerGearPositions["3"] = { railX[1].x, bottomY };

        // Third rail: 4 top, 5 bottom
        lowerGearPositions["4"] = { railX[2].x, topY };
        lowerGearPositions["5"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[0].x, bottomY };
        highGearPositions["2"] = { railX[1].x, topY };
        highGearPositions["3"] = { railX[1].x, bottomY };
        highGearPositions["4"] = { railX[2].x, topY };
        highGearPositions["5"] = { railX[2].x, bottomY };
    }

    // --- Layout 7: 4-Gear Reverse Top Layout ---
    else if (layoutType == 7)
    {
        // 4-gear with reverse on first rail top
        // First rail: R top, nothing bottom
        lowerGearPositions["R"] = { railX[0].x, topY };

        // Second rail: 1 top, 2 bottom  
        lowerGearPositions["1"] = { railX[1].x, topY };
        lowerGearPositions["2"] = { railX[1].x, bottomY };

        // Third rail: 3 top, 4 bottom
        lowerGearPositions["3"] = { railX[2].x, topY };
        lowerGearPositions["4"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[1].x, topY };
        highGearPositions["2"] = { railX[1].x, bottomY };
        highGearPositions["3"] = { railX[2].x, topY };
        highGearPositions["4"] = { railX[2].x, bottomY };
    }

    // --- Layout 8: 4-Gear Reverse Bottom Layout ---
    else if (layoutType == 8)
    {
        // 4-gear with reverse on first rail bottom
        // First rail: nothing top, R bottom
        lowerGearPositions["R"] = { railX[0].x, bottomY };

        // Second rail: 1 top, 2 bottom  
        lowerGearPositions["1"] = { railX[1].x, topY };
        lowerGearPositions["2"] = { railX[1].x, bottomY };

        // Third rail: 3 top, 4 bottom
        lowerGearPositions["3"] = { railX[2].x, topY };
        lowerGearPositions["4"] = { railX[2].x, bottomY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, bottomY };
        highGearPositions["1"] = { railX[1].x, topY };
        highGearPositions["2"] = { railX[1].x, bottomY };
        highGearPositions["3"] = { railX[2].x, topY };
        highGearPositions["4"] = { railX[2].x, bottomY };
    }

    // --- Layout 9: 4-Gear Reverse Mixed Layout ---
    else if (layoutType == 9)
    {
        // 4-gear with reverse on first rail top, 4th gear on last rail top
        // First rail: R top, 1 bottom
        lowerGearPositions["R"] = { railX[0].x, topY };
        lowerGearPositions["1"] = { railX[0].x, bottomY };

        // Second rail: 2 top, 3 bottom  
        lowerGearPositions["2"] = { railX[1].x, topY };
        lowerGearPositions["3"] = { railX[1].x, bottomY };

        // Third rail: 4 top, nothing bottom
        lowerGearPositions["4"] = { railX[2].x, topY };

        // High range uses same positions
        highGearPositions["R"] = { railX[0].x, topY };
        highGearPositions["1"] = { railX[0].x, bottomY };
        highGearPositions["2"] = { railX[1].x, topY };
        highGearPositions["3"] = { railX[1].x, bottomY };
        highGearPositions["4"] = { railX[2].x, topY };
    }
    // --- Layout 10: Reverse Top Last Layout ---
    else if (layoutType == 10)
    {
        if (!is16GearSet)
        {
            // 12-gear with reverse at top on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["R"] = { railX[3].x, topY }; // Reverse at top on last rail

            highGearPositions["7"] = { railX[0].x, topY };
            highGearPositions["8"] = { railX[0].x, bottomY };
            highGearPositions["9"] = { railX[1].x, topY };
            highGearPositions["10"] = { railX[1].x, bottomY };
            highGearPositions["11"] = { railX[2].x, topY };
            highGearPositions["12"] = { railX[2].x, bottomY };
            highGearPositions["R"] = { railX[3].x, topY }; // Reverse at top on last rail
        }
        else
        {
            // 16-gear with reverse at top on last rail
            lowerGearPositions["1"] = { railX[0].x, topY };
            lowerGearPositions["2"] = { railX[0].x, bottomY };
            lowerGearPositions["3"] = { railX[1].x, topY };
            lowerGearPositions["4"] = { railX[1].x, bottomY };
            lowerGearPositions["5"] = { railX[2].x, topY };
            lowerGearPositions["6"] = { railX[2].x, bottomY };
            lowerGearPositions["7"] = { railX[3].x, topY };
            lowerGearPositions["8"] = { railX[3].x, bottomY };
            lowerGearPositions["R"] = { railX[4].x, topY }; // Reverse at top on last rail

            highGearPositions["9"] = { railX[0].x, topY };
            highGearPositions["10"] = { railX[0].x, bottomY };
            highGearPositions["11"] = { railX[1].x, topY };
            highGearPositions["12"] = { railX[1].x, bottomY };
            highGearPositions["13"] = { railX[2].x, topY };
            highGearPositions["14"] = { railX[2].x, bottomY };
            highGearPositions["15"] = { railX[3].x, topY };
            highGearPositions["16"] = { railX[3].x, bottomY };
            highGearPositions["R"] = { railX[4].x, topY }; // Reverse at top on last rail
        }
    }
    // --- Layout 11: PRNDL Layout ---
    else if (layoutType == 11)
    {
        // PRNDL layout with single vertical rail
        // Using numbers 1-5 instead of PRNDL labels
        int railXPos = railX[0].x; // Only one rail

        // Evenly distribute positions along the vertical rail
        int totalPositions = 5;
        int segmentHeight = (bottomY - topY) / (totalPositions - 1);

        lowerGearPositions["1"] = { railXPos, topY };                    // P position
        lowerGearPositions["2"] = { railXPos, topY + segmentHeight };    // R position  
        lowerGearPositions["3"] = { railXPos, topY + segmentHeight * 2 };// N position
        lowerGearPositions["4"] = { railXPos, topY + segmentHeight * 3 };// D position
        lowerGearPositions["5"] = { railXPos, bottomY };                 // L position

        // High range uses same positions (no separate high range for PRNDL)
        highGearPositions["1"] = { railXPos, topY };
        highGearPositions["2"] = { railXPos, topY + segmentHeight };
        highGearPositions["3"] = { railXPos, topY + segmentHeight * 2 };
        highGearPositions["4"] = { railXPos, topY + segmentHeight * 3 };
        highGearPositions["5"] = { railXPos, bottomY };
    }
    // Set default knob position
    knobPos.x = railX[railCount / 2].x; // Center rail
    knobPos.y = centerY;
}