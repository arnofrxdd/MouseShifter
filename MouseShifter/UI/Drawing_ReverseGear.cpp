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

