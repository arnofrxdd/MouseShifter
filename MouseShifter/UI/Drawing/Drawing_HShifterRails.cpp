    // --- Draw H-shifter rails ---
    float clutchNorm = 0.0f;
    if (useScrollClutch)
    {
        if (useHalfClutch)
        {
            LONG mid = (axisMin + axisMax) / 2;
            clutchNorm = (float)(joyRx - mid) / (float)(axisMax - mid);
        }
        else
        {
            clutchNorm = (float)(joyRx - axisMin) / (float)(axisMax - axisMin);
        }
        clutchNorm = max(0.0f, min(1.0f, clutchNorm));
    }

    // Compute rail color based on clutch
    BYTE baseR = 200;
    BYTE baseG = 200;
    BYTE baseB = 200;

    // Gradual red glow: increase R, reduce G/B
    BYTE r = (BYTE)(baseR + (255 - baseR) * clutchNorm * 0.5f);
    BYTE g = (BYTE)(baseG * (1.0f - clutchNorm * 0.5f));
    BYTE b = (BYTE)(baseB * (1.0f - clutchNorm * 0.5f));

    Pen dynamicRailPen(Color(r, g, b), 6);
    // Determine rail count for drawing based on layout type
// Determine rail count for drawing based on layout type
// Determine rail count for drawing based on layout type
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

    // Draw rails based on layout type
    if (layoutType == 11)
    {
        // PRNDL layout: single vertical rail only, no horizontal rail
        graphics.DrawLine(&dynamicRailPen, railX[0].x, topY, railX[0].x, bottomY);
    }
    else
    {
        // Standard H-pattern layouts
        // Horizontal neutral rail
        graphics.DrawLine(&dynamicRailPen, railX[0].x, centerY, railX[drawRailCount - 1].x, centerY);

        // Vertical rails
        for (int i = 0; i < drawRailCount; ++i)
        {
            int yStart = topY;
            int yEnd = bottomY;

            bool hasTopGear = false;
            bool hasBottomGear = false;

            for (auto it = lowerGearPositions.begin(); it != lowerGearPositions.end(); ++it)
            {
                POINT pos = it->second;
                if (pos.x == railX[i].x && pos.y == topY)
                    hasTopGear = true;
                if (pos.x == railX[i].x && pos.y == bottomY)
                    hasBottomGear = true;
            }

            for (auto it = highGearPositions.begin(); it != highGearPositions.end(); ++it)
            {
                POINT pos = it->second;
                if (pos.x == railX[i].x && pos.y == topY)
                    hasTopGear = true;
                if (pos.x == railX[i].x && pos.y == bottomY)
                    hasBottomGear = true;
            }

            if (!hasTopGear)
                yStart = centerY;
            if (!hasBottomGear)
                yEnd = centerY;

            if (yStart != yEnd)
                graphics.DrawLine(&dynamicRailPen, railX[i].x, yStart, railX[i].x, yEnd);
        }

        // Draw reverse rail only for layouts that have reverse at top
        if (layoutType == 1) // Normal layout has reverse at top
        {
            graphics.DrawLine(&dynamicRailPen, railX[0].x, topY, railX[0].x, centerY);
        }
    }

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);     // Horizontal center
    format.SetLineAlignment(StringAlignmentCenter); // Vertical center

