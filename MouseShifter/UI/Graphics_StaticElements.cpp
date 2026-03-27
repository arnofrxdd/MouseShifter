void DrawStaticShifterElements(Gdiplus::Graphics& graphics)
{
    // --- Draw H-shifter rails (static) ---
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
    BYTE r = (BYTE)(baseR + (255 - baseR) * clutchNorm * 0.5f);
    BYTE g = (BYTE)(baseG * (1.0f - clutchNorm * 0.5f));
    BYTE b = (BYTE)(baseB * (1.0f - clutchNorm * 0.5f));

    Gdiplus::Pen dynamicRailPen(Gdiplus::Color(r, g, b), 6);

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

    // Horizontal neutral rail
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
}
RECT CalculateRailRedrawArea()
{
    // Get rail count for layout
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

    // Calculate boundaries for all rails
    RECT railRect = { INT_MAX, INT_MAX, INT_MIN, INT_MIN };
    int railPadding = 10; // Padding around rails

    // Horizontal neutral rail
    railRect.left = min(railRect.left, railX[0].x - railPadding);
    railRect.top = min(railRect.top, centerY - railPadding);
    railRect.right = max(railRect.right, railX[drawRailCount - 1].x + railPadding);
    railRect.bottom = max(railRect.bottom, centerY + railPadding);

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
        {
            railRect.left = min(railRect.left, railX[i].x - railPadding);
            railRect.top = min(railRect.top, yStart - railPadding);
            railRect.right = max(railRect.right, railX[i].x + railPadding);
            railRect.bottom = max(railRect.bottom, yEnd + railPadding);
        }
    }

    // Reverse rail for layout 1
    if (layoutType == 1)
    {
        railRect.left = min(railRect.left, railX[0].x - railPadding);
        railRect.top = min(railRect.top, topY - railPadding);
        railRect.right = max(railRect.right, railX[0].x + railPadding);
        railRect.bottom = max(railRect.bottom, centerY + railPadding);
    }

    return railRect;
}

bool ShouldRedrawRails()
{
    // Check if clutch position changed significantly
    float currentClutchNorm = 0.0f;
    if (useScrollClutch)
    {
        if (useHalfClutch)
        {
            LONG mid = (axisMin + axisMax) / 2;
            currentClutchNorm = (float)(joyRx - mid) / (float)(axisMax - mid);
        }
        else
        {
            currentClutchNorm = (float)(joyRx - axisMin) / (float)(axisMax - axisMin);
        }
        currentClutchNorm = max(0.0f, min(1.0f, currentClutchNorm));
    }

    // Check if clutch changed significantly (threshold to avoid constant redraws)
    bool clutchChanged = (abs(currentClutchNorm - lastClutchNorm) > 0.01f);

    // Check if rail count changed (layout changed)
    int currentDrawRailCount = 0;
    switch (layoutType)
    {
    case 1: // Normal Layout
    case 3: // Reverse Bottom Layout (First rail)
    case 4: // Reverse Bottom Layout (Last rail)
    case 10: // Reverse Top Last Layout
        currentDrawRailCount = is16GearSet ? 5 : 4;
        break;
    case 2: // No Reverse Layout
        currentDrawRailCount = is16GearSet ? 4 : 3;
        break;
    case 5: // 5-Gear Only Layout
    case 6: // 5-Gear Reverse First Layout
    case 7: // 4-Gear Reverse Top Layout
    case 8: // 4-Gear Reverse Bottom Layout
    case 9: // 4-Gear Reverse Mixed Layout
        currentDrawRailCount = 3; // Always 3 rails for these layouts
        break;
    case 11: // PRNDL Layout
        currentDrawRailCount = 1; // Only one rail for PRNDL
        break;
    }
    bool railCountChanged = (currentDrawRailCount != lastRedrawRailCount);

    // Update tracking variables
    lastClutchNorm = currentClutchNorm;
    lastRedrawRailCount = currentDrawRailCount;

    return clutchChanged || railCountChanged;
}
void DrawStaticGearElements(Gdiplus::Graphics& graphics)
{
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

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

}
