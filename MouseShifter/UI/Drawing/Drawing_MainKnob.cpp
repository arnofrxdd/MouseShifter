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

