    // --- Hotkey overlay ---
    // --- Hotkey overlay ---

    auto DrawHotkeyOverlay = [&](Graphics& g)
    {
        int padding = 40;
        int offsetX = 270;
        int boxWidth = 280;
        int boxHeight = 110; // increased to fit version line

        Rect overlayRect(padding + offsetX, padding, boxWidth, boxHeight);

        // Text
        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 14, FontStyleRegular, UnitPixel);
        SolidBrush textBrush(Color(150, 255, 255, 255)); // ARGB: 150 alpha, white

        PointF textPos(float(overlayRect.X + 10), float(overlayRect.Y + 10));
        g.DrawString(L"F12 - Enable/Disable Knob", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"F11 - Temporarily Disable Mouse Steering", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"F10 - Switch 12/16 Gearbox", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"Tab - Enable/Disable Overlay", -1, &font, textPos, &textBrush);

        // Add version info
        textPos.Y += 18;
        std::wstring versionText = L"Version: " + std::wstring(currentVersion.begin(), currentVersion.end());
        g.DrawString(versionText.c_str(), -1, &font, textPos, &textBrush);
    };

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

    // --- Draw gears ---
    bool greyOutHShifter = !knobMovementEnabled;

    for (auto& kv : lowerGearPositions)
    {
        std::string lowGear = kv.first;
        POINT pos = kv.second;

        if (lowGear == "R")
            continue;

        // Always compute high gear
        std::string highGear;
        if (is16GearSet)
        {
            int lowNum = std::stoi(lowGear);
            highGear = std::to_string(lowNum + 8);
        }
        else
        {
            if (lowGear == "1")
                highGear = "7";
            else if (lowGear == "2")
                highGear = "8";
            else if (lowGear == "3")
                highGear = "9";
            else if (lowGear == "4")
                highGear = "10";
            else if (lowGear == "5")
                highGear = "11";
            else if (lowGear == "6")
                highGear = "12";
        }

        // Override labels
        std::string displayLowGear = lowGear;
        if (gearLabelOverride.find(lowGear) != gearLabelOverride.end())
            displayLowGear = gearLabelOverride[lowGear];

        std::string displayHighGear = highGear;
        if (gearLabelOverride.find(highGear) != gearLabelOverride.end())
            displayHighGear = gearLabelOverride[highGear];

        // Only check active gear against low gear
        bool isActive = (activeGear == lowGear);
        Color gearColor = isActive ? Color(200, 200, 200) : Color(30, 30, 30);
        if (greyOutHShifter && !isActive)
            gearColor = Color(80, 80, 80);

        SolidBrush gearBrush(gearColor);
        Pen gearPen(Color(120, 120, 120), 2);

        Rect gearRect(pos.x - gearRadius, pos.y - gearRadius, gearRadius * 2, gearRadius * 2);
        graphics.FillEllipse(&gearBrush, gearRect);
        graphics.DrawEllipse(&gearPen, gearRect);

        // Gear numbers
// Gear numbers
// Gear numbers
        int gearFontSize = gearRadius / 2;
        Font font(L"Segoe UI", gearFontSize, FontStyleBold, UnitPixel);
        SolidBrush lowTextBrush(isActive ? Color(200, 200, 200) : Color(150, 150, 150));
        if (greyOutHShifter && !isActive)
            lowTextBrush.SetColor(Color(120, 120, 120));

        std::wstring lowW(displayLowGear.begin(), displayLowGear.end());

        // Determine if we should hide high gears for this specific layout
        bool hideHighGearsForLayout = hideHighGears;
        // Explicitly enable hideHighGears for layouts with less than 5 gears (where high gears don't exist)
        if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9 || layoutType == 11)
        {
            hideHighGearsForLayout = true; // Force hide high gears for these layouts
        }

        if (hideHighGearsForLayout)
        {
            // Center low gear label inside circle
            RectF centerRect(pos.x - gearRadius, pos.y - gearFontSize / 2.0f, gearRadius * 2, gearFontSize);
            graphics.DrawString(lowW.c_str(), -1, &font, centerRect, &format, &lowTextBrush);
        }
        else
        {
            // Original split layout
            float fixedSpacing = 6.0f;
            float lowYOffset = -gearFontSize - fixedSpacing / 2.0f;
            float highYOffset = fixedSpacing / 2.0f;
            float textWidth = gearRadius * 2.0f;

            // Low
            RectF lowRectF(pos.x - textWidth / 2.f, pos.y + lowYOffset, textWidth, gearFontSize);
            graphics.DrawString(lowW.c_str(), -1, &font, lowRectF, &format, &lowTextBrush);

            // High
            SolidBrush highTextBrush(activeGear == highGear ? Color(200, 200, 200) : Color(150, 150, 150));
            if (greyOutHShifter && !(activeGear == lowGear || activeGear == highGear))
                highTextBrush.SetColor(Color(120, 120, 120));

            RectF highRectF(pos.x - textWidth / 2.f, pos.y + highYOffset, textWidth, gearFontSize);
            std::wstring highW(displayHighGear.begin(), displayHighGear.end());
            graphics.DrawString(highW.c_str(), -1, &font, highRectF, &format, &highTextBrush);
        }

    }

    // --- Draw reverse gear ---
// --- Draw reverse gear ---
// Only draw reverse gear if the current layout has reverse
    bool shouldDrawReverse = false;
    std::string reverseGearKey = "R";

    // Check if reverse exists in either lower or high gear positions
    if (lowerGearPositions.find(reverseGearKey) != lowerGearPositions.end() ||
        highGearPositions.find(reverseGearKey) != highGearPositions.end())
    {
        shouldDrawReverse = true;
    }

    if (shouldDrawReverse)
    {
        POINT rPos;
        // Prefer lower gear position for reverse, fall back to high gear position
        if (lowerGearPositions.find(reverseGearKey) != lowerGearPositions.end())
        {
            rPos = lowerGearPositions["R"];
        }
        else
        {
            rPos = highGearPositions["R"];
        }

        bool rActive = activeGear == "R";
        Color rColor = rActive ? Color(200, 200, 200) : Color(30, 30, 30);
        if (greyOutHShifter && !rActive)
            rColor = Color(80, 80, 80);

        SolidBrush rBrush(rColor);
        Pen rPen(Color(120, 120, 120), 2);
        Rect rRect(rPos.x - gearRadius, rPos.y - gearRadius, gearRadius * 2, gearRadius * 2);
        graphics.FillEllipse(&rBrush, rRect);
        graphics.DrawEllipse(&rPen, rRect);

        int rFontSize = gearRadius / 2;
        Font rFont(L"Segoe UI", rFontSize, FontStyleBold, UnitPixel);
        SolidBrush rTextBrush(rActive ? Color(200, 200, 200) : Color(150, 150, 150));
        if (greyOutHShifter && !rActive)
            rTextBrush.SetColor(Color(120, 120, 120));
        RectF rTextRectF(rPos.x - gearRadius / 2.f, rPos.y - rFontSize / 2.f, gearRadius, rFontSize);

        // Override R label
        std::wstring rW = L"R";
        if (gearLabelOverride.find("R") != gearLabelOverride.end())
            rW = std::wstring(gearLabelOverride["R"].begin(), gearLabelOverride["R"].end());

        graphics.DrawString(rW.c_str(), -1, &rFont, rTextRectF, &format, &rTextBrush);
    }

    // --- Draw ghost knob (assist pointer) ---
// Draw ghost knob (assist pointer) - ONLY when assist button is HELD
    if (useAssistPointer && assistButtonHeld)
    {
        // Make ghost slightly smaller than real knob
        float ghostRadius = knobRadius * 0.70f;

        // Brighter color if gear label present
        Color ghostColor = ghostSnappedGear.empty() ? Color(150, 150, 255, 128)
            : Color(200, 200, 255, 180); // brighter and more opaque
        SolidBrush ghostBrush(ghostColor);
        Pen ghostPen(ghostColor, 2);

        Rect ghostRect(
            ghostKnobPos.x - ghostRadius,
            ghostKnobPos.y - ghostRadius,
            ghostRadius * 2,
            ghostRadius * 2);

        graphics.FillEllipse(&ghostBrush, ghostRect);
        graphics.DrawEllipse(&ghostPen, ghostRect);

        if (!ghostSnappedGear.empty())
        {
            Font font(L"Segoe UI", static_cast<INT>(ghostRadius / 2), FontStyleBold, UnitPixel);
            SolidBrush textBrush(Color(255, 255, 255, 220)); // slightly more opaque text
            std::string gearLabel = ghostSnappedGear;
            if (gearLabelOverride.find(ghostSnappedGear) != gearLabelOverride.end())
                gearLabel = gearLabelOverride[ghostSnappedGear];

            std::wstring wGear(gearLabel.begin(), gearLabel.end());

            RectF textRect(
                ghostKnobPos.x - ghostRadius,
                ghostKnobPos.y - ghostRadius / 2,
                ghostRadius * 2,
                ghostRadius);

            StringFormat format;
            format.SetAlignment(StringAlignmentCenter);
            format.SetLineAlignment(StringAlignmentCenter);
            graphics.DrawString(wGear.c_str(), -1, &font, textRect, &format, &textBrush);
        }
    }
    // --- Draw vertical Y-axis bar next to H-Shifter ---




    // --- Draw knob ---
// --- Draw knob ---
   // --- Draw knob ---
    bool isGearActive = !activeGear.empty();

    // Default colors
    Color knobFillColor = isGearActive ? Color(180, 180, 180) : Color(100, 100, 100);
    Color knobBorderColor = isGearActive ? Color(150, 150, 150) : Color(80, 80, 80);
    bool anyPressed = false;
    for (auto& kv : buttonPressedState)
    {
        if (kv.second) { anyPressed = true; break; }
    }

    // Set or fade flash
    if (anyPressed)
        knobFlash = 0.5f; // subtle max flash
    else
        knobFlash = max(0.0f, knobFlash - knobFlashFade);

    // Apply fading flash to knob color
    Color baseFill = isGearActive ? Color(180, 180, 180) : Color(100, 100, 100);
    Color baseBorder = isGearActive ? Color(150, 150, 150) : Color(80, 80, 80);

    if (knobFlash > 0.0f)
    {
        BYTE r = (BYTE)(baseFill.GetRed() + (255 - baseFill.GetRed()) * knobFlash);
        BYTE g = (BYTE)(baseFill.GetGreen() + (255 - baseFill.GetGreen()) * knobFlash);
        BYTE b = (BYTE)(baseFill.GetBlue() + (0 - baseFill.GetBlue()) * knobFlash);
        knobFillColor = Color(r, g, b);

        BYTE br = (BYTE)(baseBorder.GetRed() + (200 - baseBorder.GetRed()) * knobFlash);
        BYTE bg = (BYTE)(baseBorder.GetGreen() + (150 - baseBorder.GetGreen()) * knobFlash);
        BYTE bb = (BYTE)(baseBorder.GetBlue() + (0 - baseBorder.GetBlue()) * knobFlash);
        knobBorderColor = Color(br, bg, bb);
    }
    else
    {
        knobFillColor = baseFill;
        knobBorderColor = baseBorder;
    }

    // Override if vJoy is disabled
    if (!vJoyMouseEnabled)
    {
        knobFillColor = Color(220, 50, 50);
        knobBorderColor = Color(150, 0, 0);
    }

    // --- Smooth clutch glow (red) ---
    if (useScrollClutch)
    {
        if (!vJoyMouseEnabled)
        {
            // vJoy disabled: use fixed purple color
            knobFillColor = Color(180, 50, 200);   // bright purple fill
            knobBorderColor = Color(120, 0, 150);  // darker purple border
        }
        else
        {
            // --- Smooth clutch glow (red) ---
            float clutchNorm;
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

            // Base colors
            BYTE baseR = isGearActive ? 180 : 100;
            BYTE baseG = isGearActive ? 180 : 100;
            BYTE baseB = isGearActive ? 180 : 100;

            BYTE baseBR = isGearActive ? 150 : 80;
            BYTE baseBG = isGearActive ? 150 : 80;
            BYTE baseBB = isGearActive ? 150 : 80;

            // Apply button flash
            if (knobFlash > 0.0f)
            {
                baseR = (BYTE)(baseR + (255 - baseR) * knobFlash);
                baseG = (BYTE)(baseG + (255 - baseG) * knobFlash);
                baseB = (BYTE)(baseB + (0 - baseB) * knobFlash);

                baseBR = (BYTE)(baseBR + (200 - baseBR) * knobFlash);
                baseBG = (BYTE)(baseBG + (150 - baseBG) * knobFlash);
                baseBB = (BYTE)(baseBB + (0 - baseBB) * knobFlash);
            }

            // Apply clutch red glow on top (blends with flash)
            BYTE finalR = (BYTE)(baseR + (255 - baseR) * clutchNorm * 0.5f);
            BYTE finalG = (BYTE)(baseG * (1.0f - clutchNorm * 0.5f));
            BYTE finalB = (BYTE)(baseB * (1.0f - clutchNorm * 0.5f));

            BYTE finalBR = (BYTE)(baseBR + (255 - baseBR) * clutchNorm * 0.5f);
            BYTE finalBG = (BYTE)(baseBG * (1.0f - clutchNorm * 0.5f));
            BYTE finalBB = (BYTE)(baseBB * (1.0f - clutchNorm * 0.5f));

            knobFillColor = Color(finalR, finalG, finalB);
            knobBorderColor = Color(finalBR, finalBG, finalBB);
        }
    }

    SolidBrush knobBrush(knobFillColor);
    Pen knobPen(knobBorderColor, 2);

    if (realisticKnob)
    {
        // --- REALISTIC KNOB (Inverted Teardrop Shape) ---
        float knobWidth = knobRadius * 1.6f;
        float knobHeight = knobRadius * 2.0f;

        // Create inverted teardrop shape with rounded bottom
        GraphicsPath knobPath;

        // Top rounded part (semi-circle)
        float topRadius = knobWidth / 2;
        RectF topCircleRect(
            knobPos.x - topRadius,
            knobPos.y - knobHeight / 2,
            knobWidth,
            knobWidth
        );
        knobPath.AddArc(topCircleRect, 180, 180); // Only top half circle

        // Bottom rounded part - curved sides meeting at rounded bottom
        PointF points[7] = {
            // Start from right side of top circle
            PointF(knobPos.x + topRadius, knobPos.y),
            // Curve down right side
            PointF(knobPos.x + topRadius * 0.7f, knobPos.y + knobHeight * 0.3f),
            PointF(knobPos.x + topRadius * 0.4f, knobPos.y + knobHeight * 0.5f),
            // Bottom center (rounded)
            PointF(knobPos.x, knobPos.y + knobHeight / 2),
            // Curve up left side
            PointF(knobPos.x - topRadius * 0.4f, knobPos.y + knobHeight * 0.5f),
            PointF(knobPos.x - topRadius * 0.7f, knobPos.y + knobHeight * 0.3f),
            // Back to left side of top circle
            PointF(knobPos.x - topRadius, knobPos.y)
        };
        knobPath.AddCurve(points, 7);
        knobPath.CloseFigure();

        // Use darker grey fill (much darker than before)
        SolidBrush darkGreyBrush(Color(25, 25, 25)); // Almost black
        // Draw the realistic knob shape
        graphics.FillPath(&darkGreyBrush, &knobPath);

        // Draw outline only for the top circular part with inner padding
        float outlinePadding = knobWidth * 0.13f; // Inner padding for the outline
        float outlineRadius = topRadius - outlinePadding;

        // Create path for top circular outline only
        GraphicsPath topOutlinePath;
        RectF outlineCircleRect(
            knobPos.x - outlineRadius,
            knobPos.y - knobHeight / 2 + outlinePadding, // Adjusted for inner padding
            outlineRadius * 2,
            outlineRadius * 2
        );

        // Add the top arc (semi-circle) for the outline
        topOutlinePath.AddArc(outlineCircleRect, 0, 360); // Full circle
        // Use slightly thicker light grey outline for the top part only
        Pen lightGreyPen(Color(120, 120, 120), 10);   // Light grey outline, even thicker
        // Draw only the top circular outline
        graphics.DrawPath(&lightGreyPen, &topOutlinePath);

        // Draw mini H-shifter pattern on the top surface of the knob
        float miniScale = 0.25f;
        int miniRailCount = drawRailCount;
        float miniTopY = knobPos.y - knobHeight * 0.20f;      // Changed from 0.15f to 0.20f
        float miniBottomY = knobPos.y - knobHeight * 0.00f;   // Changed from +0.05f to 0.00f
        float miniCenterY = knobPos.y - knobHeight * 0.10f;   // Changed from 0.05f to 0.10f
        float miniLeftX = knobPos.x - knobWidth * 0.2f;
        float miniRightX = knobPos.x + knobWidth * 0.2f;

        // Calculate mini rail positions
        std::vector<float> miniRailX;
        for (int i = 0; i < miniRailCount; ++i)
        {
            float ratio = (float)i / (float)(miniRailCount - 1);
            miniRailX.push_back(miniLeftX + (miniRightX - miniLeftX) * ratio);
        }

        // Draw mini horizontal neutral rail
        Pen miniRailPen(Color(100, 100, 100), 1); // Darker grey for mini rails
        graphics.DrawLine(&miniRailPen, miniRailX[0], miniCenterY, miniRailX[miniRailCount - 1], miniCenterY);

        // Draw mini vertical rails with the same logic as actual H-shifter
        for (int i = 0; i < miniRailCount; ++i)
        {
            float yStart = miniTopY;
            float yEnd = miniBottomY;

            bool hasTopGear = false;
            bool hasBottomGear = false;

            // Check lower gear positions
            for (auto it = lowerGearPositions.begin(); it != lowerGearPositions.end(); ++it)
            {
                POINT pos = it->second;
                // Map actual rail position to mini rail position
                int actualRailIndex = -1;
                for (int j = 0; j < drawRailCount; ++j)
                {
                    if (pos.x == railX[j].x)
                    {
                        actualRailIndex = j;
                        break;
                    }
                }

                // If this gear is on the current rail in the actual layout
                if (actualRailIndex == i)
                {
                    if (pos.y == topY) hasTopGear = true;
                    if (pos.y == bottomY) hasBottomGear = true;
                }
            }

            // Check high gear positions
            for (auto it = highGearPositions.begin(); it != highGearPositions.end(); ++it)
            {
                POINT pos = it->second;
                // Map actual rail position to mini rail position
                int actualRailIndex = -1;
                for (int j = 0; j < drawRailCount; ++j)
                {
                    if (pos.x == railX[j].x)
                    {
                        actualRailIndex = j;
                        break;
                    }
                }

                // If this gear is on the current rail in the actual layout
                if (actualRailIndex == i)
                {
                    if (pos.y == topY) hasTopGear = true;
                    if (pos.y == bottomY) hasBottomGear = true;
                }
            }

            // Adjust vertical rail endpoints based on gear presence (same logic as main rails)
            if (!hasTopGear)
                yStart = miniCenterY;
            if (!hasBottomGear)
                yEnd = miniCenterY;

            // Only draw if there's actually a vertical segment
            if (yStart != yEnd)
                graphics.DrawLine(&miniRailPen, miniRailX[i], yStart, miniRailX[i], yEnd);
        }

        // Draw mini gear numbers (small and subtle)
        Font miniFont(L"Segoe UI", knobWidth * 0.1f, FontStyleBold, UnitPixel);
        SolidBrush miniTextBrush(Color(180, 180, 180)); // Light grey text
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        // Draw gear numbers on mini shifter
        for (auto& kv : lowerGearPositions)
        {
            std::string gear = kv.first;
            if (gear == "R") continue;

            POINT actualPos = kv.second;
            // Map actual position to mini shifter position
            int railIndex = -1;
            for (int i = 0; i < drawRailCount; ++i)
            {
                if (actualPos.x == railX[i].x)
                {
                    railIndex = i;
                    break;
                }
            }

            if (railIndex >= 0 && railIndex < miniRailCount)
            {
                float miniX = miniRailX[railIndex];
                float miniY = (actualPos.y == topY) ? miniTopY :
                    (actualPos.y == bottomY) ? miniBottomY : miniCenterY;

                // Override label if exists
                std::string displayGear = gear;
                if (gearLabelOverride.find(gear) != gearLabelOverride.end())
                    displayGear = gearLabelOverride[gear];

                std::wstring wGear(displayGear.begin(), displayGear.end());
                RectF textRect(miniX - knobWidth * 0.08f, miniY - knobHeight * 0.04f,
                    knobWidth * 0.16f, knobHeight * 0.08f);
                graphics.DrawString(wGear.c_str(), -1, &miniFont, textRect, &format, &miniTextBrush);
            }
        }

        // Draw reverse if applicable
        if (shouldDrawReverse)
        {
            POINT rPos;
            if (lowerGearPositions.find("R") != lowerGearPositions.end())
                rPos = lowerGearPositions["R"];
            else
                rPos = highGearPositions["R"];

            int railIndex = -1;
            for (int i = 0; i < drawRailCount; ++i)
            {
                if (rPos.x == railX[i].x)
                {
                    railIndex = i;
                    break;
                }
            }

            if (railIndex >= 0 && railIndex < miniRailCount)
            {
                float miniX = miniRailX[railIndex];
                float miniY = (rPos.y == topY) ? miniTopY :
                    (rPos.y == bottomY) ? miniBottomY : miniCenterY;

                std::string displayR = "R";
                if (gearLabelOverride.find("R") != gearLabelOverride.end())
                    displayR = gearLabelOverride["R"];

                std::wstring wR(displayR.begin(), displayR.end());
                RectF textRect(miniX - knobWidth * 0.08f, miniY - knobHeight * 0.04f,
                    knobWidth * 0.16f, knobHeight * 0.08f);
                graphics.DrawString(wR.c_str(), -1, &miniFont, textRect, &format, &miniTextBrush);
            }
        }
        // Removed the yellow highlight effect
    }
    else
    {
        // --- BASIC CIRCLE KNOB (your original code) ---
        // ... keep your existing basic circle knob code here
        // --- BASIC CIRCLE KNOB (your original code) ---
        Rect knobRect(knobPos.x - knobRadius, knobPos.y - knobRadius, knobRadius * 2, knobRadius * 2);
        graphics.FillEllipse(&knobBrush, knobRect);
        graphics.DrawEllipse(&knobPen, knobRect);

        // Draw gear number on basic knob (your existing code)
        if (!activeGear.empty())
        {
            SolidBrush knobTextBrush(Color(255, 255, 255));
            StringFormat format;
            format.SetAlignment(StringAlignmentCenter);
            format.SetLineAlignment(StringAlignmentCenter);

            std::string knobLabel = activeGear;
            if (gearLabelOverride.find(activeGear) != gearLabelOverride.end())
                knobLabel = gearLabelOverride[activeGear];
            std::wstring lowW(knobLabel.begin(), knobLabel.end());

            // Dynamic font sizing
            RectF measuredRect;
            Font* knobFont = nullptr;
            float fontSize = static_cast<float>(knobRadius);
            float maxTextHeight = knobRadius * 0.8f;

            while (fontSize > 4.0f)
            {
                if (knobFont) delete knobFont;
                knobFont = new Font(L"Segoe UI", fontSize, FontStyleBold, UnitPixel);
                graphics.MeasureString(lowW.c_str(), -1, knobFont, PointF(0, 0), &measuredRect);

                if (measuredRect.Width <= knobRadius * 1.8f && measuredRect.Height <= maxTextHeight)
                    break;

                fontSize -= 1.0f;
            }

            bool hideHighGearsForLayout = hideHighGears;
            if (layoutType == 5 || layoutType == 6 || layoutType == 7 || layoutType == 8 || layoutType == 9 || layoutType == 11)
            {
                hideHighGearsForLayout = true;
            }

            if (hideHighGearsForLayout)
            {
                PointF centerPos(knobPos.x, knobPos.y);
                graphics.DrawString(lowW.c_str(), -1, knobFont, centerPos, &format, &knobTextBrush);
            }
            else
            {
                float gapFactor = 0.3f;
                float relativePadding = knobRadius * 0.1f;

                PointF lowPos(knobPos.x, knobPos.y - measuredRect.Height * gapFactor - relativePadding);
                graphics.DrawString(lowW.c_str(), -1, knobFont, lowPos, &format, &knobTextBrush);

                if (activeGear != "R")
                {
                    int lowNum = std::stoi(activeGear);
                    std::string pairedHighGear = std::to_string(is16GearSet ? lowNum + 8 : lowNum + 6);
                    if (gearLabelOverride.find(pairedHighGear) != gearLabelOverride.end())
                        pairedHighGear = gearLabelOverride[pairedHighGear];

                    std::wstring highW(pairedHighGear.begin(), pairedHighGear.end());
                    PointF highPos(knobPos.x, knobPos.y + measuredRect.Height * gapFactor + relativePadding);
                    graphics.DrawString(highW.c_str(), -1, knobFont, highPos, &format, &knobTextBrush);
                }
            }

            delete knobFont;
        }
    }

    //Acc and brake
    // Normalize Y axis
    if (showYBar && mouseSteeringEnabled)
    {
        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
        normalizedY = max(0.0f, min(1.0f, normalizedY));

        // Smaller initial size, scale with knobRadius
        int barWidth = 8; // or smaller if you want a thin bar
        int barHeight = bottomY - topY; // span from top rail to bottom rail
        // Place bar slightly right of the last H-Shifter rail
        int lastRailX = railX[drawRailCount - 1].x;  // rightmost rail
        int yBarOffset = 20; // vertical center offset
        int barY = centerY - barHeight / 2; // keep vertical centered

        // shift horizontally if knob overlaps
        int xBarGap = 20; // minimum horizontal gap from knob
        int barX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

        // Compute dynamic color from green → red based on Y
        BYTE r = (BYTE)(normalizedY * 220);           // red increases as Y increases
        BYTE g = (BYTE)((1.0f - normalizedY) * 220); // green decreases as Y increases
        Color dynamicColor(r, g, 50);                // B stays low for contrast

        // Background (dynamic)
        SolidBrush barBg(dynamicColor);
        Pen barOutline(Color(120, 120, 120), 2);
        Rect barRect(barX, barY, barWidth, barHeight);
        graphics.FillRectangle(&barBg, barRect);
        graphics.DrawRectangle(&barOutline, barRect);

        // Filled portion (grey, inverted: top = min, bottom = max)
        int filledHeight = (int)(barHeight * normalizedY);
        int fillY = barY;                               // start from top
        SolidBrush barFill(Color(50, 50, 50));
        Rect fillRect(barX, fillY, barWidth, filledHeight);
        graphics.FillRectangle(&barFill, fillRect);

        // ---- ADDITION: small horizontal line in middle ----
        int lineY = barY + barHeight / 2;   // middle of the bar, relative to barY
        int lineLength = barWidth;           // fit inside the bar width
        Pen linePen(Color(255, 255, 255), 2); // white line
        graphics.DrawLine(&linePen, barX, lineY, barX + lineLength, lineY);

    }
    // --- Rx / Clutch Indicator Bar ---
    if (showClutchIndicator && useScrollClutch)
    {
        // Inverted normalization (axisMin → full bottom, axisMax → top)
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

        int barWidth = 8;
        int barHeight = bottomY - topY;
        int lastRailX = railX[drawRailCount - 1].x;

        // Compute horizontal position
// Default spacing from last rail
// Compute horizontal position
        int xBarGap = 40; // consistent gap from knob
        int barX = lastRailX + 50; // default spacing from last rail

        // If knob is too close, push clutch bar further right
        barX = max(barX, knobPos.x + knobRadius + xBarGap);

        // If Y-bar is also shown, push clutch bar further away from Y-bar
        if (showYBar && mouseSteeringEnabled)
            barX = max(barX, lastRailX + 50 + xBarGap);


        int barY = centerY - barHeight / 2; // vertically centered

        // Dynamic blueish color
        BYTE r = (BYTE)(normalizedRx * 50);
        BYTE g = 50;
        BYTE b = (BYTE)(normalizedRx * 220);
        Color dynamicColor(r, g, b);

        // Background
        SolidBrush barBg(dynamicColor);
        Pen barOutline(Color(120, 120, 120), 2);
        Rect barRect(barX, barY, barWidth, barHeight);
        graphics.FillRectangle(&barBg, barRect);
        graphics.DrawRectangle(&barOutline, barRect);

        // Filled portion (top = min, bottom = max)
        int filledHeight = (int)(barHeight * normalizedRx);
        int fillY = barY;
        SolidBrush barFill(Color(50, 50, 50));
        Rect fillRect(barX, fillY, barWidth, filledHeight);
        graphics.FillRectangle(&barFill, fillRect);

        // Small horizontal center line
        int lineY = barY + barHeight / 2;
        Pen linePen(Color(255, 255, 255), 2);
        graphics.DrawLine(&linePen, barX, lineY, barX + barWidth, lineY);
    }

    // --- Draw horizontal X-axis indicator (mouse steering) ---
    if (showXBar && mouseSteeringEnabled)
    {
        float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
        normalizedX = max(0.0f, min(1.0f, normalizedX));

        int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
        int xBarWidth = static_cast<int>(fullWidth * 0.7f); // 70% of full width
        xBarWidth = max(xBarWidth, 50); // minimum width
        int xBarHeight = 8;
        int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
        int xBarYOffset = 30; // minimum gap below knob
        int xBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
        int xBarLeft = xCenter - xBarWidth / 2;

        // Background bar
        SolidBrush xBarBg(Color(50, 50, 50));
        Rect xBarRect(xBarLeft, xBarY, xBarWidth, xBarHeight);
        graphics.FillRectangle(&xBarBg, xBarRect);

        // --- ALWAYS SHOW RED LOCKED REGIONS ---
        LONG centerX = (axisMax + axisMin) / 2;
        LONG axisRange = axisMax - axisMin;
        LONG steeringMaxOffset = (LONG)((maxSteeringDegrees / fullWheelDegrees) * (axisRange / 2));
        int leftClampX = xBarLeft + (int)(((centerX - steeringMaxOffset) - axisMinX) * xBarWidth / (axisMaxX - axisMinX));
        int rightClampX = xBarLeft + (int)(((centerX + steeringMaxOffset) - axisMinX) * xBarWidth / (axisMaxX - axisMinX));

        SolidBrush redBrush(Color(200, 50, 50));
        // Left locked zone
        int xBarRight = xBarLeft + xBarWidth;
        // Left locked zone
        graphics.FillRectangle(&redBrush, xBarLeft, xBarY, leftClampX - xBarLeft, xBarHeight);
        // Right locked zone (+1 pixel to fix tiny gap)
        graphics.FillRectangle(&redBrush, rightClampX, xBarY, xBarRight - rightClampX - 1, xBarHeight);

        // Center line
        Pen xCenterLine(Color(180, 180, 180), 2);
        graphics.DrawLine(&xCenterLine, xCenter, xBarY, xCenter, xBarY + xBarHeight);

        // Indicator circle
        int indicatorRadius = xBarHeight * 1.5f;
        int indicatorX = xBarLeft + (int)(normalizedX * xBarWidth);
        int indicatorY = xBarY + xBarHeight / 2;
        SolidBrush indicatorBrush(Color(180, 180, 180));
        Pen indicatorPen(Color(120, 120, 120), 2);
        Rect indicatorRect(indicatorX - indicatorRadius, indicatorY - indicatorRadius, indicatorRadius * 2, indicatorRadius * 2);
        graphics.FillEllipse(&indicatorBrush, indicatorRect);
        graphics.DrawEllipse(&indicatorPen, indicatorRect);
    }

    // --- Draw horizontal X-axis indicator (mouse steering) ---
        // --- Draw gear numbers on knob ---





