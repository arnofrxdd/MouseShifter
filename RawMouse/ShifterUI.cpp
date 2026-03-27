void DrawShifterGDIPlus(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right;
    int height = rc.bottom;
    int clientWidth = rc.right - rc.left;
    int clientHeight = rc.bottom - rc.top;
    int panelWidth = 300; // width of the keybind panel
    int panelMargin = 10; // distance from right edge
    panelRect = Gdiplus::Rect(
        clientWidth - panelWidth - panelMargin, // X = right-anchored
        panelMargin + 30,                       // leave room for title bar
        panelWidth,                             // Width
        clientHeight - panelMargin * 2          // Height
    );
    settingsPanelRect.left = panelMargin;
    settingsPanelRect.top = panelMargin;
    settingsPanelRect.right = panelMargin + panelWidth;
    settingsPanelRect.bottom = clientHeight - panelMargin;
    // --- Double buffering: create memory HDC and bitmap ---
    HDC memDC = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Initialize GDI+ graphics on memory DC
    // --- Initialize GDI+ graphics on memory DC
    Gdiplus::Graphics graphics(memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // Make the window semi-transparent and blurred
// Windowed mode background
    bool blurSucceeded = EnableWin11Blur(hwnd); // returns true/false
    bool canBlur = EnableWin11Blur(hwnd) && IsTransparencyEnabled();

    if (canBlur)
    {
        graphics.Clear(Gdiplus::Color(150, 8, 8, 8)); // semi-transparent
    }
    else
    {
        graphics.Clear(Gdiplus::Color(255, 8, 8, 8)); // fully opaque fallback
    }

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





    if (showKeybindPanel || showInputPanel || showTogglePanel)
    {
        using namespace Gdiplus;

        // --- Clip drawing to panel area ---
        graphics.SetClip(panelRect);

        FontFamily fontFamily(L"Segoe UI");
        Font titleFont(&fontFamily, 22, FontStyleBold, UnitPixel);
        Font subtitleFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
        Font rowFont(&fontFamily, 16, FontStyleRegular, UnitPixel);

        SolidBrush titleBrush(Color(0, 255, 136));
        SolidBrush subtitleBrush(Color(180, 180, 180));
        SolidBrush textBrush(Color(220, 220, 220));

        StringFormat centerAlign;
        centerAlign.SetAlignment(StringAlignmentCenter);
        centerAlign.SetLineAlignment(StringAlignmentCenter);
        StringFormat leftAlign;
        leftAlign.SetAlignment(StringAlignmentNear);       // horizontal left alignment
        leftAlign.SetLineAlignment(StringAlignmentCenter); // vertical centering
        // Unified starting position
        int y = panelRect.Y + 10 + rightPanelScrollOffset;
        int rightPanelContentBottom = y;

        const int rowHeight = 36;          // height of each interactive row
        const int rowSpacing = 6;          // vertical space between rows
        const int rowPadding = 6;          // padding inside row for text
        const int leftMargin = 10;         // left offset for text inside row
        const int gearColumnWidth = 60;    // width of gear/label column
        const int keyColumnWidth = 280;    // width of key/button column
        const int panelWidthUnified = 320; // width of input/toggle panels
        int panelWidth = panelRect.Width;

        // --- KEYBIND PANEL ---
        if (showKeybindPanel)
        {
            // Title
            RectF titleRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, 30);
            graphics.DrawString(L"Key Bindings", -1, &titleFont, titleRectF, &centerAlign, &titleBrush);

            y += 32;

            // Subtitle
            RectF subtitleRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, 20);
            graphics.DrawString(L"Shift+LMB to pick vJoy buttons", -1, &subtitleFont, subtitleRectF, &centerAlign, &subtitleBrush);

            y += 30;

            // Sorted gear keys
            std::vector<std::string> sortedKeys;
            for (auto& kv : gearInputMap)
                sortedKeys.push_back(kv.first);

            auto isNumber = [](const std::string& s)
            {
                if (s.empty())
                    return false;
                for (char c : s)
                    if (!isdigit(c))
                        return false;
                return true;
            };

            std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const std::string& a, const std::string& b)
                {
                    bool aIsNum = isNumber(a);
                    bool bIsNum = isNumber(b);

                    if (aIsNum && bIsNum) return std::stoi(a) < std::stoi(b);
                    if (aIsNum) return true;
                    if (bIsNum) return false;
                    return a < b; });

            for (auto& gear : sortedKeys)
            {
                GearInput input = gearInputMap[gear];

                std::string keyNameStr = "Unknown";
                if (input.type == KEYBOARD)
                {
                    char buffer[64];
                    if (GetKeyNameTextA(MapVirtualKey(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyNameStr = buffer;
                }
                else if (input.type == MOUSE)
                {
                    switch (input.code)
                    {
                    case 1:
                        keyNameStr = "LMB";
                        break;
                    case 2:
                        keyNameStr = "RMB";
                        break;
                    case 3:
                        keyNameStr = "MMB";
                        break;
                    default:
                        keyNameStr = "Mouse " + std::to_string(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON)
                {
                    keyNameStr = "vJoy Btn " + std::to_string(input.code);
                }

                Color textColor = (keybindBeingSet == gear) ? Color(255, 220, 0) : Color(220, 220, 220);
                SolidBrush textBrushRow(textColor);

                // Check if this gear has an active glow animation
                float glowAlpha = 0.0f;
                if (keybindAnimations.find(gear) != keybindAnimations.end()) {
                    glowAlpha = keybindAnimations[gear].glowAlpha;
                }

                // Base row color
                Color rowColor = (keybindBeingSet == gear) ? Color(40, 40, 0) : Color(36, 36, 36);

                // Apply glow effect if active
                if (glowAlpha > 0.0f) {
                    // Create green glow colors (same as title brush: Color(0, 255, 136))
                    Color innerGlow(0, 255, 136, (int)(glowAlpha * 180));  // Green inner glow
                    Color outerGlow(0, 200, 100, (int)(glowAlpha * 80));   // Slightly darker green outer glow

                    // Draw outer subtle glow
                    SolidBrush outerGlowBrush(outerGlow);
                    RectF outerGlowRect((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, (REAL)rowHeight);
                    graphics.FillRectangle(&outerGlowBrush, outerGlowRect);

                    // Draw inner strong glow
                    SolidBrush innerGlowBrush(innerGlow);
                    RectF innerGlowRect((REAL)panelRect.X + 1, (REAL)y + 1, (REAL)panelWidth - 2, (REAL)rowHeight - 2);
                    graphics.FillRectangle(&innerGlowBrush, innerGlowRect);

                    // Make the row slightly brighter during glow with green tint
                    rowColor = Color(
                        min(255, rowColor.GetR() + (int)(20 * glowAlpha)),  // Less red
                        min(255, rowColor.GetG() + (int)(40 * glowAlpha)),  // More green
                        min(255, rowColor.GetB() + (int)(10 * glowAlpha))   // Slight blue
                    );
                }

                SolidBrush rowBrush(rowColor);

                // Remove border by using the same rect without +2/-4 adjustments
                RectF rowRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, (REAL)rowHeight);
                graphics.FillRectangle(&rowBrush, rowRectF);

                RectF gearRectF((REAL)panelRect.X + 10, (REAL)(y + rowPadding / 2), 50.0f, (REAL)(rowHeight - rowPadding));
                StringFormat leftAlign;
                leftAlign.SetAlignment(StringAlignmentNear);
                leftAlign.SetLineAlignment(StringAlignmentCenter);
                std::wstring gearW(gear.begin(), gear.end());
                graphics.DrawString(gearW.c_str(), -1, &rowFont, gearRectF, &leftAlign, &textBrushRow);

                std::string displayKey = keyNameStr;
                if (input.type == MOUSE)
                    displayKey += " (Mouse)";
                else if (input.type == KEYBOARD)
                    displayKey += " (Keyboard)";
                else if (input.type == VJOY_BUTTON)
                    displayKey += " (vJoy)";

                std::wstring keyW(displayKey.begin(), displayKey.end());
                RectF keyRectF((REAL)panelRect.X + 80, (REAL)(y + rowPadding / 2), (REAL)panelWidth - 90, (REAL)(rowHeight - rowPadding));
                graphics.DrawString(keyW.c_str(), -1, &rowFont, keyRectF, &leftAlign, &textBrushRow);

                y += rowHeight + rowSpacing;
            }
            rightPanelContentBottom = max(rightPanelContentBottom, y);
            y += 10; // extra spacing before next section
        }

        // --- INPUT PANEL ---
        if (showInputPanel)
        {
            int rectWidth = 320;
            inputPanelRectUnified = RectF((REAL)panelRect.X, (REAL)y, (REAL)rectWidth, 220);

            int yInput = (int)inputPanelRectUnified.Y + 10;

            RectF subtitleRect(inputPanelRectUnified.X, (REAL)yInput, (REAL)rectWidth, 20);
            graphics.DrawString(L"Keys & Mouse → vJoy (Shifter Togglers)", -1, &subtitleFont, subtitleRect, &leftAlign, &subtitleBrush);

            yInput += 28;

            StringFormat rowTextFormat;
            rowTextFormat.SetAlignment(StringAlignmentNear);
            rowTextFormat.SetLineAlignment(StringAlignmentCenter);

            for (size_t i = 0; i < inputMap.size(); ++i)
            {
                InputToVJoy& input = inputMap[i];
                std::wstring keyName;

                // Determine key name
                if (input.type == KEYBOARD)
                {
                    wchar_t buffer[64];
                    if (GetKeyNameTextW(MapVirtualKeyW(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyName = buffer;
                }
                else if (input.type == MOUSE)
                {
                    switch (input.code)
                    {
                    case 1:
                        keyName = L"LMB";
                        break;
                    case 2:
                        keyName = L"RMB";
                        break;
                    case 3:
                        keyName = L"MMB";
                        break;
                    default:
                        keyName = L"Mouse " + std::to_wstring(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON)
                {
                    keyName = L"vJoy Btn " + std::to_wstring(input.code);
                }

                // Row background
                Color rowColor = (inputBeingSet == std::to_string(i)) ? Color(0, 255, 136) : Color(36, 36, 36);
                SolidBrush rowBrush(rowColor);
                RectF rowRect((REAL)inputPanelRectUnified.X + leftMargin, (REAL)yInput, (REAL)keyColumnWidth, (REAL)rowHeight);
                graphics.FillRectangle(&rowBrush, rowRect);

                // Combined text centered
                std::wstring combinedText = keyName + L" → vJoy " + std::to_wstring(input.vjoyButton);
                StringFormat centerFormat;
                centerFormat.SetAlignment(StringAlignmentCenter);
                centerFormat.SetLineAlignment(StringAlignmentCenter);

                graphics.DrawString(combinedText.c_str(), -1, &rowFont, rowRect, &centerFormat, &textBrush);

                yInput += rowHeight + rowSpacing;
            }

            y = yInput + 10;
            rightPanelContentBottom = max(rightPanelContentBottom, y);
        }

        // --- TOGGLE PANEL ---
        // --- TOGGLE PANEL ---
        if (showTogglePanel)
        {
            int rectWidth = 320;
            togglePanelRectUnified = Rect(panelRect.X, y, panelRect.X + rectWidth, y + 200); // store globally

            int yToggle = togglePanelRectUnified.Y + 10;

            // Knob Toggle
            RectF knobSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Activate Knob Key", -1, &subtitleFont, knobSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush1((toggleInputBeingSet || togglePedalBeingSet) ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect1F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush1, keyRect1F);

            g_toggleKeyRect.left = (LONG)keyRect1F.X;
            g_toggleKeyRect.top = (LONG)keyRect1F.Y;
            g_toggleKeyRect.right = (LONG)(keyRect1F.X + keyRect1F.Width);
            g_toggleKeyRect.bottom = (LONG)(keyRect1F.Y + keyRect1F.Height);

            wchar_t keyName1[64] = L"";
            if (toggleInputBeingSet)
            {
                wcscpy_s(keyName1, L"Press any key...");
            }
            else if (togglePedalBeingSet)
            {
                wcscpy_s(keyName1, L"Press a pedal...");
            }
            else
            {
                switch (g_knobToggleType)
                {
                case TOGGLE_KEYBOARD:
                    if (g_knobToggleKey != 0 &&
                        GetKeyNameTextW(MapVirtualKeyW(g_knobToggleKey, MAPVK_VK_TO_VSC) << 16,
                            keyName1, 64) == 0)
                    {
                        wcscpy_s(keyName1, L"Unknown Key");
                    }
                    else if (g_knobToggleKey == 0)
                    {
                        wcscpy_s(keyName1, L"Not Set");
                    }
                    break;

                    // ✅ Add mouse buttons
                case TOGGLE_MOUSE_LEFT:
                    wcscpy_s(keyName1, L"Mouse: Left Button");
                    break;
                case TOGGLE_MOUSE_RIGHT:
                    wcscpy_s(keyName1, L"Mouse: Right Button");
                    break;
                case TOGGLE_MOUSE_MIDDLE:
                    wcscpy_s(keyName1, L"Mouse: Middle Button");
                    break;
                case TOGGLE_MOUSE_BUTTON4:
                    wcscpy_s(keyName1, L"Mouse: Button 4");
                    break;
                case TOGGLE_MOUSE_BUTTON5:
                    wcscpy_s(keyName1, L"Mouse: Button 5");
                    break;

                    // Existing pedals
                case TOGGLE_PEDAL_CLUTCH:
                    wcscpy_s(keyName1, L"Pedal: Clutch");
                    break;
                case TOGGLE_PEDAL_BRAKE:
                    wcscpy_s(keyName1, L"Pedal: Brake");
                    break;
                case TOGGLE_PEDAL_ACCEL:
                    wcscpy_s(keyName1, L"Pedal: Accelerator");
                    break;
                }
            }



            graphics.DrawString(keyName1, -1, &rowFont, keyRect1F, &centerAlign, &textBrush);

            yToggle += rowHeight + rowSpacing;

            // Assist Button
            RectF assistSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Knob Assist Button (Controller)", -1, &subtitleFont, assistSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush2(assistButtonBeingSet ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect2F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush2, keyRect2F);

            g_assistButtonRect.left = (LONG)keyRect2F.X;
            g_assistButtonRect.top = (LONG)keyRect2F.Y;
            g_assistButtonRect.right = (LONG)(keyRect2F.X + keyRect2F.Width);
            g_assistButtonRect.bottom = (LONG)(keyRect2F.Y + keyRect2F.Height);

            wchar_t keyName2[64] = L"";

            if (assistButtonBeingSet) {
                wcscpy_s(keyName2, L"Press any button...");
            }
            else if (g) {
                switch ((SDL_GameControllerButton)assistButton) {
                case SDL_CONTROLLER_BUTTON_A:        wcscpy_s(keyName2, L"A Button"); break;
                case SDL_CONTROLLER_BUTTON_B:        wcscpy_s(keyName2, L"B Button"); break;
                case SDL_CONTROLLER_BUTTON_X:        wcscpy_s(keyName2, L"X Button"); break;
                case SDL_CONTROLLER_BUTTON_Y:        wcscpy_s(keyName2, L"Y Button"); break;
                case SDL_CONTROLLER_BUTTON_BACK:     wcscpy_s(keyName2, L"Back"); break;
                case SDL_CONTROLLER_BUTTON_GUIDE:    wcscpy_s(keyName2, L"Guide"); break;
                case SDL_CONTROLLER_BUTTON_START:    wcscpy_s(keyName2, L"Start"); break;
                case SDL_CONTROLLER_BUTTON_LEFTSTICK: wcscpy_s(keyName2, L"L3"); break;
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK: wcscpy_s(keyName2, L"R3"); break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  wcscpy_s(keyName2, L"LB / L1"); break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: wcscpy_s(keyName2, L"RB / R1"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:    wcscpy_s(keyName2, L"DPad Up"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  wcscpy_s(keyName2, L"DPad Down"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  wcscpy_s(keyName2, L"DPad Left"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: wcscpy_s(keyName2, L"DPad Right"); break;
                default:                               wcscpy_s(keyName2, L"Unknown Button"); break;
                }
            }
            else {
                wcscpy_s(keyName2, L"None");
            }

            graphics.DrawString(keyName2, -1, &rowFont, keyRect2F, &centerAlign, &textBrush);

            yToggle += rowHeight + rowSpacing;

            // Reverse Unlock Button
            RectF reverseSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Reverse Unlock Button", -1, &subtitleFont, reverseSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush3(reverseUnlockBeingSet ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect3F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush3, keyRect3F);

            reverseUnlockKeyRect.left = (LONG)keyRect3F.X;
            reverseUnlockKeyRect.top = (LONG)keyRect3F.Y;
            reverseUnlockKeyRect.right = (LONG)(keyRect3F.X + keyRect3F.Width);
            reverseUnlockKeyRect.bottom = (LONG)(keyRect3F.Y + keyRect3F.Height);

            wchar_t keyName3[64] = L"";

            if (reverseUnlockBeingSet) {
                wcscpy_s(keyName3, L"Press any key/button...");
            }
            else {
                switch (g_reverseUnlockType)
                {
                case TOGGLE_KEYBOARD:
                    if (g_reverseUnlockKey != 0 &&
                        GetKeyNameTextW(MapVirtualKeyW(g_reverseUnlockKey, MAPVK_VK_TO_VSC) << 16,
                            keyName3, 64) == 0)
                    {
                        wcscpy_s(keyName3, L"Unknown Key");
                    }
                    else if (g_reverseUnlockKey == 0)
                    {
                        wcscpy_s(keyName3, L"Not Set");
                    }
                    break;
                case TOGGLE_MOUSE_LEFT: wcscpy_s(keyName3, L"Mouse: Left Button"); break;
                case TOGGLE_MOUSE_RIGHT: wcscpy_s(keyName3, L"Mouse: Right Button"); break;
                case TOGGLE_MOUSE_MIDDLE: wcscpy_s(keyName3, L"Mouse: Middle Button"); break;
                case TOGGLE_MOUSE_BUTTON4: wcscpy_s(keyName3, L"Mouse: Button 4"); break;
                case TOGGLE_MOUSE_BUTTON5: wcscpy_s(keyName3, L"Mouse: Button 5"); break;
                case TOGGLE_PEDAL_CLUTCH: wcscpy_s(keyName3, L"Pedal: Clutch"); break;
                case TOGGLE_PEDAL_BRAKE: wcscpy_s(keyName3, L"Pedal: Brake"); break;
                case TOGGLE_PEDAL_ACCEL: wcscpy_s(keyName3, L"Pedal: Accelerator"); break;
                }
            }

            graphics.DrawString(keyName3, -1, &rowFont, keyRect3F, &centerAlign, &textBrush);
            yToggle += rowHeight + rowSpacing;
            rightPanelContentBottom = max(rightPanelContentBottom, yToggle);
        }

        // --- Update max scroll for right panel ---
        // Add 50px extra padding at the bottom so the last items aren't stuck at the bottom edge
        int bottomPadding = 50;
        rightPanelScrollMax = max(0, (rightPanelContentBottom + bottomPadding - rightPanelScrollOffset) - (panelRect.Y + panelRect.Height - 10));

        graphics.ResetClip();
    }

    if (showSettingsPanel)
    {
        using namespace Gdiplus;

        Graphics graphics(memDC);

        // --- Clip drawing to panel ---
        graphics.SetClip(Rect(
            settingsPanelRect.left,
            settingsPanelRect.top,
            settingsPanelRect.right - settingsPanelRect.left,
            settingsPanelRect.bottom - settingsPanelRect.top));

        // --- Smooth scroll toward target ---
        settingsScrollOffsetF += (settingsScrollTarget - settingsScrollOffsetF) * settingsScrollSpeed;
        settingsScrollOffset = (int)settingsScrollOffsetF;

        // --- Fonts & brushes ---
        FontFamily fontFamily(L"Segoe UI");
        Font titleFont(&fontFamily, 22, FontStyleBold, UnitPixel);
        Font rowFont(&fontFamily, 16, FontStyleRegular, UnitPixel);
        SolidBrush titleBrush(Color(0, 255, 136));
        SolidBrush labelBrush(Color(220, 220, 220));
        SolidBrush valueBrush(Color(200, 200, 200));

        int verticalSpacing = 88;    // spacing between sliders/toggles
        int nameControlSpacing = 32; // spacing between name and slider/control
        int startY = 90;
        int sliderHeight = 20;
        int toggleHeight = 30;
        int comboHeight = 28;

        // --- Panel Title ---
        RectF titleRectF(
            (REAL)settingsPanelRect.left,
            (REAL)(settingsPanelRect.top + 10 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left),
            30.0f);
        StringFormat titleFormat;
        titleFormat.SetAlignment(StringAlignmentCenter);
        titleFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(L"Settings", -1, &titleFont, titleRectF, &titleFormat, &titleBrush);

        // --- Slider Helper ---
        auto DrawSlider = [&](RECT rect, float t, const char* label, const char* valueText)
        {
            RectF sliderRectF((REAL)rect.left, (REAL)rect.top + settingsScrollOffset, (REAL)(rect.right - rect.left), (REAL)(rect.bottom - rect.top));

            // Slider background
            SolidBrush bgBrush(Color(50, 50, 50));
            graphics.FillRectangle(&bgBrush, sliderRectF);

            // Slider knob
            float knobX = rect.left + t * (rect.right - rect.left);
            RectF knobRect(knobX - 5, (REAL)rect.top + settingsScrollOffset, 10.0f, (REAL)(rect.bottom - rect.top));
            SolidBrush knobBrush(Color(0, 255, 136));
            graphics.FillRectangle(&knobBrush, knobRect);

            // Label
            PointF labelPos((REAL)rect.left, (REAL)(rect.top - nameControlSpacing + settingsScrollOffset));
            graphics.DrawString(std::wstring(label, label + strlen(label)).c_str(), -1, &rowFont, labelPos, &labelBrush);

            // Value
            StringFormat valueFormat;
            valueFormat.SetAlignment(StringAlignmentCenter);
            PointF valuePos((REAL)(rect.left + (rect.right - rect.left) / 2), (REAL)(rect.bottom + 2 + settingsScrollOffset));
            graphics.DrawString(std::wstring(valueText, valueText + strlen(valueText)).c_str(), -1, &rowFont, valuePos, &valueFormat, &valueBrush);
        };

        // --- Draw sliders ---
        char valueBuffer[16];

        // Knob Radius
// Knob Radius
        float tKnob = float(knobRadius - knobRadiusMin) / float(knobRadiusMax - knobRadiusMin);
        sprintf_s(valueBuffer, "%d", knobRadius);
        knobSliderRect = { settingsPanelRect.left + 20, startY, settingsPanelRect.right - 20, startY + sliderHeight };
        DrawSlider(knobSliderRect, tKnob, "Knob Radius", valueBuffer);

        // --- Tooltip for Knob Radius ---
        tooltips[0].bounds = knobSliderRect;
        tooltips[0].text = L"Adjusts the size of the knob.";
        // Gear Radius
        float tGear = float(gearRadius - gearRadiusMin) / float(gearRadiusMax - gearRadiusMin);
        sprintf_s(valueBuffer, "%d", gearRadius);
        gearSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing, settingsPanelRect.right - 20, startY + verticalSpacing + sliderHeight };
        DrawSlider(gearSliderRect, tGear, "Gear Radius", valueBuffer);

        tooltips[1].bounds = gearSliderRect;
        tooltips[1].text = L"Adjusts the size of gears.";
        // Sensitivity
        float tSens = (knobSensitivity - knobSensitivityMin) / (knobSensitivityMax - knobSensitivityMin);
        sprintf_s(valueBuffer, "%.2f", knobSensitivity);
        sensSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 2, settingsPanelRect.right - 20, startY + verticalSpacing * 2 + sliderHeight };
        DrawSlider(sensSliderRect, tSens, "Knob Sensitivity", valueBuffer);
        tooltips[2].bounds = sensSliderRect;
        tooltips[2].text = L"Adjusts mouse sensitivity for knob movement.";
        // Diagonal Assist
        float tDiag = (diagonalAssist - diagMin) / (diagMax - diagMin);
        sprintf_s(valueBuffer, "%.2f", diagonalAssist);
        diagSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 3, settingsPanelRect.right - 20, startY + verticalSpacing * 3 + sliderHeight };
        DrawSlider(diagSliderRect, tDiag, "Diagonal Assist Strength", valueBuffer);
        tooltips[3].bounds = diagSliderRect;
        tooltips[3].text = L"Adjusts assistance for diagonal gear shifts. Higher values may not help with smaller H-shifter layout.";

        // Snap-In Threshold
        float tSnap = (gearSnapInMultiplier - snapInMin) / (snapInMax - snapInMin);
        sprintf_s(valueBuffer, "%.2f", gearSnapInMultiplier);
        snapInSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 4, settingsPanelRect.right - 20, startY + verticalSpacing * 4 + sliderHeight };
        DrawSlider(snapInSliderRect, tSnap, "Snap Sensitivity", valueBuffer);
        tooltips[4].bounds = snapInSliderRect;
        tooltips[4].text = L"Adjusts how easily gears snap into position.";

        // Snap Speed
        float tSnapSpeed = (snapSpeed - snapSpeedMin) / (snapSpeedMax - snapSpeedMin);
        sprintf_s(valueBuffer, "%.2f", snapSpeed);
        snapSpeedSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 5, settingsPanelRect.right - 20, startY + verticalSpacing * 5 + sliderHeight };
        DrawSlider(snapSpeedSliderRect, tSnapSpeed, "Snap Speed", valueBuffer);
        tooltips[5].bounds = snapSpeedSliderRect;
        tooltips[5].text = L"Adjusts how fast the knob snaps to gears. Lower for smooth transitions, higher for instant teleport.";
        // Layout Scale
        float tLayout = (layoutScale - 1.0f) / (3.0f - 1.0f);
        sprintf_s(valueBuffer, "%.2f", layoutScale);
        layoutScaleSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 6, settingsPanelRect.right - 20, startY + verticalSpacing * 6 + sliderHeight };
        DrawSlider(layoutScaleSliderRect, tLayout, "H-Shifter Size", valueBuffer);
        tooltips[6].bounds = layoutScaleSliderRect;
        tooltips[6].text = L"Adjusts the overall size of H-shifter layout.";
        // --- Subtle Line Above H-Shifter Settings ---
        int headingY = startY + verticalSpacing * 7 - 15; // Position above Neutral Toggle
        int lineY = headingY - 10; // 10 pixels above the heading
        Pen linePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&linePen,
            settingsPanelRect.left + 40, lineY + settingsScrollOffset,
            settingsPanelRect.right - 40, lineY + settingsScrollOffset);

        // --- H-Shifter Settings Heading ---
        PointF headingPos((REAL)(settingsPanelRect.left), (REAL)(headingY + settingsScrollOffset));
        SolidBrush headingBrush(Color(180, 180, 180)); // Subtle gray-white color
        Font headingFont(&fontFamily, 16, FontStyleRegular, UnitPixel); // Regular weight, normal size
        StringFormat headingFormat;
        headingFormat.SetAlignment(StringAlignmentCenter);
        headingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF headingRect((REAL)settingsPanelRect.left, (REAL)(headingY + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"H-Shifter Settings", -1, &headingFont, headingRect, &headingFormat, &headingBrush);
        // --- Neutral Toggle ---
        int toggleY = startY + verticalSpacing * 7 + 25;
        neutralToggleRect = { settingsPanelRect.left + 20, toggleY, settingsPanelRect.right - 20, toggleY + toggleHeight };
        PointF toggleLabelPos((REAL)neutralToggleRect.left, (REAL)neutralToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Enable Neutral", -1, &rowFont, toggleLabelPos, &labelBrush);
        // --- Tooltip for Enable Neutral (EXCEPTIONAL - 35px below) ---
// --- Tooltip for Enable Neutral (EXCEPTIONAL - 35px below) ---
        tooltips[7].bounds = neutralToggleRect;
        tooltips[7].text = L"Enables neutral key binding. When knob is in neutral position, sends a neutral key press for games that require explicit neutral binding.";

        RectF checkboxRect(neutralToggleRect.left + 220, neutralToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush boxBrush(isNeutralEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&boxBrush, checkboxRect);
        Pen boxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&boxPen, checkboxRect);
        // --- No Reverse Layout Toggle ---

        // --- Hide High Gears Toggle ---
        // --- Hide High Gears Toggle ---
        int hideHighGearsY = neutralToggleRect.bottom + 10; // stack below noReverse
        hideHighGearsToggleRect = { settingsPanelRect.left + 20, hideHighGearsY, settingsPanelRect.right - 20, hideHighGearsY + toggleHeight };

        PointF hideHighGearsLabelPos((REAL)hideHighGearsToggleRect.left, (REAL)hideHighGearsToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Hide High Gears", -1, &rowFont, hideHighGearsLabelPos, &labelBrush);
        tooltips[8].bounds = hideHighGearsToggleRect;
        tooltips[8].text = L"Hides high gear labeling";
        // Checkbox
        RectF hideHighGearsCheckboxRect(hideHighGearsToggleRect.left + 220, hideHighGearsToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush hideHighGearsBoxBrush(hideHighGears ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&hideHighGearsBoxBrush, hideHighGearsCheckboxRect);
        graphics.DrawRectangle(&boxPen, hideHighGearsCheckboxRect);

        // --- Precision Knob Movement Toggle (between Hide High Gears and Dynamic Transparency) ---
        int knobAccelY = hideHighGearsToggleRect.bottom + 10; // 15px gap
        knobAccelToggleRect = { settingsPanelRect.left + 20, knobAccelY, settingsPanelRect.right - 20, knobAccelY + toggleHeight };

        PointF knobLabelPos((REAL)knobAccelToggleRect.left, (REAL)knobAccelToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Precision Knob Movement", -1, &rowFont, knobLabelPos, &labelBrush);

        // Checkbox rectangle (inverted: checked = disabled, unchecked = enabled)
        RectF knobCheckboxRect(knobAccelToggleRect.left + 220, knobAccelToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush knobBoxBrush(!knobAccelerationEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&knobBoxBrush, knobCheckboxRect);
        Pen boxPen1(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&boxPen1, knobCheckboxRect);
        tooltips[9].bounds = knobAccelToggleRect;
        tooltips[9].text = L"Enables precision mode for mouse knob movement. When enabled, knob movement is more precise and controlled.";
        // --- Realistic Knob Drawing Toggle ---
        int realisticKnobY = knobAccelToggleRect.bottom + 10; // 10px gap
        realisticKnobToggleRect = { settingsPanelRect.left + 20, realisticKnobY, settingsPanelRect.right - 20, realisticKnobY + toggleHeight };

        PointF realisticKnobLabelPos((REAL)realisticKnobToggleRect.left, (REAL)realisticKnobToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Fancy Knob Mode", -1, &rowFont, realisticKnobLabelPos, &labelBrush);

        // Checkbox
        RectF realisticKnobCheckboxRect(realisticKnobToggleRect.left + 220, realisticKnobToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush realisticKnobBoxBrush(realisticKnob ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&realisticKnobBoxBrush, realisticKnobCheckboxRect);
        graphics.DrawRectangle(&boxPen, realisticKnobCheckboxRect);

        tooltips[10].bounds = realisticKnobToggleRect;
        tooltips[10].text = L"Make your knob look extra fancy";

        // --- Clutch Lock Gear Toggle ---
        int clutchLockY = realisticKnobToggleRect.bottom + 10;
        clutchLockGearToggleRect = { settingsPanelRect.left + 20, clutchLockY, settingsPanelRect.right - 20, clutchLockY + toggleHeight };

        PointF clutchLockLabelPos((REAL)clutchLockGearToggleRect.left, (REAL)clutchLockGearToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Clutch Lock Gear", -1, &rowFont, clutchLockLabelPos, &labelBrush);

        RectF clutchLockCheckboxRect(clutchLockGearToggleRect.left + 220, clutchLockGearToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush clutchLockBoxBrush(clutchLockGear ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&clutchLockBoxBrush, clutchLockCheckboxRect);
        graphics.DrawRectangle(&boxPen, clutchLockCheckboxRect);

        // Tooltip for Clutch Lock
        tooltips[13].bounds = clutchLockGearToggleRect;
        tooltips[13].text = L"Prevents leaving/entering gears without clutch engagement.";

        // --- Reverse Gear Lock Toggle ---
        int reverseLockY = clutchLockGearToggleRect.bottom + 10;
        reverseLockToggleRect = { settingsPanelRect.left + 20, reverseLockY, settingsPanelRect.right - 20, reverseLockY + toggleHeight };

        PointF reverseLockLabelPos((REAL)reverseLockToggleRect.left, (REAL)reverseLockToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Reverse Gear Lock", -1, &rowFont, reverseLockLabelPos, &labelBrush);

        RectF reverseLockCheckboxRect(reverseLockToggleRect.left + 220, reverseLockToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush reverseLockBoxBrush(reverseLockEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&reverseLockBoxBrush, reverseLockCheckboxRect);
        graphics.DrawRectangle(&boxPen, reverseLockCheckboxRect);

        tooltips[20].bounds = reverseLockToggleRect;
        tooltips[20].text = L"Prevents accidental reverse engagement unless the unlock key is held.";

        // --- H-Shifter Layout ---
        // Start right after reverseLockToggleRect
        int hShifterLayoutY = reverseLockToggleRect.bottom + 40;
        hShifterLayoutButtonRect = { settingsPanelRect.left + 20, hShifterLayoutY, settingsPanelRect.right - 20, hShifterLayoutY + toggleHeight };

        PointF hShifterLayoutLabelPos((REAL)hShifterLayoutButtonRect.left, (REAL)(hShifterLayoutButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"H-Shifter Layout:", -1, &rowFont, hShifterLayoutLabelPos, &labelBrush);
        tooltips[11].bounds = hShifterLayoutButtonRect;
        tooltips[11].text = L"Select the physical layout and number of gear positions for your H-shifter.";
        RectF hShifterLayoutBoxRect(
            (REAL)hShifterLayoutButtonRect.left,
            (REAL)(hShifterLayoutButtonRect.top + settingsScrollOffset),
            (REAL)(hShifterLayoutButtonRect.right - hShifterLayoutButtonRect.left),
            (REAL)hShifterLayoutButtonRect.bottom - hShifterLayoutButtonRect.top
        );
        SolidBrush layoutBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&layoutBoxBrush, hShifterLayoutBoxRect);
        Pen layoutBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&layoutBoxPen, hShifterLayoutBoxRect);

        std::wstring hShifterLayoutName = hShifterLayouts[currentHShifterLayout - 1].name;
        PointF hShifterLayoutValuePos(hShifterLayoutBoxRect.X + 4, hShifterLayoutBoxRect.Y + 4);
        graphics.DrawString(hShifterLayoutName.c_str(), -1, &rowFont, hShifterLayoutValuePos, &valueBrush);

        PointF hShifterArrowPos(hShifterLayoutBoxRect.X + (hShifterLayoutBoxRect.Width - 20), hShifterLayoutBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, hShifterArrowPos, &valueBrush);

        // --- Gear Layout Cycle Button ---
        const int spacingAfterHShifter = 50;
        int layoutButtonY = hShifterLayoutY + toggleHeight + spacingAfterHShifter;
        gearLayoutButtonRect = { settingsPanelRect.left + 20, layoutButtonY, settingsPanelRect.right - 20, layoutButtonY + toggleHeight };

        PointF layoutLabelPos((REAL)gearLayoutButtonRect.left, (REAL)(gearLayoutButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"Gear Label Layout:", -1, &rowFont, layoutLabelPos, &labelBrush);
        tooltips[12].bounds = gearLayoutButtonRect;
        tooltips[12].text = L"Select the visual theme and styling for gear number labels and text display.";
        RectF layoutBoxRect(
            (REAL)gearLayoutButtonRect.left,
            (REAL)(gearLayoutButtonRect.top + settingsScrollOffset),
            (REAL)(gearLayoutButtonRect.right - gearLayoutButtonRect.left),
            (REAL)gearLayoutButtonRect.bottom - gearLayoutButtonRect.top
        );
        graphics.FillRectangle(&layoutBoxBrush, layoutBoxRect);
        graphics.DrawRectangle(&layoutBoxPen, layoutBoxRect);

        std::wstring layoutName = gearLayoutNames[currentGearLayout];
        PointF layoutValuePos(layoutBoxRect.X + 4, layoutBoxRect.Y + 4);
        graphics.DrawString(layoutName.c_str(), -1, &rowFont, layoutValuePos, &valueBrush);

        PointF arrowPos(layoutBoxRect.X + (layoutBoxRect.Width - 20), layoutBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, arrowPos, &valueBrush);
        // --- Profile Selector ---
        const int spacingAfterGearLayout = 50; // Updated spacing as requested
        int profileSelectorY = layoutButtonY + toggleHeight + spacingAfterGearLayout;
        profileButtonRect = { settingsPanelRect.left + 20, profileSelectorY, settingsPanelRect.right - 20, profileSelectorY + toggleHeight };

        PointF profileLabelPos((REAL)profileButtonRect.left, (REAL)(profileButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"Profile:", -1, &rowFont, profileLabelPos, &labelBrush);
        tooltips[13].bounds = profileButtonRect;
        tooltips[13].text = L"Select or create configuration profiles for different games or setups.";

        RectF profileBoxRect(
            (REAL)profileButtonRect.left,
            (REAL)(profileButtonRect.top + settingsScrollOffset),
            (REAL)(profileButtonRect.right - profileButtonRect.left),
            (REAL)profileButtonRect.bottom - profileButtonRect.top
        );
        SolidBrush profileBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&profileBoxBrush, profileBoxRect);
        Pen profileBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&profileBoxPen, profileBoxRect);

        // Display current profile name or new profile name being typed
        std::string currentProfileDisplay;
        if (creatingNewProfile) {
            currentProfileDisplay = newProfileName; // Show typing in dropdown box
        }
        else if (!profileNames.empty()) {
            currentProfileDisplay = profileNames[currentProfileIndex];
            // Remove .ini extension for display
            if (currentProfileDisplay.size() > 4 && currentProfileDisplay.substr(currentProfileDisplay.size() - 4) == ".ini") {
                currentProfileDisplay = currentProfileDisplay.substr(0, currentProfileDisplay.size() - 4);
            }
        }
        else {
            currentProfileDisplay = "No Profiles";
        }

        std::wstring profileDisplayW(currentProfileDisplay.begin(), currentProfileDisplay.end());
        PointF profileValuePos(profileBoxRect.X + 4, profileBoxRect.Y + 4);

        if (creatingNewProfile) {
            // Draw the text input field with selection in the DROPDOWN BOX
            std::wstring profileNameW(newProfileName.begin(), newProfileName.end());

            // Use StringFormat for consistent text rendering
            StringFormat format;
            format.SetAlignment(StringAlignmentNear);
            format.SetLineAlignment(StringAlignmentNear);
            format.SetFormatFlags(StringFormatFlagsNoClip | StringFormatFlagsNoWrap);

            // Measure the entire string for height
            RectF fullTextBounds;
            graphics.MeasureString(profileNameW.c_str(), -1, &rowFont, PointF(0, 0), &format, &fullTextBounds);

            // Draw selection background if text is selected
            if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                // Measure text before selection using CharacterRanges for precise measurement
                CharacterRange ranges[2];
                ranges[0] = CharacterRange(0, profileTextSelectionStart);
                ranges[1] = CharacterRange(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);

                StringFormat measureFormat;
                measureFormat.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces);
                measureFormat.SetMeasurableCharacterRanges(2, ranges);

                RectF measureRect(0, 0, 10000, 10000); // Large enough rectangle for measurement
                Region regions[2];
                graphics.MeasureCharacterRanges(profileNameW.c_str(), -1, &rowFont, measureRect, &measureFormat, 2, regions);

                // Get bounds for selection region
                RectF selectionBounds;
                regions[1].GetBounds(&selectionBounds, &graphics);

                // Draw selection highlight at precise position
                RectF selectionRect(
                    profileBoxRect.X + 4 + selectionBounds.X,
                    profileBoxRect.Y + 4,
                    selectionBounds.Width,
                    fullTextBounds.Height
                );
                SolidBrush selectionBrush(Color(0, 120, 215));
                graphics.FillRectangle(&selectionBrush, selectionRect);
            }

            // Draw the text in DROPDOWN BOX
            SolidBrush textBrush(Color(255, 255, 255));
            graphics.DrawString(profileNameW.c_str(), -1, &rowFont, profileValuePos, &format, &textBrush);

            // Draw blinking cursor in DROPDOWN BOX if no selection
            if (!profileTextSelected || profileTextSelectionStart == profileTextSelectionEnd) {
                static DWORD lastBlink = GetTickCount();
                static bool cursorVisible = true;

                DWORD currentTime = GetTickCount();
                if (currentTime - lastBlink > 500) {
                    cursorVisible = !cursorVisible;
                    lastBlink = currentTime;
                }

                if (cursorVisible) {
                    // Calculate cursor position using precise CharacterRanges measurement
                    if (profileTextSelectionStart >= 0) {
                        CharacterRange range(0, profileTextSelectionStart);
                        StringFormat cursorFormat;
                        cursorFormat.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces);
                        cursorFormat.SetMeasurableCharacterRanges(1, &range);

                        RectF measureRect(0, 0, 10000, 10000);
                        Region regions[1];
                        graphics.MeasureCharacterRanges(profileNameW.c_str(), -1, &rowFont, measureRect, &cursorFormat, 1, regions);

                        RectF bounds;
                        if (regions[0].GetBounds(&bounds, &graphics) == Ok) {
                            REAL cursorX = profileBoxRect.X + 4 + bounds.GetRight();
                            REAL cursorY = profileBoxRect.Y + 4;
                            REAL cursorHeight = fullTextBounds.Height > 0 ? fullTextBounds.Height : rowFont.GetHeight(&graphics);

                            RectF cursorRect(cursorX, cursorY, 2, cursorHeight);
                            SolidBrush cursorBrush(Color(255, 255, 255));
                            graphics.FillRectangle(&cursorBrush, cursorRect);
                        }
                    }
                    else {
                        // Fallback for cursor at start (no text)
                        REAL cursorHeight = fullTextBounds.Height > 0 ? fullTextBounds.Height : rowFont.GetHeight(&graphics);
                        RectF cursorRect(profileBoxRect.X + 4, profileBoxRect.Y + 4, 2, cursorHeight);
                        SolidBrush cursorBrush(Color(255, 255, 255));
                        graphics.FillRectangle(&cursorBrush, cursorRect);
                    }
                }
            }
        }
        else {
            // Normal profile display in dropdown box
            graphics.DrawString(profileDisplayW.c_str(), -1, &rowFont, profileValuePos, &valueBrush);
        }

        PointF profileArrowPos(profileBoxRect.X + (profileBoxRect.Width - 20), profileBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, profileArrowPos, &valueBrush);



        // --- Create New Profile Button (separate button below dropdown) ---
        int createProfileButtonY = profileSelectorY + toggleHeight + 10; // 10px spacing after profile selector
        createProfileButtonRect = { settingsPanelRect.left + 20, createProfileButtonY, settingsPanelRect.right - 20, createProfileButtonY + toggleHeight };

        RectF createProfileBoxRect(
            (REAL)createProfileButtonRect.left,
            (REAL)(createProfileButtonRect.top + settingsScrollOffset),
            (REAL)(createProfileButtonRect.right - createProfileButtonRect.left),
            (REAL)createProfileButtonRect.bottom - createProfileButtonRect.top
        );

        // Different color for create button to make it stand out
        SolidBrush createProfileBoxBrush(Color(60, 60, 60));
        graphics.FillRectangle(&createProfileBoxBrush, createProfileBoxRect);
        Pen createProfileBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&createProfileBoxPen, createProfileBoxRect);

        // ALWAYS show instruction text in create button, never show the typing
        std::wstring createButtonText = creatingNewProfile ? L"Type profile name and press Enter" : L"Create New Profile";

        PointF createButtonTextPos(createProfileBoxRect.X + 4, createProfileBoxRect.Y + 4);
        graphics.DrawString(createButtonText.c_str(), -1, &rowFont, createButtonTextPos, &valueBrush);

        // Update the deviceComboY position to be after profile selector

        // --- Mouse Device Selector ---
// --- Mouse Device Selector ---
        SolidBrush deviceBoxBrush(Color(30, 30, 30));
        Pen deviceBoxPen(Color(0, 255, 136), 2);

        // Update the deviceComboY position to be after create profile button
        const int spacingAfterProfile = 50;
        int deviceComboY = createProfileButtonY + toggleHeight + spacingAfterProfile;
        deviceComboRect = { settingsPanelRect.left + 20, deviceComboY, settingsPanelRect.right - 20, deviceComboY + comboHeight };

        PointF deviceLabelPos((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"H-Shifter Mouse Device:", -1, &rowFont, deviceLabelPos, &labelBrush);
        tooltips[14].bounds = deviceComboRect;
        tooltips[14].text = L"Select which mouse device controls the H-shifter knob movement.";
        RectF deviceBoxRect((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top + settingsScrollOffset),
            (REAL)(deviceComboRect.right - deviceComboRect.left), (REAL)(deviceComboRect.bottom - deviceComboRect.top));
        graphics.FillRectangle(&deviceBoxBrush, deviceBoxRect);
        graphics.DrawRectangle(&deviceBoxPen, deviceBoxRect);


        std::wstring selName = L"All Mice";
        if (g_selectedDevice)
        {
            for (auto& d : g_mouseDevices)
                if (d.hDevice == g_selectedDevice)
                    selName = d.name;
        }
        PointF selTextPos((REAL)deviceComboRect.left + 4, (REAL)deviceComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selName.c_str(), -1, &rowFont, selTextPos, &valueBrush);

        // --- Transparencies ---
        // Dynamic Transparency Toggle
        int dynTransY = deviceComboY + comboHeight + 50;
        dynamicTransparencyToggleRect = { settingsPanelRect.left + 20, dynTransY, settingsPanelRect.right - 20, dynTransY + toggleHeight };

        PointF dynTransLabelPos((REAL)dynamicTransparencyToggleRect.left, (REAL)dynamicTransparencyToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Smart Adaptive Transparency", -1, &rowFont, dynTransLabelPos, &labelBrush);
        tooltips[15].bounds = dynamicTransparencyToggleRect;
        tooltips[15].text = L"Automatically adjusts transparency based on background brightness when in overlay mode.";
        RectF dynTransCheckboxRect(dynamicTransparencyToggleRect.left + 220, dynamicTransparencyToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush dynTransBoxBrush(dynamicTransparencyEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&dynTransBoxBrush, dynTransCheckboxRect);
        graphics.DrawRectangle(&boxPen, dynTransCheckboxRect);

        // Max Alpha Slider
        int transSliderY = dynTransY + toggleHeight + 35;
        transparencySliderRect = { settingsPanelRect.left + 20, transSliderY, settingsPanelRect.right - 20, transSliderY + sliderHeight };
        float tAlpha = float(maxAlpha) / 255.0f;
        int alphaPercent = int(tAlpha * 100.0f);
        sprintf_s(valueBuffer, "%d%%", alphaPercent);
        DrawSlider(transparencySliderRect, tAlpha, "Transparency", valueBuffer);
        tooltips[16].bounds = transparencySliderRect;
        tooltips[16].text = L"Sets the transparency level when the overlay is active.";

        // Idle Transparency Slider
        int idleSliderY = transparencySliderRect.bottom + 55;
        minTransparencySliderRect = { settingsPanelRect.left + 20, idleSliderY, settingsPanelRect.right - 20, idleSliderY + sliderHeight };
        float tIdle = float(minAlpha) / 255.0f;
        int idlePercent = int(tIdle * 100.0f);
        sprintf_s(valueBuffer, "%d%%", idlePercent);
        DrawSlider(minTransparencySliderRect, tIdle, "Idle Transparency", valueBuffer);
        tooltips[17].bounds = minTransparencySliderRect;
        tooltips[17].text = L"Sets the minimum transparency when idle (requires Smart Adaptive Transparency to be enabled).";
        // Fade Delay Slider
// Fade Delay Slider
        int delaySliderY = minTransparencySliderRect.bottom + 55;
        transparencyFadeDelaySliderRect = { settingsPanelRect.left + 20, delaySliderY, settingsPanelRect.right - 20, delaySliderY + sliderHeight };
        float tDelay = float(transparencyFadeDelay) / 2000.0f;
        tDelay = max(0.0f, min(1.0f, tDelay));
        char delayValueBuffer[32];
        sprintf_s(delayValueBuffer, "%lu ms", transparencyFadeDelay);
        DrawSlider(transparencyFadeDelaySliderRect, tDelay, "Fade Delay", delayValueBuffer);
        tooltips[18].bounds = transparencyFadeDelaySliderRect;
        tooltips[18].text = L"Sets the delay before transparency fades to idle level after stopping H-shifter use.";
        // --- Performance Optimization Toggle ---
        int optimizationY = transparencyFadeDelaySliderRect.bottom + 40;
        optimizationToggleRect = { settingsPanelRect.left + 20, optimizationY, settingsPanelRect.right - 20, optimizationY + toggleHeight };

        PointF optimizationLabelPos((REAL)optimizationToggleRect.left, (REAL)optimizationToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Performance Mode", -1, &rowFont, optimizationLabelPos, &labelBrush);
        Font descFont(&fontFamily, 11, FontStyleRegular, UnitPixel); // Slightly bigger font
        tooltips[19].bounds = optimizationToggleRect;
        tooltips[19].text = L"Optimizes for maximum game performance. Highly Recommended.";

        RectF optimizationCheckboxRect(optimizationToggleRect.left + 220, optimizationToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush optimizationBoxBrush(!disableSmartRedraws ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&optimizationBoxBrush, optimizationCheckboxRect);
        graphics.DrawRectangle(&boxPen, optimizationCheckboxRect);


        // --- Subtle Line Below Device Selector / Transparencies ---
        int bottomLineY = optimizationToggleRect.bottom + 15; // Adjusted spacing
        Pen bottomLinePen(Color(80, 80, 80), 1.0f);
        graphics.DrawLine(&bottomLinePen,
            settingsPanelRect.left + 40, bottomLineY + settingsScrollOffset,
            settingsPanelRect.right - 40, bottomLineY + settingsScrollOffset);

        // --- Game Control Panel ---
        // Mouse Block / Process Selector
        const int gameControlSpacing = 40;
        int gameControlY = bottomLineY + gameControlSpacing;
        int processComboY = gameControlY + 25;

        processComboRect = { settingsPanelRect.left + 20, processComboY, settingsPanelRect.right - 20, processComboY + comboHeight };
        PointF processLabelPos((REAL)processComboRect.left, (REAL)(processComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"Block Mouse & Look:", -1, &rowFont, processLabelPos, &labelBrush);


        // Store label bounds for hover detection (instead of the "i" icon)
        RECT labelBoundsRect = {
            (int)processLabelPos.X,
            (int)processLabelPos.Y,
            (int)(processLabelPos.X + 200), // Approximate width of "Game Mouse Blocker:"
            (int)(processLabelPos.Y + 20)   // Approximate height
        };
        g_tooltipBounds = labelBoundsRect; // Use the same variable for hover detection


        // Display tooltip text if hovering


        // Continue with existing code...
        RectF processBoxRect((REAL)processComboRect.left, (REAL)(processComboRect.top + settingsScrollOffset),
            (REAL)(processComboRect.right - processComboRect.left), (REAL)(processComboRect.bottom - processComboRect.top));
        SolidBrush processBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&processBoxBrush, processBoxRect);
        Pen processBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&processBoxPen, processBoxRect);

        // Display selected game name
        std::wstring gameName = L"Click to Select Game";
        std::wstring exeName = L"";
        if (g_selectedProcessId != 0) {
            // Get window title for display
            std::wstring windowTitle = L"";
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                std::wstring* result = reinterpret_cast<std::wstring*>(lParam);
                DWORD windowProcessId;
                GetWindowThreadProcessId(hwnd, &windowProcessId);

                if (windowProcessId == g_selectedProcessId && IsWindowVisible(hwnd)) {
                    wchar_t title[256];
                    if (GetWindowTextW(hwnd, title, 256) > 0) {
                        *result = title;
                        // Filter out system windows
                        if (!result->empty() &&
                            *result != L"Default IME" &&
                            *result != L"MSCTFIME UI") {
                            return FALSE; // Found good title, stop
                        }
                    }
                }
                return TRUE; // Continue
                }, reinterpret_cast<LPARAM>(&windowTitle));

            // Get executable name
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe;
                pe.dwSize = sizeof(PROCESSENTRY32);
                if (Process32First(hSnapshot, &pe)) {
                    do {
                        if (pe.th32ProcessID == g_selectedProcessId) {
                            exeName = pe.szExeFile;
                            break;
                        }
                    } while (Process32Next(hSnapshot, &pe));
                }
                CloseHandle(hSnapshot);
            }

            // Build display name
            if (!windowTitle.empty() && windowTitle != L"Default IME" && windowTitle != L"MSCTFIME UI") {
                gameName = windowTitle + L" (" + exeName + L")";
            }
            else {
                gameName = exeName;
            }
        }

        // Draw the game name text
        RectF textRect((REAL)processComboRect.left + 4, (REAL)processComboRect.top + 4 + settingsScrollOffset,
            (REAL)(processComboRect.right - processComboRect.left - 8), (REAL)(processComboRect.bottom - processComboRect.top - 8));

        StringFormat format;
        format.SetFormatFlags(StringFormatFlagsNoWrap);
        format.SetTrimming(StringTrimmingEllipsisCharacter);
        graphics.DrawString(gameName.c_str(), -1, &rowFont, textRect, &format, &valueBrush);

        // Control buttons
// Control buttons - Two checkboxes instead of one button
        int buttonWidth = 125;
        int buttonHeight = 30;
        int buttonSpacing = 10;
        int buttonsY = processComboY + comboHeight + 15;

        // Mouse Block checkbox
        mouseBlockCheckboxRect = {
            settingsPanelRect.left + 20,
            buttonsY,
            settingsPanelRect.left + 20 + buttonWidth,
            buttonsY + buttonHeight
        };
        RectF mouseBlockRect((REAL)mouseBlockCheckboxRect.left, (REAL)(mouseBlockCheckboxRect.top + settingsScrollOffset),
            (REAL)(mouseBlockCheckboxRect.right - mouseBlockCheckboxRect.left), (REAL)(mouseBlockCheckboxRect.bottom - mouseBlockCheckboxRect.top));

        // XInput Block checkbox
        xinputBlockCheckboxRect = {
            settingsPanelRect.left + 20 + buttonWidth + buttonSpacing,
            buttonsY,
            settingsPanelRect.left + 20 + buttonWidth + buttonSpacing + buttonWidth,
            buttonsY + buttonHeight
        };
        RectF xinputBlockRect((REAL)xinputBlockCheckboxRect.left, (REAL)(xinputBlockCheckboxRect.top + settingsScrollOffset),
            (REAL)(xinputBlockCheckboxRect.right - xinputBlockCheckboxRect.left), (REAL)(xinputBlockCheckboxRect.bottom - xinputBlockCheckboxRect.top));

        // Draw Mouse Block checkbox
        SolidBrush mouseBlockBrush(g_mouseBlockEnabled ? Color(0, 255, 136) : Color(60, 60, 60));
        graphics.FillRectangle(&mouseBlockBrush, mouseBlockRect);
        graphics.DrawRectangle(&processBoxPen, mouseBlockRect);

        // Draw XInput Block checkbox  
        SolidBrush xinputBlockBrush(g_xinputBlockEnabled ? Color(0, 255, 136) : Color(60, 60, 60));
        graphics.FillRectangle(&xinputBlockBrush, xinputBlockRect);
        graphics.DrawRectangle(&processBoxPen, xinputBlockRect);

        // Centered text for both checkboxes
        std::wstring mouseBlockText = g_mouseBlockEnabled ? L"Mouse Blocked" : L"Mouse Free";
        std::wstring xinputBlockText = g_xinputBlockEnabled ? L"XInput Blocked" : L"XInput Free";

        RectF mouseBlockTextRect = mouseBlockRect;
        RectF xinputBlockTextRect = xinputBlockRect;

        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);

        graphics.DrawString(mouseBlockText.c_str(), -1, &rowFont, mouseBlockTextRect, &centerFormat, &valueBrush);
        graphics.DrawString(xinputBlockText.c_str(), -1, &rowFont, xinputBlockTextRect, &centerFormat, &valueBrush);

        // Subtle instruction text
        PointF instructionPos((REAL)mouseBlockCheckboxRect.left, (REAL)(mouseBlockCheckboxRect.bottom + 5 + settingsScrollOffset));
        SolidBrush instructionBrush(Color(150, 150, 150));
        Font smallFont(&fontFamily, 10, FontStyleRegular, UnitPixel);
        graphics.DrawString(L"Hold RMB to use mouse", -1, &smallFont, instructionPos, &instructionBrush);
        int mouseSteeringY = buttonsY + 130; // 30px gap after injection buttons

        // --- Subtle Line and Heading Above Mouse Steering ---
        int mouseHeadingY = mouseSteeringY - 45; // Position above the toggle
        Pen mouseLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&mouseLinePen,
            settingsPanelRect.left + 40, mouseHeadingY + settingsScrollOffset,
            settingsPanelRect.right - 40, mouseHeadingY + settingsScrollOffset);

        PointF mouseHeadingPos((REAL)(settingsPanelRect.left), (REAL)(mouseHeadingY + 15 + settingsScrollOffset));
        SolidBrush mouseHeadingBrush(Color(180, 180, 180)); // Subtle gray-white color
        StringFormat mouseHeadingFormat;
        mouseHeadingFormat.SetAlignment(StringAlignmentCenter);
        mouseHeadingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF mouseHeadingRect((REAL)settingsPanelRect.left, (REAL)(mouseHeadingY + 5 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Mouse Steering", -1, &rowFont, mouseHeadingRect, &mouseHeadingFormat, &mouseHeadingBrush);

        // --- Mouse Steering Toggle ---
        mouseSteeringToggleRect = { settingsPanelRect.left + 20, mouseSteeringY, settingsPanelRect.right - 20, mouseSteeringY + toggleHeight * 2 };
        PointF mouseLabel1((REAL)mouseSteeringToggleRect.left, (REAL)mouseSteeringToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Mouse Steering", -1, &rowFont, mouseLabel1, &labelBrush);
        tooltips[20].bounds = {
            mouseSteeringToggleRect.left,
            mouseSteeringToggleRect.top,
            mouseSteeringToggleRect.right,
            mouseSteeringToggleRect.top + toggleHeight  // Only first line height
        };
        tooltips[20].text = L"Enables mouse steering for games. DO NOT USE in game's default mouse steering, always use this. Also works on games that don't have mouse steering support.";
        RectF mouseCheckboxRect(mouseLabel1.X + 140, mouseLabel1.Y, 20.0f, 20.0f);
        SolidBrush mouseBoxBrush(mouseSteeringEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&mouseBoxBrush, mouseCheckboxRect);
        graphics.DrawRectangle(&boxPen, mouseCheckboxRect);


        // --- Steering Sensitivity Slider ---
        int steeringSensY = mouseSteeringToggleRect.bottom + 15; // small gap below toggle
        steeringSensSliderRect = {
            settingsPanelRect.left + 20,
            steeringSensY,
            settingsPanelRect.right - 20,
            steeringSensY + sliderHeight
        };
        char steeringBuffer[8];
        sprintf_s(steeringBuffer, "%.2f", steeringSensitivity);
        float steeringNorm = (steeringSensitivity - 0.1f) / (5.0f - 0.1f);
        DrawSlider(steeringSensSliderRect, steeringNorm, "Steering Sensitivity", steeringBuffer);
        tooltips[21].bounds = steeringSensSliderRect;
        tooltips[21].text = L"Adjusts the sensitivity of mouse steering input.";
        // --- Max Steering Degrees Slider ---
        int steeringDegreesY = steeringSensSliderRect.bottom + 65; // gap below steering sensitivity
        steeringDegreesSliderRect = {
            settingsPanelRect.left + 20,
            steeringDegreesY,
            settingsPanelRect.right - 20,
            steeringDegreesY + sliderHeight
        };

        // Clamp maxSteeringDegrees to 90–900
        maxSteeringDegrees = max(90.0f, min(900.0f, maxSteeringDegrees));

        char steeringDegreesBuffer[8];
        sprintf_s(steeringDegreesBuffer, "%.0f", maxSteeringDegrees); // show integer degrees

        // Normalize for slider: 90 → 0, 900 → 1
        float steeringDegreesNorm = (maxSteeringDegrees - 90.0f) / (900.0f - 90.0f);

        DrawSlider(steeringDegreesSliderRect, steeringDegreesNorm, "Max Steering Degrees", steeringDegreesBuffer);
        tooltips[22].bounds = steeringDegreesSliderRect;
        tooltips[22].text = L"Limits the maximum steering rotation to prevent over-rotation. Higher values allow more steering range.";
        // --- Acc/Brake Sensitivity Slider ---
// --- Acc/Brake Sensitivity Slider ---
        int accBrakeSensY = steeringDegreesSliderRect.bottom + 65; // gap below max steering degrees
        accBrakeSensSliderRect = {
            settingsPanelRect.left + 20,
            accBrakeSensY,
            settingsPanelRect.right - 20,
            accBrakeSensY + sliderHeight
        };
        char accBrakeBuffer[8];
        sprintf_s(accBrakeBuffer, "%.2f", accBrakeSensitivity);
        float accBrakeNorm = (accBrakeSensitivity - 0.1f) / 19.9f; // normalize 0.1-20.0 range
        DrawSlider(accBrakeSensSliderRect, accBrakeNorm, "Acc/Brake Sensitivity", accBrakeBuffer);
        tooltips[23].bounds = accBrakeSensSliderRect;
        tooltips[23].text = L"Adjusts the sensitivity of throttle and brake input from vertical mouse movement.";

        // --- Throttle/Brake Indicator ---
        int showYBarY = accBrakeSensSliderRect.bottom + 30;
        showYBarToggleRect = { settingsPanelRect.left + 20, showYBarY, settingsPanelRect.right - 20, showYBarY + toggleHeight };

        PointF showYBarLabelPos((REAL)showYBarToggleRect.left, (REAL)showYBarToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Throttle/Brake Indicator", -1, &rowFont, showYBarLabelPos, &labelBrush);
        tooltips[24].bounds = showYBarToggleRect;
        tooltips[24].text = L"Shows a vertical bar next to the H-shifter displaying throttle and brake input levels.";

        RectF showYBarCheckboxRect(showYBarToggleRect.left + 190, showYBarToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush showYBarBoxBrush(showYBar ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showYBarBoxBrush, showYBarCheckboxRect);
        graphics.DrawRectangle(&boxPen, showYBarCheckboxRect);
        // --- Brake Resistance Factor Slider ---
        int brakeResistanceY = showYBarY + sliderHeight + 55;
        brakeresistanceSlider = {
            settingsPanelRect.left + 20,
            brakeResistanceY,
            settingsPanelRect.right - 20,
            brakeResistanceY + sliderHeight
        };

        char brakeResistanceBuffer[8];
        sprintf_s(brakeResistanceBuffer, "%.1f", brakeresistanceFactor);
        float brakeResistanceNorm = brakeresistanceFactor / 50.0f; // normalize 0-50 range
        DrawSlider(brakeresistanceSlider, brakeResistanceNorm, "Brake Resistance", brakeResistanceBuffer);
        tooltips[25].bounds = brakeresistanceSlider;
        tooltips[25].text = L"Adjusts resistance when pressing brakes (higher = more resistance at full brake).";

        // --- Acceleration Resistance Factor Slider ---
        int accelerationResistanceY = brakeResistanceY + sliderHeight + 55;
        accelerationresistanceSlider = {
            settingsPanelRect.left + 20,
            accelerationResistanceY,
            settingsPanelRect.right - 20,
            accelerationResistanceY + sliderHeight
        };

        char accelerationResistanceBuffer[8];
        sprintf_s(accelerationResistanceBuffer, "%.1f", accelerationResistanceFactor);
        float accelerationResistanceNorm = accelerationResistanceFactor / 50.0f; // normalize 0-50 range
        DrawSlider(accelerationresistanceSlider, accelerationResistanceNorm, "Throttle Resistance", accelerationResistanceBuffer);
        tooltips[26].bounds = accelerationresistanceSlider;
        tooltips[26].text = L"Adjusts resistance when pressing throttle (higher = more resistance at full throttle).";
        // --- Y-bar Fixed Transparency Toggle ---
        int yBarTransparencyY = accelerationresistanceSlider.bottom + 30;
        yBarFixedTransToggle = { settingsPanelRect.left + 20, yBarTransparencyY, settingsPanelRect.right - 20, yBarTransparencyY + toggleHeight };

        PointF yBarTransLabelPos((REAL)yBarFixedTransToggle.left, (REAL)yBarFixedTransToggle.top + settingsScrollOffset);
        graphics.DrawString(L"T/B Better Visibility", -1, &rowFont, yBarTransLabelPos, &labelBrush);
        tooltips[27].bounds = yBarFixedTransToggle;
        tooltips[27].text = L"Enhances throttle/brake indicator with dynamic colors and separate transparency control for better visibility in all conditions.";
        RectF yBarTransCheckboxRect(yBarFixedTransToggle.left + 190, yBarFixedTransToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush yBarTransBoxBrush(useYbarFixedTransparency ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&yBarTransBoxBrush, yBarTransCheckboxRect);
        graphics.DrawRectangle(&boxPen, yBarTransCheckboxRect);

        // --- Y-bar Alpha Slider ---
// --- Y-bar Alpha Slider ---
        int yBarAlphaY = yBarFixedTransToggle.bottom + 35;
        yBarAlphaSlider = { settingsPanelRect.left + 20, yBarAlphaY, settingsPanelRect.right - 20, yBarAlphaY + sliderHeight };

        char yBarAlphaBuffer[8];
        sprintf_s(yBarAlphaBuffer, "%d%%", (int)((yBarAlpha / 255.0f) * 100.0f)); // Convert to percentage
        DrawSlider(yBarAlphaSlider, (float)yBarAlpha / 255.0f, "T/B Transparency Level", yBarAlphaBuffer); // Changed label too
        tooltips[28].bounds = yBarAlphaSlider;
        tooltips[28].text = L"Sets the transparency level for the throttle/brake indicator when fixed transparency is enabled (0% = fully transparent, 100% = fully opaque).";

        // --- Axis Interdependent Smoothing ---
        int axisSmoothingY = yBarAlphaSlider.bottom + 35;  // Changed from showYBarToggleRect.bottom + 30
        useAxisSmoothingToggle = { settingsPanelRect.left + 20, axisSmoothingY, settingsPanelRect.right - 20, axisSmoothingY + toggleHeight };

        PointF axisSmoothingLabelPos((REAL)useAxisSmoothingToggle.left, (REAL)useAxisSmoothingToggle.top + settingsScrollOffset);
        graphics.DrawString(L"Steering T/B Smoothing", -1, &rowFont, axisSmoothingLabelPos, &labelBrush);
        tooltips[29].bounds = useAxisSmoothingToggle;
        tooltips[29].text = L"When enabled, reduces steering sensitivity during heavy acceleration/braking and vice versa for more stable control.";

        RectF axisSmoothingCheckboxRect(useAxisSmoothingToggle.left + 190, useAxisSmoothingToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush axisSmoothingBoxBrush(useAxisSmoothing ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&axisSmoothingBoxBrush, axisSmoothingCheckboxRect);
        graphics.DrawRectangle(&boxPen, axisSmoothingCheckboxRect);

        // --- Axis Smoothing Factor Slider ---
        int axisSmoothingFactorY = useAxisSmoothingToggle.bottom + 35;
        axisSmoothingFactorSlider = { settingsPanelRect.left + 20, axisSmoothingFactorY, settingsPanelRect.right - 20, axisSmoothingFactorY + sliderHeight };

        char axisSmoothingBuffer[8];
        sprintf_s(axisSmoothingBuffer, "%.2f", axisSmoothingFactor);
        DrawSlider(axisSmoothingFactorSlider, axisSmoothingFactor, "Smoothing Strength", axisSmoothingBuffer);
        tooltips[30].bounds = axisSmoothingFactorSlider;
        tooltips[30].text = L"Controls how much one axis's sensitivity is reduced when the other axis is active. Higher values = more stability.";

        // --- Mouse Steering Indicator ---
        int showXBarY = axisSmoothingFactorSlider.bottom + 35;
        showXBarToggleRect = { settingsPanelRect.left + 20, showXBarY, settingsPanelRect.right - 20, showXBarY + toggleHeight };

        PointF showXBarLabelPos((REAL)showXBarToggleRect.left, (REAL)showXBarToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Mouse Steering Indicator", -1, &rowFont, showXBarLabelPos, &labelBrush);
        tooltips[31].bounds = showXBarToggleRect;
        tooltips[31].text = L"Shows a horizontal bar below the H-shifter displaying steering input position.";

        RectF showXBarCheckboxRect(showXBarToggleRect.left + 190, showXBarToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush showXBarBoxBrush(showXBar ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showXBarBoxBrush, showXBarCheckboxRect);
        graphics.DrawRectangle(&boxPen, showXBarCheckboxRect);

        // --- Use Throttle/Brake Toggle ---
        int throttleBrakeY = showXBarToggleRect.bottom + 10;  // spacing after Mouse Steering Indicator
        useThrottleBrakeToggleRect = { settingsPanelRect.left + 20, throttleBrakeY, settingsPanelRect.right - 20, throttleBrakeY + toggleHeight };

        PointF throttleBrakeLabelPos((REAL)useThrottleBrakeToggleRect.left, (REAL)useThrottleBrakeToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Throttle/Brake (W/S)", -1, &rowFont, throttleBrakeLabelPos, &labelBrush);
        tooltips[32].bounds = useThrottleBrakeToggleRect;  // Update tooltip index as needed
        tooltips[32].text = L"Enables using W/S keys for throttle and brake control on vJoy Z/RZ axes.";

        RectF throttleBrakeCheckboxRect(throttleBrakeLabelPos.X + 190, throttleBrakeLabelPos.Y, 20.0f, 20.0f);
        SolidBrush throttleBrakeBoxBrush(useThrottleBrakeAxes ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&throttleBrakeBoxBrush, throttleBrakeCheckboxRect);
        graphics.DrawRectangle(&boxPen, throttleBrakeCheckboxRect);

        // --- Use Scroll → Rx Toggle ---
        int scrollClutchY = throttleBrakeY + toggleHeight + 10;  // spacing after throttle/brake toggle
        useScrollClutchToggleRect = { settingsPanelRect.left + 20, scrollClutchY, settingsPanelRect.right - 20, scrollClutchY + toggleHeight };

        PointF scrollLabelPos((REAL)useScrollClutchToggleRect.left, (REAL)useScrollClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Scroll to Clutch", -1, &rowFont, scrollLabelPos, &labelBrush);
        tooltips[33].bounds = useScrollClutchToggleRect;
        tooltips[33].text = L"Enables using mouse scroll wheel to control clutch input.";

        RectF scrollCheckboxRect(scrollLabelPos.X + 190, scrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush scrollBoxBrush(useScrollClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&scrollBoxBrush, scrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, scrollCheckboxRect);
        // --- Half Scroll Clutch Toggle ---
        int halfScrollClutchY = scrollClutchY + toggleHeight + 10; // spacing below scroll clutch toggle
        halfScrollClutchToggleRect = { settingsPanelRect.left + 20, halfScrollClutchY, settingsPanelRect.right - 20, halfScrollClutchY + toggleHeight };

        PointF halfScrollLabelPos((REAL)halfScrollClutchToggleRect.left, (REAL)halfScrollClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Half Scroll Clutch", -1, &rowFont, halfScrollLabelPos, &labelBrush);
        tooltips[34].bounds = halfScrollClutchToggleRect;
        tooltips[34].text = L"Uses only half the scroll range for full clutch control (0-100% clutch in half the axis range).";
        RectF halfScrollCheckboxRect(halfScrollLabelPos.X + 190, halfScrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush halfScrollBoxBrush(useHalfClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&halfScrollBoxBrush, halfScrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, halfScrollCheckboxRect);

        // --- Show Clutch / Rx Indicator Toggle ---
        int showClutchY = halfScrollClutchY + toggleHeight + 10;
        showClutchToggleRect = { settingsPanelRect.left + 20, showClutchY, settingsPanelRect.right - 20, showClutchY + toggleHeight };

        PointF showClutchLabelPos((REAL)showClutchToggleRect.left, (REAL)showClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Clutch Indicator", -1, &rowFont, showClutchLabelPos, &labelBrush);
        tooltips[35].bounds = showClutchToggleRect;
        tooltips[35].text = L"Shows clutch level indicator bar and changes H-shifter color when clutch is applied.";
        RectF showClutchCheckboxRect(showClutchLabelPos.X + 190, showClutchLabelPos.Y, 20.0f, 20.0f);
        SolidBrush showClutchBoxBrush(showClutchIndicator ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showClutchBoxBrush, showClutchCheckboxRect);
        graphics.DrawRectangle(&boxPen, showClutchCheckboxRect);


        // --- Scroll Sensitivity Slider ---
        int scrollSensY = showClutchY + toggleHeight + 45; // 20px gap after showClutch toggle
        scrollSensSliderRect = {
            settingsPanelRect.left + 20,
            scrollSensY,
            settingsPanelRect.right - 20,
            scrollSensY + sliderHeight
        };

        char scrollSensBuffer[8];
        sprintf_s(scrollSensBuffer, "%.2f", scrollClutchSens);
        float scrollSensNorm = scrollClutchSens / 10.0f; // normalize 0–10 range
        DrawSlider(scrollSensSliderRect, scrollSensNorm, "Scroll Sensitivity", scrollSensBuffer);
        tooltips[36].bounds = scrollSensSliderRect;
        tooltips[36].text = L"Adjusts how sensitive the scroll wheel is for clutch control.";

        // --- Smooth Scroll Speed Slider ---
        int smoothScrollY = scrollSensY + toggleHeight + 45;
        smoothScrollSlider = {
            settingsPanelRect.left + 20,
            smoothScrollY,
            settingsPanelRect.right - 20,
            smoothScrollY + sliderHeight
        };

        char smoothScrollBuffer[8];
        sprintf_s(smoothScrollBuffer, "%.1f", smoothScrollSpeed);
        float smoothScrollNorm = (smoothScrollSpeed - 1.0f) / 19.0f; // normalize 1.0-20.0 range
        DrawSlider(smoothScrollSlider, smoothScrollNorm, "Scroll Smoothness", smoothScrollBuffer);
        tooltips[37].bounds = smoothScrollSlider;
        tooltips[37].text = L"Adjusts how smoothly the scroll clutch moves (higher = faster response, lower = smoother movement).";

        // --- Invert Scroll → Clutch Toggle ---
        int invertScrollY = smoothScrollY + sliderHeight + 30; // 20px gap after scroll sensitivity slider
        invertScrollToggleRect = { settingsPanelRect.left + 20, invertScrollY, settingsPanelRect.right - 20, invertScrollY + toggleHeight };

        PointF invertScrollLabelPos((REAL)invertScrollToggleRect.left, (REAL)invertScrollToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Invert Scroll to Clutch", -1, &rowFont, invertScrollLabelPos, &labelBrush);
        tooltips[38].bounds = invertScrollToggleRect;
        tooltips[38].text = L"Reverses the scroll direction for clutch control (scroll up/down reversed).";
        RectF invertScrollCheckboxRect(invertScrollLabelPos.X + 190, invertScrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush invertScrollBoxBrush(invertScrollClutchAxis ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&invertScrollBoxBrush, invertScrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, invertScrollCheckboxRect);

        // --- Adjust steering combo position ---
        int steeringY = invertScrollY + toggleHeight + 40; // 40px gap after invert toggle
        steeringDeviceComboRect = { settingsPanelRect.left + 20, steeringY, settingsPanelRect.right - 20, steeringY + comboHeight };


        // --- Steering Device Selector ---
// --- Steering Device Selector ---


        PointF steeringLabelPos((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"Steering Mouse Device:", -1, &rowFont, steeringLabelPos, &labelBrush);
        tooltips[39].bounds = steeringDeviceComboRect;
        tooltips[39].text = L"Select which mouse device controls steering (can be different from H-shifter mouse).";

        RectF steeringBoxRect((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top + settingsScrollOffset),
            (REAL)(steeringDeviceComboRect.right - steeringDeviceComboRect.left), (REAL)(steeringDeviceComboRect.bottom - steeringDeviceComboRect.top));
        graphics.FillRectangle(&deviceBoxBrush, steeringBoxRect);
        graphics.DrawRectangle(&deviceBoxPen, steeringBoxRect);

        std::wstring selName3 = L"All Mice";
        if (g_selectedSteeringDevice)
        {
            for (auto& d : g_mouseDevices)
                if (d.hDevice == g_selectedSteeringDevice)
                    selName3 = d.name;
        }
        PointF steeringTextPos((REAL)steeringDeviceComboRect.left + 4, (REAL)steeringDeviceComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selName3.c_str(), -1, &rowFont, steeringTextPos, &valueBrush);
        // --- Subtle Line Below Steering Device Selector ---
        int steeringLineY = steeringDeviceComboRect.bottom + 20; // 20 pixels below the steering device selector
        Pen steeringLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&steeringLinePen,
            settingsPanelRect.left + 40, steeringLineY + settingsScrollOffset,
            settingsPanelRect.right - 40, steeringLineY + settingsScrollOffset);
        // --- Use XInput Toggle ---
        int lastY = steeringDeviceComboRect.bottom + 90; // 30px gap

        // --- Use XInput / Controller Toggle ---


// --- Subtle Line and Heading Above Controller ---
        int controllerHeadingY = lastY - 45; // Position above the toggle
        Pen controllerLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&controllerLinePen,
            settingsPanelRect.left + 40, controllerHeadingY + settingsScrollOffset,
            settingsPanelRect.right - 40, controllerHeadingY + settingsScrollOffset);

        PointF controllerHeadingPos((REAL)(settingsPanelRect.left), (REAL)(controllerHeadingY + 8 + settingsScrollOffset));
        SolidBrush controllerHeadingBrush(Color(180, 180, 180)); // Subtle gray-white color
        StringFormat controllerHeadingFormat;
        controllerHeadingFormat.SetAlignment(StringAlignmentCenter);
        controllerHeadingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF controllerHeadingRect((REAL)settingsPanelRect.left, (REAL)(controllerHeadingY + 8 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Controller", -1, &rowFont, controllerHeadingRect, &controllerHeadingFormat, &controllerHeadingBrush);

        // --- Enable Controller Toggle ---
        useXInputToggleRect = { settingsPanelRect.left + 20, lastY, settingsPanelRect.right - 20, lastY + toggleHeight };
        PointF useXInputLabelPos((REAL)useXInputToggleRect.left, (REAL)useXInputToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Enable Controller", -1, &rowFont, useXInputLabelPos, &labelBrush);
        // Enable Controller
        tooltips[40].bounds = useXInputToggleRect;
        tooltips[40].text = L"Uses controller instead of mouse for H-shifter input.";
        // Draw checkbox closer to label
        RectF useXInputCheckboxRect(useXInputToggleRect.left + 180, useXInputToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush useXInputBoxBrush(useXInput ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&useXInputBoxBrush, useXInputCheckboxRect);
        graphics.DrawRectangle(&boxPen, useXInputCheckboxRect);

        // --- Gamepad Device Selector ---
        const int spacingAfterToggle = 40; // gap after Enable Controller
        int gamepadComboY = useXInputToggleRect.bottom + spacingAfterToggle;

        gamepadComboRect = { settingsPanelRect.left + 20, gamepadComboY, settingsPanelRect.right - 20, gamepadComboY + comboHeight };
        PointF gamepadLabelPos((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset - nameControlSpacing));
        graphics.DrawString(L"Select Controller:", -1, &rowFont, gamepadLabelPos, &labelBrush);
        // Gamepad Device Selector
        tooltips[41].bounds = gamepadComboRect;
        tooltips[41].text = L"Select which gamepad/controller to use for H-shifter input.";


        // Draw selector box
        RectF gamepadBoxRect((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset),
            (REAL)(gamepadComboRect.right - gamepadComboRect.left), (REAL)(gamepadComboRect.bottom - gamepadComboRect.top));
        SolidBrush gamepadBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&gamepadBoxBrush, gamepadBoxRect);
        Pen gamepadBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&gamepadBoxPen, gamepadBoxRect);

        // Draw selected gamepad name
        std::wstring selGamepadName = L"None";
        if (g_selectedGamepadIndex >= 0 && g_selectedGamepadIndex < g_gamepads.size()) {
            const char* name = SDL_GameControllerName(g_gamepads[g_selectedGamepadIndex].controller);
            selGamepadName = ToWString(name);
        }
        PointF selTextPos1((REAL)gamepadComboRect.left + 4, (REAL)gamepadComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selGamepadName.c_str(), -1, &rowFont, selTextPos1, &valueBrush);

        // --- Controller Sensitivity Slider ---
        // Position it just below Gamepad selector
        int controllerSensSliderY = gamepadComboRect.bottom + 55; // small gap
        controllerSensSliderRect = {
            settingsPanelRect.left + 20,
            controllerSensSliderY,
            settingsPanelRect.right - 20,
            controllerSensSliderY + sliderHeight
        };

        char sensValueBuffer[8];
        sprintf_s(sensValueBuffer, "%.2f", controllerSensMultiplier);
        DrawSlider(controllerSensSliderRect, controllerSensSliderValue, "Controller Sensitivity", sensValueBuffer);
        // Controller Sensitivity
        tooltips[42].bounds = controllerSensSliderRect;
        tooltips[42].text = L"Adjusts the sensitivity of controller stick inputs for H-shifter.";

        // --- Use Right Stick for Knob Toggle ---
        // (your existing code continues here)

        // --- Use Right Stick for Knob Toggle ---
        int rightStickToggleY = controllerSensSliderRect.bottom + 40; // space after slider
        useRightStickToggleRect = { settingsPanelRect.left + 20, rightStickToggleY, settingsPanelRect.right - 20, rightStickToggleY + toggleHeight };

        PointF rightStickLabelPos((REAL)useRightStickToggleRect.left, (REAL)useRightStickToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Right Stick for Knob", -1, &rowFont, rightStickLabelPos, &labelBrush);
        tooltips[43].bounds = useRightStickToggleRect;
        tooltips[43].text = L"Uses right stick instead of left stick for H-shifter knob movement.";
        // Checkbox
        RectF rightStickCheckboxRect(useRightStickToggleRect.left + 180, useRightStickToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush rightStickBoxBrush(useRightStick ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&rightStickBoxBrush, rightStickCheckboxRect);
        graphics.DrawRectangle(&boxPen, rightStickCheckboxRect);
        // --- Disable Real Knob Movement Toggle ---
        int disableKnobY = useRightStickToggleRect.bottom + 20; // space after Right Stick toggle
        disableRealKnobMovementToggleRect = { settingsPanelRect.left + 20, disableKnobY, settingsPanelRect.right - 20, disableKnobY + toggleHeight };
        PointF disableKnobLabelPos((REAL)disableRealKnobMovementToggleRect.left, (REAL)disableRealKnobMovementToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Right Stick to look", -1, &rowFont, disableKnobLabelPos, &labelBrush);
        // Use Right Stick to look
        tooltips[44].bounds = disableRealKnobMovementToggleRect;
        tooltips[44].text = L"Uses right stick for camera look instead of H-shifter movement. Holding assist button enables knob assist and disables camera look.";
        // Checkbox closer to label
        RectF disableKnobCheckboxRect(disableRealKnobMovementToggleRect.left + 180, disableRealKnobMovementToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush disableKnobBoxBrush(disableRealKnobMovement ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&disableKnobBoxBrush, disableKnobCheckboxRect);
        graphics.DrawRectangle(&boxPen, disableKnobCheckboxRect);

        // --- Binding Mode For R-Axis Toggle ---
        int bindingModeY = disableRealKnobMovementToggleRect.bottom + 20; // space after previous toggle
        bindingModeForAxisToggle = { settingsPanelRect.left + 20, bindingModeY, settingsPanelRect.right - 20, bindingModeY + toggleHeight };
        PointF bindingModeLabelPos((REAL)bindingModeForAxisToggle.left, (REAL)bindingModeForAxisToggle.top + settingsScrollOffset);
        graphics.DrawString(L"Binding Mode", -1, &rowFont, bindingModeLabelPos, &labelBrush);
        // Binding Mode for R-Axis
        tooltips[45].bounds = bindingModeForAxisToggle; // Add new tooltip index
        tooltips[45].text = L"When enabled, right stick and clutch will output full values with minimal input for easier controller binding. DISABLE IT WHEN IT'S NOT NEEDED.";
        // Checkbox
        RectF bindingModeCheckboxRect(bindingModeForAxisToggle.left + 180, bindingModeForAxisToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush bindingModeBoxBrush(bindingModeForRAxis ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&bindingModeBoxBrush, bindingModeCheckboxRect);
        graphics.DrawRectangle(&boxPen, bindingModeCheckboxRect);

        // --- Invert Assist Axes Toggle ---
        int invertAssistY = bindingModeForAxisToggle.bottom + 20; // space after Binding Mode toggle
        invertAssistToggleRect = { settingsPanelRect.left + 20, invertAssistY, settingsPanelRect.right - 20, invertAssistY + toggleHeight };
        PointF invertAssistLabelPos((REAL)invertAssistToggleRect.left, (REAL)invertAssistToggleRect.top + settingsScrollOffset);
        // Main label
        graphics.DrawString(L"Hold Assist to Look", -1, &rowFont, invertAssistLabelPos, &labelBrush);
        // Hold Assist to Look
        tooltips[46].bounds = invertAssistToggleRect;
        tooltips[46].text = L"Holding assist button enables right stick for camera look (disables assist knob).";
        // --- Subtitle / hint ---
        // Checkbox
        RectF invertAssistCheckboxRect(invertAssistToggleRect.left + 180, invertAssistToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush invertAssistBoxBrush(invertAssistAxes ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&invertAssistBoxBrush, invertAssistCheckboxRect);
        graphics.DrawRectangle(&boxPen, invertAssistCheckboxRect);

        // --- Use LT as Clutch Toggle ---
        int useLTAsClutchY = invertAssistToggleRect.bottom + 20; // space after previous toggle
        useLTAsClutchToggleRect = { settingsPanelRect.left + 20, useLTAsClutchY, settingsPanelRect.right - 20, useLTAsClutchY + toggleHeight };
        PointF useLTAsClutchLabelPos((REAL)useLTAsClutchToggleRect.left, (REAL)useLTAsClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use LT/L2 as Clutch", -1, &rowFont, useLTAsClutchLabelPos, &labelBrush);
        // Use LT/L2 as Clutch
        tooltips[47].bounds = useLTAsClutchToggleRect;
        tooltips[47].text = L"Uses LT/L2 trigger as clutch input instead of scroll wheel.";
        // Checkbox
        RectF useLTAsClutchCheckboxRect(useLTAsClutchToggleRect.left + 180, useLTAsClutchToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush useLTAsClutchBoxBrush(useLTAsClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&useLTAsClutchBoxBrush, useLTAsClutchCheckboxRect);
        graphics.DrawRectangle(&boxPen, useLTAsClutchCheckboxRect);
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();
            RectF listRect((REAL)gearLayoutButtonRect.left, (REAL)listY, (REAL)gearLayoutButtonRect.right - gearLayoutButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&layoutBoxPen, listRect);

            for (size_t i = 0; i < gearLayouts.size(); ++i)
            {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight (traditional green)
                if ((int)i == hoveredGearLayoutIndex)
                {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Traditional green
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected item highlight
                if (i == currentGearLayout)
                {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::wstring itemName = gearLayoutNames[i];
                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(itemName.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();
            RectF listRect((REAL)hShifterLayoutButtonRect.left, (REAL)listY, (REAL)hShifterLayoutButtonRect.right - hShifterLayoutButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&layoutBoxPen, listRect);

            for (size_t i = 0; i < hShifterLayouts.size(); ++i)
            {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight (traditional green)
                if ((int)i == hoveredHShifterLayoutIndex)
                {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Traditional green
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected item highlight
                if ((i + 1) == currentHShifterLayout)
                {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::wstring itemName = hShifterLayouts[i].name;
                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(itemName.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }
        if (profileDropdownOpen) {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = profileButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size();
            RectF listRect((REAL)profileButtonRect.left, (REAL)listY, (REAL)profileButtonRect.right - profileButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&profileBoxPen, listRect);

            for (size_t i = 0; i < profileNames.size(); ++i) {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight
                if ((int)i == hoveredProfileIndex) {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Same green as other dropdowns
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected profile highlight
                if ((int)i == currentProfileIndex) {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::string profileName = profileNames[i];
                if (profileName.size() > 4 && profileName.substr(profileName.size() - 4) == ".ini") {
                    profileName = profileName.substr(0, profileName.size() - 4);
                }
                std::wstring profileNameW(profileName.begin(), profileName.end());

                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(profileNameW.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }
        // --- Update max scroll ---
// --- Update max scroll ---
        if (g_showTooltip) {
            std::wstring tooltipText = L"Prevents in-game mouse look & right stick while using H-shifter.\n\n• Blocks mouse input to selected game\n• Blocks controller right stick (XInput)\n• Hold RMB to temporarily use mouse\n• Knob Assist Button affects R stick blocking\n• Disable Knob to restore normal controls\n\n Experimental: PlayStation controllers may have issues (DS4Windows)";            Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
            SolidBrush tooltipTextBrush(Color(255, 255, 255));

            // Set up format WITH word wrapping
            StringFormat tooltipFormat;
            tooltipFormat.SetAlignment(StringAlignmentNear);
            tooltipFormat.SetLineAlignment(StringAlignmentNear);
            // Remove StringFormatFlagsNoWrap to enable word wrapping

            // Measure text with wrapping
            RectF textBounds;
            graphics.MeasureString(tooltipText.c_str(), -1, &tooltipFont, RectF(0, 0, 250, 500), &tooltipFormat, &textBounds);

            // Create tooltip rectangle
            RectF tooltipTextRect(
                (REAL)processComboRect.left,
                (REAL)(processComboRect.top - 40 + settingsScrollOffset),
                (REAL)270,
                textBounds.Height + 20
            );

            // Draw background and border
            SolidBrush tooltipBgBrush(Color(30, 30, 30));
            graphics.FillRectangle(&tooltipBgBrush, tooltipTextRect);
            graphics.DrawRectangle(&layoutBoxPen, tooltipTextRect);

            // Draw text with word wrapping
            graphics.DrawString(
                tooltipText.c_str(),
                -1,
                &tooltipFont,
                RectF(tooltipTextRect.X + 8, tooltipTextRect.Y + 8, tooltipTextRect.Width - 16, tooltipTextRect.Height - 16),
                &tooltipFormat,
                &tooltipTextBrush
            );
        }
        // Find the bottom-most control: transparency slider is the last one
        int lastControlBottom = useLTAsClutchToggleRect.bottom; // now last toggle
// Update scroll max to account for dropdowns
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int layoutListBottom = (int)(gearLayoutButtonRect.bottom + listItemHeight * gearLayouts.size());
            lastControlBottom = max(lastControlBottom, layoutListBottom + 20);
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int layoutListBottom = (int)(hShifterLayoutButtonRect.bottom + listItemHeight * hShifterLayouts.size());
            lastControlBottom = max(lastControlBottom, layoutListBottom + 20);
        }
        if (profileDropdownOpen) {
            int listItemHeight = 25;
            int profileListBottom = (int)(profileButtonRect.bottom + listItemHeight * profileNames.size());
            lastControlBottom = max(lastControlBottom, profileListBottom + 20);
        }
        settingsScrollMax = max(0, lastControlBottom - (settingsPanelRect.bottom - settingsPanelRect.top));

        for (const auto& tooltip : tooltips) {
            // Special case for sliders only (indices 0-6, 14-16, 19-21, 27, 32) - draw 25px below
            if ((&tooltip >= &tooltips[0] && &tooltip <= &tooltips[6]) ||    // First 7 sliders
                (&tooltip >= &tooltips[16] && &tooltip <= &tooltips[18]) ||  // Transparency sliders
                (&tooltip >= &tooltips[21] && &tooltip <= &tooltips[23]) ||
                (&tooltip >= &tooltips[25] && &tooltip <= &tooltips[26]) ||
                (&tooltip >= &tooltips[28] && &tooltip <= &tooltips[29]) ||// Steering sensitivity sliders
                (&tooltip >= &tooltips[30] && &tooltip <= &tooltips[30]) ||
                (&tooltip >= &tooltips[37] && &tooltip <= &tooltips[36]) ||
                // Scroll sensitivity slider
                &tooltip == &tooltips[42]) {                                 // Controller sensitivity slider
                DrawTooltip(graphics, tooltip, settingsScrollOffset, 25);
            }
            // Special handling for inverted scroll toggle to position it above
            else if (&tooltip == &tooltips[47]) {
                DrawTooltip(graphics, tooltip, settingsScrollOffset, -85); // Position above instead of below
            }
            else {
                // All other tooltips (toggles, buttons, dropdowns) use default offset (25px)
                DrawTooltip(graphics, tooltip, settingsScrollOffset);
            }
        }
        // --- Reset clip ---
        graphics.ResetClip();
    }

    if (showVJoyPicker && vJoyButtonCount > 0)
    {
        int panelWidth = 300;  // wider window
        int panelPadding = 20; // padding around buttons
        int buttonHeight = 28; // button height
        int buttonSpacing = 8; // space between buttons
        int titleHeight = 35;

        int panelHeight = titleHeight + panelPadding + vJoyButtonCount * (buttonHeight + buttonSpacing) + panelPadding;

        RECT parentRect;
        GetClientRect(hwnd, &parentRect);

        int left = max(0, (parentRect.right - panelWidth) / 2);
        int top = max(0, (parentRect.bottom - panelHeight) / 2);

        // Update global picker RECT
        g_vJoyPickerRect = { left, top, left + panelWidth, top + panelHeight };

        // GDI+ Rect for drawing
        Gdiplus::Rect panelRectGDI(left, top, panelWidth, panelHeight);

        // --- Background ---
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(40, 40, 40)); // dark gray
        graphics.FillRectangle(&bgBrush, panelRectGDI);

        // --- Title ---
        Gdiplus::FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font titleFont(&fontFamily, 18, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush titleBrush(Gdiplus::Color(0, 200, 136)); // accent color
        Gdiplus::RectF titleRectF((Gdiplus::REAL)left, (Gdiplus::REAL)top + 5, (Gdiplus::REAL)panelWidth, (Gdiplus::REAL)titleHeight);

        Gdiplus::StringFormat titleFormat;
        titleFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        titleFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        graphics.DrawString(L"Select vJoy Button", -1, &titleFont, titleRectF, &titleFormat, &titleBrush);

        // --- Buttons ---
        g_vJoyButtonRects.clear();
        Gdiplus::Font btnFont(&fontFamily, 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush btnTextBrush(Gdiplus::Color(220, 220, 220)); // light text

        int btnX = left + panelPadding;
        int btnW = panelWidth - 2 * panelPadding;
        int btnYStart = top + titleHeight + panelPadding;

        for (int i = 0; i < vJoyButtonCount; ++i)
        {
            int btnY = btnYStart + i * (buttonHeight + buttonSpacing);

            RECT btnRectWin = { btnX, btnY, btnX + btnW, btnY + buttonHeight };
            g_vJoyButtonRects.push_back(btnRectWin);

            Gdiplus::Rect btnRectGDI(btnX, btnY, btnW, buttonHeight);
            Gdiplus::SolidBrush btnBrush(Gdiplus::Color(60, 60, 60));
            graphics.FillRectangle(&btnBrush, btnRectGDI);

            // --- Centered button text ---
            wchar_t buf[32];
            swprintf_s(buf, L"vJoy Btn %d", i + 1);
            Gdiplus::RectF btnTextRectF((Gdiplus::REAL)btnRectGDI.X, (Gdiplus::REAL)btnRectGDI.Y,
                (Gdiplus::REAL)btnRectGDI.Width, (Gdiplus::REAL)btnRectGDI.Height);
            Gdiplus::StringFormat btnFormat;
            btnFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            btnFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

            graphics.DrawString(buf, -1, &btnFont, btnTextRectF, &btnFormat, &btnTextBrush);
        }
    }
    // --- Floating Update Button ---
// --- Floating Update Button Stub ---
    if (updateAvailable && !isBorderless)
    {
        // Position in middle-bottom (for potential click detection)
        int buttonWidth = 220;   // slightly wider
        int buttonHeight = 50;   // slightly taller
        int margin = 20;

        int centerX = width / 2;
        int buttonX = centerX - (buttonWidth / 2);
        int buttonY = height - buttonHeight - margin;

        updateButtonRect = {
            buttonX,
            buttonY,
            buttonX + buttonWidth,
            buttonY + buttonHeight
        };

        // Draw a simple text message with bigger, subtle yellow color
        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 16, FontStyleRegular, UnitPixel); // bigger font
        SolidBrush brush(Color(255, 255, 230, 150)); // subtle yellow

        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        RectF rectF((REAL)updateButtonRect.left, (REAL)updateButtonRect.top, (REAL)buttonWidth, (REAL)buttonHeight);
        graphics.DrawString(L"Update available, click here", -1, &font, rectF, &format, &brush);
    }

    if (!isBorderless)
    {
        int titleHeight = 24; // thinner title bar
        int buttonSize = 20;
        int buttonMargin = 6;

        // Rounded Close button (red)
        Gdiplus::RectF closeRectF(width - buttonMargin - buttonSize, (titleHeight - buttonSize) / 2, buttonSize, buttonSize);
        Gdiplus::SolidBrush closeBrush(Gdiplus::Color(220, 235, 70, 70));
        graphics.FillEllipse(&closeBrush, closeRectF);
        g_CloseButtonRect = { (int)closeRectF.X, (int)closeRectF.Y, (int)(closeRectF.X + closeRectF.Width), (int)(closeRectF.Y + closeRectF.Height) };

        // Rounded Maximize button (greenish)
        Gdiplus::RectF maxRectF(width - buttonMargin * 2 - buttonSize * 2, (titleHeight - buttonSize) / 2, buttonSize, buttonSize);
        Gdiplus::SolidBrush maxBrush(Gdiplus::Color(220, 70, 220, 70));
        graphics.FillEllipse(&maxBrush, maxRectF);
        g_MaxButtonRect = { (int)maxRectF.X, (int)maxRectF.Y, (int)(maxRectF.X + maxRectF.Width), (int)(maxRectF.Y + maxRectF.Height) };

        // Window title text
        Gdiplus::FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font titleFont(&fontFamily, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush textBrush(Gdiplus::Color(220, 200, 200, 200));
        Gdiplus::RectF textRect(0, 0, (REAL)width, (REAL)titleHeight); // full width
        Gdiplus::StringFormat sf;
        sf.SetAlignment(Gdiplus::StringAlignmentCenter);     // center horizontally
        sf.SetLineAlignment(Gdiplus::StringAlignmentCenter); // center vertically
        graphics.DrawString(L"MouseShifter", -1, &titleFont, textRect, &sf, &textBrush);
    }

    // --- Draw overlay if not borderless ---
    if (!isBorderless)
        DrawHotkeyOverlay(graphics);
    // --- Blit the offscreen bitmap to the window DC ---
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // --- Cleanup ---
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
