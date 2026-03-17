#include "AppGlobals.h"
#include "Updater.h"
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
// Add these global tracking variables for rails
static float lastClutchNorm = 0.0f;
static int lastRedrawRailCount = 0;
static DWORD lastRailRedrawTime = 0;
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
void DrawBorderless(HDC hdc, int width, int height)
{
    // We're drawing directly on the provided HDC which already has the background
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // --- Draw static elements (rails and gears) ---
    DrawStaticShifterElements(graphics);
    DrawStaticGearElements(graphics);

    // --- Draw dynamic elements (knob and ghost knob) ---
    // Draw ghost knob (assist pointer)
    if (useAssistPointer)
    {
        float ghostRadius = knobRadius * 0.70f;
        Gdiplus::Color ghostColor = ghostSnappedGear.empty() ? Gdiplus::Color(150, 150, 255, 128)
            : Gdiplus::Color(200, 200, 255, 180);
        Gdiplus::SolidBrush ghostBrush(ghostColor);
        Gdiplus::Pen ghostPen(ghostColor, 2);

        Gdiplus::Rect ghostRect(
            ghostKnobPos.x - ghostRadius,
            ghostKnobPos.y - ghostRadius,
            ghostRadius * 2,
            ghostRadius * 2);

        graphics.FillEllipse(&ghostBrush, ghostRect);
        graphics.DrawEllipse(&ghostPen, ghostRect);

        if (!ghostSnappedGear.empty())
        {
            Gdiplus::Font font(L"Segoe UI", static_cast<INT>(ghostRadius / 2), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 220));
            std::string gearLabel = ghostSnappedGear;
            if (gearLabelOverride.find(ghostSnappedGear) != gearLabelOverride.end())
                gearLabel = gearLabelOverride[ghostSnappedGear];

            std::wstring wGear(gearLabel.begin(), gearLabel.end());

            Gdiplus::RectF textRect(
                ghostKnobPos.x - ghostRadius,
                ghostKnobPos.y - ghostRadius / 2,
                ghostRadius * 2,
                ghostRadius);

            Gdiplus::StringFormat format;
            format.SetAlignment(Gdiplus::StringAlignmentCenter);
            format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
            graphics.DrawString(wGear.c_str(), -1, &font, textRect, &format, &textBrush);
        }
    }

    // Draw main knob
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
        bool shouldDrawReverse = false;
        std::string reverseGearKey = "R";

        // Check if reverse exists in either lower or high gear positions
        if (lowerGearPositions.find(reverseGearKey) != lowerGearPositions.end() ||
            highGearPositions.find(reverseGearKey) != highGearPositions.end())
        {
            shouldDrawReverse = true;
        }
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

    // --- Draw horizontal X-axis indicator (mouse steering) ---
    if (showXBar && mouseSteeringEnabled)
    {
        float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
        normalizedX = max(0.0f, min(1.0f, normalizedX));

        // Get rail count for positioning
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

        int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
        int xBarWidth = static_cast<int>(fullWidth * 0.7f);
        xBarWidth = max(xBarWidth, 50);
        int xBarHeight = 8;
        int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
        int xBarYOffset = 30;
        int xBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
        int xBarLeft = xCenter - xBarWidth / 2;

        // Background bar
        Gdiplus::SolidBrush xBarBg(Gdiplus::Color(50, 50, 50));
        Gdiplus::Rect xBarRect(xBarLeft, xBarY, xBarWidth, xBarHeight);
        graphics.FillRectangle(&xBarBg, xBarRect);

        // Red locked regions
        LONG centerX = (axisMax + axisMin) / 2;
        LONG axisRange = axisMax - axisMin;
        LONG steeringMaxOffset = (LONG)((maxSteeringDegrees / fullWheelDegrees) * (axisRange / 2));
        int leftClampX = xBarLeft + (int)(((centerX - steeringMaxOffset) - axisMinX) * xBarWidth / (axisMaxX - axisMinX));
        int rightClampX = xBarLeft + (int)(((centerX + steeringMaxOffset) - axisMinX) * xBarWidth / (axisMaxX - axisMinX));

        Gdiplus::SolidBrush redBrush(Gdiplus::Color(200, 50, 50));
        int xBarRight = xBarLeft + xBarWidth;

        // Left locked zone
        graphics.FillRectangle(&redBrush, xBarLeft, xBarY, leftClampX - xBarLeft, xBarHeight);
        // Right locked zone
        graphics.FillRectangle(&redBrush, rightClampX, xBarY, xBarRight - rightClampX - 1, xBarHeight);

        // Center line
        Gdiplus::Pen xCenterLine(Gdiplus::Color(180, 180, 180), 2);
        graphics.DrawLine(&xCenterLine, xCenter, xBarY, xCenter, xBarY + xBarHeight);

        // Indicator circle
        int indicatorRadius = xBarHeight * 1.5f;
        int indicatorX = xBarLeft + (int)(normalizedX * xBarWidth);
        int indicatorY = xBarY + xBarHeight / 2;
        Gdiplus::SolidBrush indicatorBrush(Gdiplus::Color(180, 180, 180)); // Light grey fill
        Gdiplus::Pen indicatorPen(Gdiplus::Color(120, 120, 120), 2);      // Darker grey outline
        Gdiplus::Rect indicatorRect(indicatorX - indicatorRadius, indicatorY - indicatorRadius,
            indicatorRadius * 2, indicatorRadius * 2);
        graphics.FillEllipse(&indicatorBrush, indicatorRect);
        graphics.DrawEllipse(&indicatorPen, indicatorRect);
    }
    if (showYBar && mouseSteeringEnabled && !useYbarFixedTransparency)
    {
        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
        normalizedY = max(0.0f, min(1.0f, normalizedY));

        // Get rail count for positioning
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

        int barWidth = 8;
        int barHeight = bottomY - topY;
        int lastRailX = railX[drawRailCount - 1].x;
        int yBarOffset = 20;
        int barY = centerY - barHeight / 2;
        int xBarGap = 20;
        int barX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

        // Compute dynamic color from green → red based on Y
        BYTE r = (BYTE)(normalizedY * 220);
        BYTE g = (BYTE)((1.0f - normalizedY) * 220);
        Gdiplus::Color dynamicColor(r, g, 50);

        // Background (dynamic)
        Gdiplus::SolidBrush barBg(dynamicColor);
        Gdiplus::Pen barOutline(Gdiplus::Color(120, 120, 120), 2);
        Gdiplus::Rect barRect(barX, barY, barWidth, barHeight);
        graphics.FillRectangle(&barBg, barRect);
        graphics.DrawRectangle(&barOutline, barRect);

        // Filled portion (grey, inverted: top = min, bottom = max)
        int filledHeight = (int)(barHeight * normalizedY);
        int fillY = barY;
        Gdiplus::SolidBrush barFill(Gdiplus::Color(50, 50, 50));
        Gdiplus::Rect fillRect(barX, fillY, barWidth, filledHeight);
        graphics.FillRectangle(&barFill, fillRect);

        // Small horizontal line in middle
        int lineY = barY + barHeight / 2;
        Gdiplus::Pen linePen(Gdiplus::Color(255, 255, 255), 2);
        graphics.DrawLine(&linePen, barX, lineY, barX + barWidth, lineY);
    }

    // Normalize Y axis
    if (showClutchIndicator && useScrollClutch)
    {
        // Inverted normalization (axisMin → full bottom, axisMax → top)
        float normalizedRx;
        // Get rail count for positioning
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
}
void DrawYBarOnly(HDC hdc, int width, int height)
{
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // --- Draw vertical Y-axis indicator (throttle/brake) ---
    if (showYBar && mouseSteeringEnabled)
    {
        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
        normalizedY = max(0.0f, min(1.0f, normalizedY));

        // Get rail count for positioning
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

        int barWidth = 8;
        int barHeight = bottomY - topY;
        int lastRailX = railX[drawRailCount - 1].x;
        int yBarOffset = 20;
        int barY = centerY - barHeight / 2;
        int xBarGap = 20;
        int barX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

        // Compute dynamic color from green → red based on Y
        BYTE r = (BYTE)(normalizedY * 255);  // Increased to 255 for more vibrant colors
        BYTE g = (BYTE)((1.0f - normalizedY) * 255);  // Increased to 255 for more vibrant colors
        Gdiplus::Color dynamicColor(r, g, 50);

        // Background (dynamic) - only change the background color
        Gdiplus::SolidBrush barBg(dynamicColor);
        Gdiplus::Pen barOutline(Gdiplus::Color(200, 200, 200), 2);  // Brighter outline for better contrast
        Gdiplus::Rect barRect(barX, barY, barWidth, barHeight);
        graphics.FillRectangle(&barBg, barRect);
        graphics.DrawRectangle(&barOutline, barRect);

        // Filled portion (darker version of background color for subtle contrast)
        int filledHeight = (int)(barHeight * normalizedY);
        int fillY = barY;
        Gdiplus::SolidBrush barFill(Gdiplus::Color(max(0, r - 80), max(0, g - 80), 30));  // Darker version of dynamic color
        Gdiplus::Rect fillRect(barX, fillY, barWidth, filledHeight);
        graphics.FillRectangle(&barFill, fillRect);

        // Small horizontal line in middle (brighter for better visibility)
        int lineY = barY + barHeight / 2;
        Gdiplus::Pen linePen(Gdiplus::Color(255, 255, 255), 2);
        graphics.DrawLine(&linePen, barX, lineY, barX + barWidth, lineY);
    }
}
RECT CalculateHShifterBoundaries()
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

    // Fixed boundaries for entire H-shifter area
    RECT hShifterRect = {
        railX[0].x - 50,    // Increased from 100 to 200 (more left padding)
        topY - 25,          // Increased from 50 to 100 (more top padding)  
        railX[drawRailCount - 1].x + 50,  // Increased from 100 to 200 (more right padding)
        bottomY + 25        // Increased from 50 to 150 (more bottom padding)
    };
    return hShifterRect;
}
// Add this function to capture background brightness from H-Shifter boundaries (DEBUG VERSION)
float GetHShifterBackgroundBrightnessDebug(HWND hwnd)
{
    // Get the H-Shifter boundaries
    RECT hShifterRect = CalculateHShifterBoundaries();

    // Get the screen DC
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    // Create a bitmap to capture the area behind our H-Shifter
    int captureWidth = hShifterRect.right - hShifterRect.left;
    int captureHeight = hShifterRect.bottom - hShifterRect.top;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, captureWidth, captureHeight);
    SelectObject(hdcMem, hBitmap);

    // Copy the screen content from behind our H-Shifter area
    BitBlt(hdcMem, 0, 0, captureWidth, captureHeight,
        hdcScreen, hShifterRect.left, hShifterRect.top, SRCCOPY);

    // Analyze the bitmap for brightness
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int bytesPerPixel = bmp.bmBitsPixel / 8;
    int imageSize = bmp.bmWidth * bmp.bmHeight * bytesPerPixel;

    BYTE* bits = new BYTE[imageSize];
    GetBitmapBits(hBitmap, imageSize, bits);

    // Calculate average brightness (sample every 5th pixel for performance)
    long long totalBrightness = 0;
    int pixelCount = 0;

    for (int y = 0; y < bmp.bmHeight; y += 5) {
        for (int x = 0; x < bmp.bmWidth; x += 5) {
            int offset = (y * bmp.bmWidth + x) * bytesPerPixel;

            if (offset + 2 < imageSize) {
                BYTE blue = bits[offset];
                BYTE green = bits[offset + 1];
                BYTE red = bits[offset + 2];

                // Calculate pixel brightness (using luminance formula)
                float brightness = 0.299f * red + 0.587f * green + 0.114f * blue;
                totalBrightness += (long long)brightness;
                pixelCount++;
            }
        }
    }

    float avgBrightness = pixelCount > 0 ? (float)totalBrightness / pixelCount : 128.0f;

    // Cleanup
    delete[] bits;
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Debug output

    return avgBrightness;
}
// Add global to track previous position
static POINT lastRedrawKnobPos = knobPos;
static POINT lastRedrawGhostPos = ghostKnobPos;
static DWORD lastGhostKnobMoveTime = 0;
static float ghostKnobMoveDistance = 0.0f;
static int ghostRedrawFPS = 60;
// Add these global tracking variables for clutch knob glow
static float lastClutchKnobNorm = 0.0f;
static bool lastVJoyMouseEnabledState = vJoyMouseEnabled;
static DWORD lastClutchKnobRedrawTime = 0;
RECT CalculateClutchKnobRedrawArea()
{
    int padding = knobRadius + 10;

    RECT rect;
    rect.left = knobPos.x - padding;
    rect.top = knobPos.y - padding;
    rect.right = knobPos.x + padding;
    rect.bottom = knobPos.y + padding;

    return rect;
}
static DWORD lastKnobMoveTime = 0;
static float knobMoveDistance = 0.0f;
static int currentRedrawFPS = 60; // Start with high FPS
static POINT lastRedrawGhostKnobPos = ghostKnobPos;

static bool isAggressiveMode = false;
float CalculateKnobMovementIntensity()
{
    float dx = (float)(knobPos.x - lastKnobPos.x);
    float dy = (float)(knobPos.y - lastKnobPos.y);
    return sqrtf(dx * dx + dy * dy);
}
bool ShouldRedrawClutchKnob()
{
    // Check if vJoy state changed
    bool vJoyStateChanged = (vJoyMouseEnabled != lastVJoyMouseEnabledState);

    // Check if clutch position changed significantly
    float currentClutchNorm = 0.0f;
    if (useScrollClutch && vJoyMouseEnabled)
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

    bool clutchChanged = (abs(currentClutchNorm - lastClutchKnobNorm) > 0.01f);

    // Update tracking variables
    lastClutchKnobNorm = currentClutchNorm;
    lastVJoyMouseEnabledState = vJoyMouseEnabled;

    return clutchChanged || vJoyStateChanged;
}
RECT CalculateKnobRedrawArea()
{
    int padding = knobRadius + 10;

    // Calculate area for current position
    RECT currentRect;
    currentRect.left = knobPos.x - padding;
    currentRect.top = knobPos.y - padding;
    currentRect.right = knobPos.x + padding;
    currentRect.bottom = knobPos.y + padding;

    // Calculate area for previous position (to clean up trails)
    RECT prevRect;
    prevRect.left = lastRedrawKnobPos.x - padding;
    prevRect.top = lastRedrawKnobPos.y - padding;
    prevRect.right = lastRedrawKnobPos.x + padding;
    prevRect.bottom = lastRedrawKnobPos.y + padding;

    // Combine both rectangles
    RECT combinedRect;
    combinedRect.left = min(currentRect.left, prevRect.left);
    combinedRect.top = min(currentRect.top, prevRect.top);
    combinedRect.right = max(currentRect.right, prevRect.right);
    combinedRect.bottom = max(currentRect.bottom, prevRect.bottom);

    // Update for next call
    lastRedrawKnobPos = knobPos;

    return combinedRect;
}
RECT CalculateGhostKnobRedrawArea()
{
    int padding = knobRadius + 10; // Same padding as real knob

    // Calculate area for current position
    RECT currentRect;
    currentRect.left = ghostKnobPos.x - padding;
    currentRect.top = ghostKnobPos.y - padding;
    currentRect.right = ghostKnobPos.x + padding;
    currentRect.bottom = ghostKnobPos.y + padding;

    // Calculate area for previous position (to clean up trails)
    RECT prevRect;
    prevRect.left = lastRedrawGhostKnobPos.x - padding;
    prevRect.top = lastRedrawGhostKnobPos.y - padding;
    prevRect.right = lastRedrawGhostKnobPos.x + padding;
    prevRect.bottom = lastRedrawGhostKnobPos.y + padding;

    // Combine both rectangles
    RECT combinedRect;
    combinedRect.left = min(currentRect.left, prevRect.left);
    combinedRect.top = min(currentRect.top, prevRect.top);
    combinedRect.right = max(currentRect.right, prevRect.right);
    combinedRect.bottom = max(currentRect.bottom, prevRect.bottom);

    // Update for next call
    lastRedrawGhostKnobPos = ghostKnobPos;

    return combinedRect;
}
// Add these global tracking variables
static std::string lastActiveGearState = "";
static bool lastKnobMovementEnabledState = knobMovementEnabled;
static std::map<std::string, POINT> lastGearStates; // Track individual gear states
RECT CalculateSingleGearRedrawArea(const std::string& gearName)
{
    POINT pos;
    int padding = gearRadius + 8; // Slightly larger than gear radius

    // Find the gear position
    if (lowerGearPositions.find(gearName) != lowerGearPositions.end())
    {
        pos = lowerGearPositions[gearName];
    }
    else if (highGearPositions.find(gearName) != highGearPositions.end())
    {
        pos = highGearPositions[gearName];
    }
    else
    {
        // Gear not found, return empty rect
        return { 0, 0, 0, 0 };
    }

    RECT rect;
    rect.left = pos.x - padding;
    rect.top = pos.y - padding;
    rect.right = pos.x + padding;
    rect.bottom = pos.y + padding;
    return rect;
}

RECT CalculateAllGearsRedrawArea()
{
    RECT combinedRect = { INT_MAX, INT_MAX, INT_MIN, INT_MIN };

    // Combine all gear positions
    for (auto& kv : lowerGearPositions)
    {
        RECT gearRect = CalculateSingleGearRedrawArea(kv.first);
        combinedRect.left = min(combinedRect.left, gearRect.left);
        combinedRect.top = min(combinedRect.top, gearRect.top);
        combinedRect.right = max(combinedRect.right, gearRect.right);
        combinedRect.bottom = max(combinedRect.bottom, gearRect.bottom);
    }

    for (auto& kv : highGearPositions)
    {
        RECT gearRect = CalculateSingleGearRedrawArea(kv.first);
        combinedRect.left = min(combinedRect.left, gearRect.left);
        combinedRect.top = min(combinedRect.top, gearRect.top);
        combinedRect.right = max(combinedRect.right, gearRect.right);
        combinedRect.bottom = max(combinedRect.bottom, gearRect.bottom);
    }

    // If no gears found, return empty rect
    if (combinedRect.left == INT_MAX)
        return { 0, 0, 0, 0 };

    return combinedRect;
}
// Add these global tracking variables for X-bar
static int lastXBarY = 0;
static int lastIndicatorX = 0;
// Add global to track previous positions
static int lastRedrawXBarY = 0;
static int lastRedrawIndicatorX = 0;
// Add these global tracking variables for Y-bar
static int lastYBarX = 0;
static int lastYBarFillHeight = 0;
static int lastRedrawYBarX = 0;
static int lastRedrawYBarFillHeight = 0;
// Add these global variables for frame throttling
static DWORD lastXBarRedrawTime = 0;
static DWORD lastYBarRedrawTime = 0;
// Add these global tracking variables for clutch bar
static int lastClutchBarX = 0;
static int lastClutchBarFillHeight = 0;
static int lastRedrawClutchBarX = 0;
static int lastRedrawClutchBarFillHeight = 0;
static DWORD lastClutchBarRedrawTime = 0;
const DWORD BAR_REDRAW_INTERVAL = 33; // 1000ms / 30FPS = ~33ms

RECT CalculateXBarRedrawArea()
{
    if (!showXBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Get current X-bar position
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
    int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
    int xBarWidth = static_cast<int>(fullWidth * 0.7f);
    xBarWidth = max(xBarWidth, 50);
    int xBarHeight = 8;
    int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
    int xBarYOffset = 30;
    int xBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
    int xBarLeft = xCenter - xBarWidth / 2;

    // Add padding for the indicator circle
    int indicatorRadius = xBarHeight * 1.5f;
    int padding = indicatorRadius + 5;

    RECT rect;
    rect.left = xBarLeft - padding;
    rect.top = xBarY - padding;
    rect.right = xBarLeft + xBarWidth + padding;
    rect.bottom = xBarY + xBarHeight + padding;

    return rect;
}

RECT CalculateXBarIndicatorRedrawArea()
{
    if (!showXBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Calculate current indicator position
    float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
    normalizedX = max(0.0f, min(1.0f, normalizedX));

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

    int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
    int xBarWidth = static_cast<int>(fullWidth * 0.7f);
    xBarWidth = max(xBarWidth, 50);
    int xBarHeight = 8;
    int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
    int xBarYOffset = 30;
    int xBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
    int xBarLeft = xCenter - xBarWidth / 2;

    int indicatorRadius = xBarHeight * 1.5f;
    int indicatorX = xBarLeft + (int)(normalizedX * xBarWidth);
    int indicatorY = xBarY + xBarHeight / 2;

    // Create rect for indicator with padding
    int padding = indicatorRadius + 2;
    RECT rect;
    rect.left = indicatorX - padding;
    rect.top = indicatorY - padding;
    rect.right = indicatorX + padding;
    rect.bottom = indicatorY + padding;

    return rect;
}

RECT CalculateXBarCompleteRedrawArea()
{
    if (!showXBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Calculate CURRENT X-bar area
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

    int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
    int xBarWidth = static_cast<int>(fullWidth * 0.7f);
    xBarWidth = max(xBarWidth, 50);
    int xBarHeight = 8;
    int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
    int xBarYOffset = 30;
    int currentXBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
    int xBarLeft = xCenter - xBarWidth / 2;

    // Calculate CURRENT indicator position
    float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
    normalizedX = max(0.0f, min(1.0f, normalizedX));
    int currentIndicatorX = xBarLeft + (int)(normalizedX * xBarWidth);

    // Calculate CURRENT areas
    int indicatorRadius = xBarHeight * 1.5f;
    int barPadding = indicatorRadius + 5;
    int indicatorPadding = indicatorRadius + 2;

    // Current X-bar area
    RECT currentBarRect;
    currentBarRect.left = xBarLeft - barPadding;
    currentBarRect.top = currentXBarY - barPadding;
    currentBarRect.right = xBarLeft + xBarWidth + barPadding;
    currentBarRect.bottom = currentXBarY + xBarHeight + barPadding;

    // Current indicator area
    int currentIndicatorY = currentXBarY + xBarHeight / 2;
    RECT currentIndicatorRect;
    currentIndicatorRect.left = currentIndicatorX - indicatorPadding;
    currentIndicatorRect.top = currentIndicatorY - indicatorPadding;
    currentIndicatorRect.right = currentIndicatorX + indicatorPadding;
    currentIndicatorRect.bottom = currentIndicatorY + indicatorPadding;

    // Calculate PREVIOUS areas
    int lastXBarY = lastRedrawXBarY;
    int lastIndicatorX = lastRedrawIndicatorX;

    // Previous X-bar area
    RECT prevBarRect;
    prevBarRect.left = xBarLeft - barPadding;
    prevBarRect.top = lastXBarY - barPadding;
    prevBarRect.right = xBarLeft + xBarWidth + barPadding;
    prevBarRect.bottom = lastXBarY + xBarHeight + barPadding;

    // Previous indicator area
    int lastIndicatorY = lastXBarY + xBarHeight / 2;
    RECT prevIndicatorRect;
    prevIndicatorRect.left = lastIndicatorX - indicatorPadding;
    prevIndicatorRect.top = lastIndicatorY - indicatorPadding;
    prevIndicatorRect.right = lastIndicatorX + indicatorPadding;
    prevIndicatorRect.bottom = lastIndicatorY + indicatorPadding;

    // Combine ALL rectangles (current + previous)
    RECT combinedRect;
    combinedRect.left = min(min(currentBarRect.left, currentIndicatorRect.left), min(prevBarRect.left, prevIndicatorRect.left));
    combinedRect.top = min(min(currentBarRect.top, currentIndicatorRect.top), min(prevBarRect.top, prevIndicatorRect.top));
    combinedRect.right = max(max(currentBarRect.right, currentIndicatorRect.right), max(prevBarRect.right, prevIndicatorRect.right));
    combinedRect.bottom = max(max(currentBarRect.bottom, currentIndicatorRect.bottom), max(prevBarRect.bottom, prevIndicatorRect.bottom));

    // Update for next call
    lastRedrawXBarY = currentXBarY;
    lastRedrawIndicatorX = currentIndicatorX;

    return combinedRect;
}



RECT CalculateYBarRedrawArea()
{
    if (!showYBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Get current Y-bar position
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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int yBarOffset = 20;
    int barY = centerY - barHeight / 2;
    int xBarGap = 20;
    int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

    // Add padding for the filled portion
    int padding = 5;

    RECT rect;
    rect.left = currentYBarX - padding;
    rect.top = barY - padding;
    rect.right = currentYBarX + barWidth + padding;
    rect.bottom = barY + barHeight + padding;

    return rect;
}

RECT CalculateYBarFillRedrawArea()
{
    if (!showYBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Calculate current fill position
    float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
    normalizedY = max(0.0f, min(1.0f, normalizedY));

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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int yBarOffset = 20;
    int barY = centerY - barHeight / 2;
    int xBarGap = 20;
    int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

    int currentFillHeight = (int)(barHeight * normalizedY);
    int fillY = barY;

    // Create rect for fill area with padding
    int padding = 2;
    RECT rect;
    rect.left = currentYBarX - padding;
    rect.top = fillY - padding;
    rect.right = currentYBarX + barWidth + padding;
    rect.bottom = fillY + currentFillHeight + padding;

    return rect;
}

RECT CalculateYBarCompleteRedrawArea()
{
    if (!showYBar || !mouseSteeringEnabled)
        return { 0, 0, 0, 0 };

    // Calculate CURRENT Y-bar position
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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int yBarOffset = 20;
    int barY = centerY - barHeight / 2;
    int xBarGap = 20;
    int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

    // Calculate CURRENT fill position
    float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
    normalizedY = max(0.0f, min(1.0f, normalizedY));
    int currentFillHeight = (int)(barHeight * normalizedY);

    // Calculate CURRENT areas
    int barPadding = 5;
    int fillPadding = 2;

    // Current Y-bar area
    RECT currentBarRect;
    currentBarRect.left = currentYBarX - barPadding;
    currentBarRect.top = barY - barPadding;
    currentBarRect.right = currentYBarX + barWidth + barPadding;
    currentBarRect.bottom = barY + barHeight + barPadding;

    // Current fill area
    RECT currentFillRect;
    currentFillRect.left = currentYBarX - fillPadding;
    currentFillRect.top = barY - fillPadding;
    currentFillRect.right = currentYBarX + barWidth + fillPadding;
    currentFillRect.bottom = barY + currentFillHeight + fillPadding;

    // Calculate PREVIOUS areas
    int lastYBarX = lastRedrawYBarX;
    int lastFillHeight = lastRedrawYBarFillHeight;

    // Previous Y-bar area
    RECT prevBarRect;
    prevBarRect.left = lastYBarX - barPadding;
    prevBarRect.top = barY - barPadding;
    prevBarRect.right = lastYBarX + barWidth + barPadding;
    prevBarRect.bottom = barY + barHeight + barPadding;

    // Previous fill area
    RECT prevFillRect;
    prevFillRect.left = lastYBarX - fillPadding;
    prevFillRect.top = barY - fillPadding;
    prevFillRect.right = lastYBarX + barWidth + fillPadding;
    prevFillRect.bottom = barY + lastFillHeight + fillPadding;

    // Combine ALL rectangles (current + previous)
    RECT combinedRect;
    combinedRect.left = min(min(currentBarRect.left, currentFillRect.left), min(prevBarRect.left, prevFillRect.left));
    combinedRect.top = min(min(currentBarRect.top, currentFillRect.top), min(prevBarRect.top, prevFillRect.top));
    combinedRect.right = max(max(currentBarRect.right, currentFillRect.right), max(prevBarRect.right, prevFillRect.right));
    combinedRect.bottom = max(max(currentBarRect.bottom, currentFillRect.bottom), max(prevBarRect.bottom, prevFillRect.bottom));

    // Update for next call
    lastRedrawYBarX = currentYBarX;
    lastRedrawYBarFillHeight = currentFillHeight;

    return combinedRect;
}
RECT CalculateClutchBarRedrawArea()
{
    if (!showClutchIndicator || !useScrollClutch)
        return { 0, 0, 0, 0 };

    // Get current clutch bar position
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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int xBarGap = 40;
    int barX = lastRailX + 50;

    // If knob is too close, push clutch bar further right
    barX = max(barX, knobPos.x + knobRadius + xBarGap);

    // If Y-bar is also shown, push clutch bar further away from Y-bar
    if (showYBar && mouseSteeringEnabled)
        barX = max(barX, lastRailX + 50 + xBarGap);

    int barY = centerY - barHeight / 2;

    // Add padding
    int padding = 5;

    RECT rect;
    rect.left = barX - padding;
    rect.top = barY - padding;
    rect.right = barX + barWidth + padding;
    rect.bottom = barY + barHeight + padding;

    return rect;
}

RECT CalculateClutchBarFillRedrawArea()
{
    if (!showClutchIndicator || !useScrollClutch)
        return { 0, 0, 0, 0 };

    // Calculate current fill position
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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int xBarGap = 40;
    int barX = lastRailX + 50;
    barX = max(barX, knobPos.x + knobRadius + xBarGap);
    if (showYBar && mouseSteeringEnabled)
        barX = max(barX, lastRailX + 50 + xBarGap);

    int barY = centerY - barHeight / 2;
    int currentFillHeight = (int)(barHeight * normalizedRx);
    int fillY = barY;

    // Create rect for fill area with padding
    int padding = 2;
    RECT rect;
    rect.left = barX - padding;
    rect.top = fillY - padding;
    rect.right = barX + barWidth + padding;
    rect.bottom = fillY + currentFillHeight + padding;

    return rect;
}

RECT CalculateClutchBarCompleteRedrawArea()
{
    if (!showClutchIndicator || !useScrollClutch)
        return { 0, 0, 0, 0 };

    // Calculate CURRENT clutch bar position
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

    int barWidth = 8;
    int barHeight = bottomY - topY;
    int lastRailX = railX[drawRailCount - 1].x;
    int xBarGap = 40;
    int currentClutchBarX = lastRailX + 50;
    currentClutchBarX = max(currentClutchBarX, knobPos.x + knobRadius + xBarGap);
    if (showYBar && mouseSteeringEnabled)
        currentClutchBarX = max(currentClutchBarX, lastRailX + 50 + xBarGap);

    int barY = centerY - barHeight / 2;

    // Calculate CURRENT fill position
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
    int currentFillHeight = (int)(barHeight * normalizedRx);

    // Calculate CURRENT areas
    int barPadding = 5;
    int fillPadding = 2;

    // Current clutch bar area
    RECT currentBarRect;
    currentBarRect.left = currentClutchBarX - barPadding;
    currentBarRect.top = barY - barPadding;
    currentBarRect.right = currentClutchBarX + barWidth + barPadding;
    currentBarRect.bottom = barY + barHeight + barPadding;

    // Current fill area
    RECT currentFillRect;
    currentFillRect.left = currentClutchBarX - fillPadding;
    currentFillRect.top = barY - fillPadding;
    currentFillRect.right = currentClutchBarX + barWidth + fillPadding;
    currentFillRect.bottom = barY + currentFillHeight + fillPadding;

    // Calculate PREVIOUS areas
    int lastClutchBarX = lastRedrawClutchBarX;
    int lastFillHeight = lastRedrawClutchBarFillHeight;

    // Previous clutch bar area
    RECT prevBarRect;
    prevBarRect.left = lastClutchBarX - barPadding;
    prevBarRect.top = barY - barPadding;
    prevBarRect.right = lastClutchBarX + barWidth + barPadding;
    prevBarRect.bottom = barY + barHeight + barPadding;

    // Previous fill area
    RECT prevFillRect;
    prevFillRect.left = lastClutchBarX - fillPadding;
    prevFillRect.top = barY - fillPadding;
    prevFillRect.right = lastClutchBarX + barWidth + fillPadding;
    prevFillRect.bottom = barY + lastFillHeight + fillPadding;

    // Combine ALL rectangles (current + previous)
    RECT combinedRect;
    combinedRect.left = min(min(currentBarRect.left, currentFillRect.left), min(prevBarRect.left, prevFillRect.left));
    combinedRect.top = min(min(currentBarRect.top, currentFillRect.top), min(prevBarRect.top, prevFillRect.top));
    combinedRect.right = max(max(currentBarRect.right, currentFillRect.right), max(prevBarRect.right, prevFillRect.right));
    combinedRect.bottom = max(max(currentBarRect.bottom, currentFillRect.bottom), max(prevBarRect.bottom, prevFillRect.bottom));

    // Update for next call
    lastRedrawClutchBarX = currentClutchBarX;
    lastRedrawClutchBarFillHeight = currentFillHeight;

    return combinedRect;
}


// --- REVERSE LOCK FIX ---

// Helper to check unlock state (since original function might be inaccessible)
bool IsReverseUnlockActive_Fix()
{
    bool active = false;
    // Check keyboard override
    if (g_reverseUnlockType == TOGGLE_KEYBOARD) {
        if (g_reverseUnlockKey == 0) active = false;
        else active = (GetAsyncKeyState(g_reverseUnlockKey) & 0x8000) != 0;
    }
    // Check mouse buttons
    if (g_reverseUnlockType == TOGGLE_MOUSE_LEFT) active = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    else if (g_reverseUnlockType == TOGGLE_MOUSE_RIGHT) active = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    else if (g_reverseUnlockType == TOGGLE_MOUSE_MIDDLE) active = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    else if (g_reverseUnlockType == TOGGLE_MOUSE_BUTTON4) active = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    else if (g_reverseUnlockType == TOGGLE_MOUSE_BUTTON5) active = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
    
    if (active) g_reverseAuthorizedOnRail = true;
    
    return active || g_reverseAuthorizedOnRail;
}

void ClampMovementForReverseLock_Fix(int& newY, int currentX)
{
    if (!reverseLockEnabled || IsReverseUnlockActive_Fix())
        return;

    std::map<std::string, POINT>& activeMap = is16GearSet ? highGearPositions : lowerGearPositions;

    for (auto& kv : activeMap)
    {
        if (kv.first == "R")
        {
            int reverseX = kv.second.x;
            int reverseY = kv.second.y;
            int safetyMargin = 30; // Zone where we protect reverse entry

            // If aligned horizontally with Reverse
            if (abs(currentX - reverseX) < safetyMargin)
            {
                // Reverse is at Top
                if (reverseY == topY)
                {
                    // If trying to move UP into Reverse zone (Y < centerY)
                    // We clamp Y to be below the danger zone
                    int limitY = centerY - 20; // 20px above center allowed
                    if (newY < limitY)
                        newY = limitY;
                }
                // Reverse is at Bottom
                else if (reverseY == bottomY)
                {
                    int limitY = centerY + 20;
                    if (newY > limitY)
                        newY = limitY;
                }
            }
        }
    }
}

void ClampHorizontalMovementForReverseLock_Fix(long& newX, int currentX, int currentY)
{
    if (!reverseLockEnabled || IsReverseUnlockActive_Fix())
        return;

    std::map<std::string, POINT>& activeMap = is16GearSet ? highGearPositions : lowerGearPositions;

    for (auto& kv : activeMap)
    {
        if (kv.first == "R")
        {
            int reverseX = kv.second.x;
            int reverseY = kv.second.y;
            
            // Check if we are in the vertical danger zone (level with Reverse)
            bool inDangerZone = false;
            // Case 1: Reverse at Top
            if (reverseY == topY && currentY < centerY - 20) inDangerZone = true;
            // Case 2: Reverse at Bottom
            if (reverseY == bottomY && currentY > centerY + 20) inDangerZone = true;

            if (inDangerZone)
            {
                // Prevent X from entering Reverse X zone
                int safeDist = 30; // 1/2 of inter-rail distance approx
                
                // If we are currently LEFT of Reverse
                if (currentX < reverseX - safeDist)
                {
                     if (newX > reverseX - safeDist)
                         newX = reverseX - safeDist;
                }
                // If we are currently RIGHT of Reverse
                if (currentX > reverseX + safeDist)
                {
                    if (newX < reverseX + safeDist)
                        newX = reverseX + safeDist;
                }
            }
        }
    }
}

// Ensure final position is valid regarding Reverse Lock
void EnforceReverseLockBoundary(POINT& pt)
{
    if (!reverseLockEnabled || IsReverseUnlockActive_Fix())
        return;

    std::map<std::string, POINT>& activeMap = is16GearSet ? highGearPositions : lowerGearPositions;

    for (auto& kv : activeMap)
    {
        if (kv.first == "R")
        {
            int reverseX = kv.second.x;
            int reverseY = kv.second.y;
            
            // Define strict Forbidden Zone
            // If Reverse is at TOP: forbids y < centerY - 20 IF x is within range
            // If Reverse is at BOTTOM: forbids y > centerY + 20 IF x is within range
            
            int forbiddenXRange = 35; // slightly wider than the clamp safety margin
            
            if (abs(pt.x - reverseX) < forbiddenXRange)
            {
                if (reverseY == topY)
                {
                    int safeY = centerY - 20;
                    if (pt.y < safeY)
                        pt.y = safeY;
                }
                else if (reverseY == bottomY)
                {
                    int safeY = centerY + 20;
                    if (pt.y > safeY)
                        pt.y = safeY;
                }
            }
        }
    }
}
// ------------------------

void ProcessRawInput(RAWINPUT* raw)
{
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        RAWKEYBOARD& kb = raw->data.keyboard;

        // F11 toggle using raw input
        static bool wasF11Down = false;
        if (kb.VKey == VK_F11)
        {
            bool isDown = (kb.Message == WM_KEYDOWN || kb.Message == WM_SYSKEYDOWN);
            if (isDown && !wasF11Down)
            {
                vJoyMouseEnabled = !vJoyMouseEnabled; // toggle on press
            }
            wasF11Down = isDown;
        }
    }

    for (auto& mapping : inputMap)
    {
        if (mapping.type == KEYBOARD && raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            RAWKEYBOARD& kb = raw->data.keyboard;

            if (kb.Message == WM_KEYDOWN && kb.VKey == mapping.code)
            {
                SetBtn(true, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = true;  // <--- add this
            }
            else if (kb.Message == WM_KEYUP && kb.VKey == mapping.code)
            {
                SetBtn(false, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = false; // <--- add this
            }

        }
        else if (mapping.type == MOUSE && raw->header.dwType == RIM_TYPEMOUSE)
        {
            RAWMOUSE& rm = raw->data.mouse;
            bool pressed = false;
            bool released = false;

            if (mapping.code == 1)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP;
            }
            else if (mapping.code == 2)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP;
            }
            else if (mapping.code == 3)
            {
                pressed = rm.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP;
            }
            else if (mapping.code == 4) {
                pressed = rm.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_BUTTON_4_UP;
            }
            else if (mapping.code == 5) {
                pressed = rm.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN;
                released = rm.usButtonFlags & RI_MOUSE_BUTTON_5_UP;
            }


            if (pressed)
            {
                SetBtn(true, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = true; // mark pressed
            }
            if (released)
            {
                SetBtn(false, vjoyDeviceId, mapping.vjoyButton);
                buttonPressedState[mapping.vjoyButton] = false; // mark released
            }

        }
    }
}

void UpdateKnobMovement(HWND hwnd)
{
    if (!mouseSteeringEnabled)
        return;

    bool sameDevice = (g_selectedDevice != NULL && g_selectedSteeringDevice != NULL &&
        g_selectedDevice == g_selectedSteeringDevice);
    if (!sameDevice)
    {
        if (knobDisabledByF9)
        {
            // When disabled by F9, only allow temporary enable with toggle
            knobMovementEnabled = IsKnobToggleActive();
            mouseTrackingEnabled = !IsKnobToggleActive();
        }
        else
        {
            // Normal case: knob always enabled + mouse tracking
            knobMovementEnabled = true;
            mouseTrackingEnabled = true;
        }
    }

    else
    {
        // Devices are the same: use Shift key logic
        if (IsKnobToggleActive())
        {
            knobMovementEnabled = true;
            mouseTrackingEnabled = false;
        }
        else
        {
            knobMovementEnabled = false;
            mouseTrackingEnabled = true;
        }
    }
}
// Variables now defined at top of file


#include <mutex>
#include <chrono>

std::mutex rumbleMutex;
std::chrono::steady_clock::time_point lastRumbleTime;

void SafeRumble(int left, int right, int durationMs)
{
    std::lock_guard<std::mutex> lock(rumbleMutex);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRumbleTime).count();

    if (elapsed < 60)
        return; // debounce rumble spam

    for (auto& g : g_gamepads)
    {
        if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, 0, 0, 0);
            SDL_HapticStopAll(g.haptic);
        }

        if (g.haptic)
        {
            SDL_HapticRumblePlay(g.haptic, (std::max)(left, right) / 65535.0f, durationMs);
        }
        else if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, left, right, durationMs);
        }
    }

    lastRumbleTime = now;
}



void UpdateKnobFromXInput(HWND hwnd)
{
    if (!useXInput)
        return;
    float dx = 0.0f, dy = 0.0f;
    g = nullptr; // reset at start
    if (PlayStationMode && g_selectedGamepadIndex >= 0 && g_selectedGamepadIndex < g_gamepads.size()) {
        SDL_GameControllerUpdate();
        g = &g_gamepads[g_selectedGamepadIndex]; // use global pointer



        int LX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_LEFTX);
        int LY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_LEFTY);
        int RX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int RY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTY);

        // Deadzone
        const float DEADZONE = 1000.0f;
        auto fixAxis = [DEADZONE](int v) { return (abs(v) < DEADZONE) ? 0 : (float)v; };
        float fx = fixAxis(useRightStick ? RX : LX);
        float fy = fixAxis(useRightStick ? RY : LY);
        if (invertPlayStationYAxis) fy = -fy;

        // Apply acceleration
        const float BASE_SENSITIVITY = 0.25f;
        const float ACCEL_FACTOR = 5.5f;
        float magnitude = sqrt(fx * fx + fy * fy);
        if (magnitude > DEADZONE) {
            float normX = fx / magnitude;
            float normY = fy / magnitude;
            float adjustedMag = pow(magnitude / 32767.0f, ACCEL_FACTOR) * BASE_SENSITIVITY * controllerSensMultiplier * 50.0f;
            dx = normX * adjustedMag;
            dy = normY * adjustedMag;
        }
        else {
            dx = 0.0f;
            dy = 0.0f;
        }



        if (assistButtonBeingSet && g) { // only check buttons if g is valid
            for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; ++b) {
                if (SDL_GameControllerGetButton(g->controller, (SDL_GameControllerButton)b)) {
                    assistButton = b;              // store SDL button index, not a mask
                    assistButtonBeingSet = false;
                    break;                         // stop after first detected button
                }
            }
        }

    }










    // --- Check assist buttons (RB / LB) ---
    if (g) {
        assistButtonHeld = SDL_GameControllerGetButton(
            g->controller,
            (SDL_GameControllerButton)assistButton
        );
    }



    // If assist just pressed, initialize ghost knob at real knob
    if (invertAssistAxes)
    {
        // We don't want ghost knob at all
        useAssistPointer = false; // ignore ghost knob completely
    }
    else
    {
        // original logic
        if (assistButtonHeld && !useAssistPointer)
        {
            ghostKnobPos = knobPos;
            ghostSnappedGear = "";
        }
        useAssistPointer = assistButtonHeld;
    }



    // PlayStationMode already calculated dx/dy above, so we use those values

    // --- Unified knob handling (ghost + real) ---
    POINT* targetKnob = &knobPos; // default to real knob

    if (invertAssistAxes && disableRealKnobMovement)
    {
        // Real knob always moves, unless assist button is held
        if (assistButtonHeld)
        {
            dx = 0.0f;
            dy = 0.0f;
        }
    }
    else
    {
        // original logic for non-inverted
        if (useAssistPointer)
            targetKnob = &ghostKnobPos;

        if (disableRealKnobMovement)
        {
            targetKnob = &ghostKnobPos;
            if (!useAssistPointer)
            {
                dx = 0.0f;
                dy = 0.0f;
            }
        }
    }

    // --- Snap detection for ghost / targetKnob ---
// --- Snap detection for ghost / targetKnob ---
    auto& activeMap = lowerGearPositions;
    std::string snappedGear = "";

    if (layoutType == 11) // PRNDL layout
    {
        // For PRNDL, only consider vertical distance for snapping
        float closestDist = FLT_MAX;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;

            // For PRNDL, only vertical distance matters
            double dist = abs(targetKnob->y - g.y);
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                snappedGear = gear;
                closestDist = dist;
            }
        }
    }
    else
    {
        // Regular diamond layout snapping
        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
                (targetKnob->y - g.y) * (targetKnob->y - g.y));
            if (dist < gearSnapInThreshold)
            {
                snappedGear = gear;
                break;
            }
        }
    }

    bool isSnapped = !snappedGear.empty();

    if (isSnapped)
    {
        const int maxHorizontalOffset = 20;
        POINT g = activeMap[snappedGear];
        targetKnob->x += (int)roundf(dx * 0.2f);
        if (targetKnob->x < g.x - maxHorizontalOffset)
            targetKnob->x = g.x - maxHorizontalOffset;
        if (targetKnob->x > g.x + maxHorizontalOffset)
            targetKnob->x = g.x + maxHorizontalOffset;
        targetKnob->y += (int)roundf(dy);

        // --- Only update gear if targetKnob is the real knob ---
        // --- Only update gear if targetKnob is the real knob ---
        if (!useAssistPointer)
        {
            SetGearKey(snappedGear);
            activeGear = snappedGear;
            lockedInGear = true;

            // RELEASE NEUTRAL immediately
            if (neutralHeld)
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
                else if (gi.type == VJOY_BUTTON)
                {
                    SetBtn(false, vjoyDeviceId, gi.code);
                }
                neutralHeld = false;
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
            }
        }
    }

    else
    {
        if (layoutType == 11) // PRNDL layout
        {
            // PRNDL: Vertical movement only with strong horizontal centering
            targetKnob->y += (int)roundf(dy);
            targetKnob->x += (int)roundf(dx * 0.1f); // Minimal horizontal movement
            targetKnob->x += (int)roundf((railX[0].x - targetKnob->x) * 0.3f); // Strong center pull

            // Reset rail state for PRNDL (no rail switching)
            currentRail = VERTICAL;
            currentVerticalIndex = 0;
        }
        else
        {
            // --- Not snapped: regular rail + lerp logic ---
            bool insideIntersection = IsInsideIntersection(targetKnob->x, targetKnob->y);
            if (currentRail == HORIZONTAL)
            {
                targetKnob->x += (int)roundf(dx);

                if (insideIntersection)
                {
                    targetKnob->y += (int)roundf(dy);

                    int railCount = is16GearSet ? 5 : 4;
                    for (int i = 0; i < railCount; ++i)
                    {
                        int railXPos = railX[i].x;
                        if (abs(targetKnob->x - railXPos) < enterVerticalThreshold && abs(dy) > 0)
                        {
                            bool movingDown = dy > 0;
                            bool movingUp = dy < 0;
                            bool canMoveVertically = false;

                            for (auto& kv : activeMap)
                            {
                                if (kv.second.x == railXPos)
                                {
                                    if (movingDown && kv.second.y == bottomY)
                                        canMoveVertically = true;
                                    if (movingUp && kv.second.y == topY)
                                        canMoveVertically = true;
                                }
                            }

                            if (canMoveVertically)
                            {
                                currentRail = VERTICAL;
                                currentVerticalIndex = i;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    targetKnob->y += (int)roundf(dy * 0.2f);
                    targetKnob->y += (int)roundf((centerY - targetKnob->y) * 0.05f);
                }
            }
            else if (currentRail == VERTICAL && currentVerticalIndex != -1)
            {
                targetKnob->y += (int)roundf(dy);
                int railCenter = railX[currentVerticalIndex].x;
                targetKnob->x += (int)roundf(dx * 0.2f);
                targetKnob->x += (int)roundf((railCenter - targetKnob->x) * 0.07f);

                if (IsInsideIntersection(targetKnob->x, targetKnob->y) && abs(dx) > 0)
                {
                    currentRail = HORIZONTAL;
                    currentVerticalIndex = -1;
                }
            }
        }
    }

    // --- Clamp knob ---
    if (targetKnob->x < knobMinX)
        targetKnob->x = knobMinX;
    if (targetKnob->x > knobMaxX)
        targetKnob->x = knobMaxX;
    if (targetKnob->y < knobMinY)
        targetKnob->y = knobMinY;
    if (targetKnob->y > knobMaxY)
        targetKnob->y = knobMaxY;

    // --- Snap knob ---
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
            (targetKnob->y - g.y) * (targetKnob->y - g.y));
        if (dist < gearSnapInThreshold)
        {
            snappedGear = gear;
            break;
        }
    }

    // --- Handle ghost teleport if assist released ---
    // --- Animate real knob toward ghost knob if assist released ---
    if (!useAssistPointer && !ghostSnappedGear.empty())
    {
        POINT target = lowerGearPositions[ghostSnappedGear];

        // Linear interpolation for smooth movement
        float lerpFactor = 0.50f; // tweak for speed
        knobPos.x = knobPos.x + (int)roundf((target.x - knobPos.x) * lerpFactor);
        knobPos.y = knobPos.y + (int)roundf((target.y - knobPos.y) * lerpFactor);

        // Check if close enough to snap to target
        if (abs(knobPos.x - target.x) < 2 && abs(knobPos.y - target.y) < 2)
        {
            knobPos = target;
            SetGearKey(ghostSnappedGear);
            activeGear = ghostSnappedGear;
            lockedInGear = true;

            // Release neutral key now that we're at the gear
            if (neutralHeld)
            {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = gearInputMap["N"].code;
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                neutralHeld = false;
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
            }

            ghostSnappedGear = "";
        }
    }

    if (useAssistPointer)
    {
        // Handle snapping for ghost knob with hysteresis (like real knob)
        std::string gearToSnap = "";
        double closestDist = 1e9;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((ghostKnobPos.x - g.x) * (ghostKnobPos.x - g.x) +
                (ghostKnobPos.y - g.y) * (ghostKnobPos.y - g.y));
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                closestDist = dist;
                gearToSnap = gear;
            }
        }

        if (!ghostKnobSnapped && !gearToSnap.empty())
        {
            ghostKnobPos = activeMap[gearToSnap];
            ghostSnappedGear = gearToSnap;
            ghostKnobSnapped = true;       // you need to define this boolean similar to realKnobSnapped
// Replace: PulseVibrationAsync(0, 65535);
            SafeRumble(0, 65535, 100); // ghost knob
        }
        else if (ghostKnobSnapped && gearToSnap.empty())
        {
            ghostSnappedGear = "";
            ghostKnobSnapped = false;
        }
    }

    // --- Handle snapping for real knob with hysteresis ---
    std::string gearToSnap = "";
    double closestDist = 1e9;

    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold && dist < closestDist)
        {
            closestDist = dist;
            gearToSnap = gear;
        }
    }

    if (!realKnobSnapped && !gearToSnap.empty())
    {
        knobPos = activeMap[gearToSnap];
        SetGearKey(gearToSnap);
        activeGear = gearToSnap;
        lockedInGear = true;
        realKnobSnapped = true;
        // Replace: PulseVibrationAsync(65535, 0);
        SafeRumble(65535, 0, 100); // real knob

        if (neutralHeld)
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
            else if (gi.type == VJOY_BUTTON)
            {
                SetBtn(false, vjoyDeviceId, gi.code);
            }
            neutralHeld = false;
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
        }
    }

    else if (realKnobSnapped && gearToSnap.empty())
    {
        ReleaseGearKey();
        activeGear = "";
        lockedInGear = false;
        realKnobSnapped = false;
    }

    // --- Handle Neutral for real knob ---
    bool inNeutral = true;
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold)
        {
            inNeutral = false;
            break;
        }
    }

    if (inNeutral && isNeutralEnabled && !neutralHeld)
    {
        GearInput gi = gearInputMap["N"];
        if (gi.type == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = gi.code;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(true, vjoyDeviceId, gi.code);
        }
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
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(false, vjoyDeviceId, gi.code);
        }
        // === STOP NEUTRAL GLOW ===
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
        else {
            OutputDebugStringA("=== NEUTRAL GLOW NOT FOUND IN ANIMATIONS ===\n");
        }


        neutralHeld = false;
    }

    // --- DEBUG ---

    // --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
    if (isBorderless && isTransparent && dynamicTransparencyEnabled)
    {
        static DWORD lastActiveTime = 0;
        static DWORD lastAlphaUpdate = 0;
        static DWORD lastBrightnessCheck = 0;
        static BYTE dynamicMinAlpha = minAlpha;
        static BYTE lastAppliedMinAlpha = minAlpha;
        static BYTE lastUserMinAlpha = minAlpha;

        // === ADD HISTORY TRACKING FOR CONTROLLER ===
        static std::vector<BYTE> minAlphaHistory;
        static const int HISTORY_SIZE = 3;
        static const BYTE CHANGE_THRESHOLD = 8;

        DWORD now = GetTickCount();

        // Update if user changed the slider
        if (lastUserMinAlpha != minAlpha) {
            lastUserMinAlpha = minAlpha;
            dynamicMinAlpha = minAlpha;
            lastAppliedMinAlpha = minAlpha;
            minAlphaHistory.clear(); // Clear history on user change
            lastBrightnessCheck = 0;
            lastAlphaUpdate = 0;
        }

        // Check background brightness at 2 FPS (every 500ms) - SAME AS MOUSE VERSION
        if (now - lastBrightnessCheck > 1500)
        {
            float brightness = GetHShifterBackgroundBrightnessDebug(hwnd);

            // Linear mapping: brightness 0-255 maps to brightnessFactor 0.2-1.0
            float brightnessFactor = 0.2f + (brightness / 255.0f) * 0.8f;
            brightnessFactor = max(0.2f, min(1.0f, brightnessFactor));

            BYTE newDynamicMinAlpha = (BYTE)(minAlpha * brightnessFactor);
            newDynamicMinAlpha = max(10, min(150, newDynamicMinAlpha));

            // Add to history
            minAlphaHistory.push_back(newDynamicMinAlpha);
            if (minAlphaHistory.size() > HISTORY_SIZE)
                minAlphaHistory.erase(minAlphaHistory.begin());

            // Smart update check using history (SAME LOGIC AS MOUSE)
            bool shouldUpdate = false;

            if (minAlphaHistory.size() == HISTORY_SIZE)
            {
                int sum = 0;
                for (BYTE val : minAlphaHistory) sum += val;
                BYTE recentAverage = sum / minAlphaHistory.size();

                // Only update if significant deviation from recent trend
                if (abs((int)newDynamicMinAlpha - (int)recentAverage) >= CHANGE_THRESHOLD)
                {
                    shouldUpdate = true;
                }
                else
                {
                    // Check if all recent values are similar (stable state)
                    bool allSimilar = true;
                    for (BYTE val : minAlphaHistory)
                    {
                        if (abs((int)val - (int)newDynamicMinAlpha) > (CHANGE_THRESHOLD / 2))
                        {
                            allSimilar = false;
                            break;
                        }
                    }
                    // If values are stable and different from current dynamicMinAlpha, update
                    if (allSimilar && abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD)
                    {
                        shouldUpdate = true;
                    }
                }
            }
            else
            {
                // Not enough history, use simple threshold
                shouldUpdate = (abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD);
            }

            if (shouldUpdate)
            {
                dynamicMinAlpha = newDynamicMinAlpha;
                lastAlphaUpdate = 0; // Force redraw on next frame
            }

            lastBrightnessCheck = now;
        }

        // AGGRESSIVE 15 FPS LIMIT (66ms between updates)
        if (now - lastAlphaUpdate >= 66)
        {
            // Detect activity: LB/RB held OR slight knob movement
            bool inputActive = (abs(dx) > 0.1f || abs(dy) > 0.1f);
            if (!invertAssistAxes)
                inputActive = inputActive || assistButtonHeld;

            if (inputActive)
            {
                currentAlpha = maxAlpha;
                lastActiveTime = now; // reset timer
                SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                lastAppliedMinAlpha = dynamicMinAlpha; // Track what's applied
            }
            else if (now - lastActiveTime >= transparencyFadeDelay)
            {
                // Fade back to DYNAMIC minAlpha (not static minAlpha) smoothly
                BYTE newAlpha = BYTE(dynamicMinAlpha + (currentAlpha - dynamicMinAlpha) * alphaDecay);

                // Only update if alpha actually changed OR if dynamicMinAlpha changed
                if (newAlpha != currentAlpha || dynamicMinAlpha != lastAppliedMinAlpha)
                {
                    currentAlpha = newAlpha;
                    lastAppliedMinAlpha = dynamicMinAlpha; // Track what we're applying
                    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                }
            }

            lastAlphaUpdate = now; // Update FPS timer
        }
    }


}
#include <chrono>

// Globals
std::chrono::steady_clock::time_point rbReleaseTime;
bool rbWasPressed = false;
const int RB_DELAY_MS = 40;
void UpdateVJoyFromXInput()
{
    if (g_selectedGamepadIndex < 0 || g_selectedGamepadIndex >= g_gamepads.size())
        return;

    Gamepad* g = &g_gamepads[g_selectedGamepadIndex];
    if (!g->controller)
        return;

    SDL_GameControllerUpdate();

    // --- LT as clutch ---
    if (useLTAsClutch && !useThrottleBrakeAxes)
    {
        // SDL axis range is -32768 → 32767 for triggers too, but SDL triggers usually 0–32767
        int ltValue = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);

        LONG zAxisValue;
        if (bindingModeForRAxis && ltValue > 1000) // Little bit of movement detection for clutch
        {
            zAxisValue = axisMax; // Full value when binding mode is active
        }
        else
        {
            zAxisValue = axisMin + (ltValue / 32767.0) * (axisMax - axisMin);
        }
        SetAxis(zAxisValue, vjoyDeviceId, HID_USAGE_Z);
    }

    if (!disableRealKnobMovement)
        return;

    // --- Assist button ---
    bool rbPressed = false;
    if (assistButton >= 0 && assistButton < SDL_CONTROLLER_BUTTON_MAX)
        rbPressed = SDL_GameControllerGetButton(g->controller, (SDL_GameControllerButton)assistButton) != 0;

    // --- Inverted assist axis logic ---
    if (invertAssistAxes)
    {
        if (!rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            return;
        }
        // else → assist pressed → continue normal stick processing
    }
    else
    {
        if (rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            rbWasPressed = true;
            return;
        }
        else if (rbWasPressed)
        {
            rbReleaseTime = std::chrono::steady_clock::now();
            rbWasPressed = false;
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if ((now - rbReleaseTime) < std::chrono::milliseconds(RB_DELAY_MS))
            return;
    }

    // --- Process right stick normally ---
    int sThumbRX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTX);
    int sThumbRY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTY);

    double normRX = 0.0, normRY = 0.0;
    const int DEADZONE = 8000; // adjust as needed

    if (bindingModeForRAxis)
    {
        // Binding mode: go to full value with little movement
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = (sThumbRX > 0) ? 1.0 : -1.0; // Full value
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = (sThumbRY > 0) ? 1.0 : -1.0; // Full value
    }
    else
    {
        // Normal mode
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = sThumbRX / 32767.0;
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = sThumbRY / 32767.0;
    }

    joyRX = (LONG)(axisMin + (normRX + 1.0) * 0.5 * (axisMax - axisMin));
    joyRY = (LONG)(axisMin + (normRY + 1.0) * 0.5 * (axisMax - axisMin));

    SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
    SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
}
void PollPedalBinding(HWND hwnd)
{
    if (!togglePedalBeingSet || !g_hasPedals || !g_pJoystick)
        return;

    PollDirectInput();

    TCHAR buf[256];
    //_stprintf_s(buf, _T("Polling Pedals -> Clutch: %ld, Brake: %ld, Accel: %ld\n"),
        //g_diState.lZ, g_diState.lY, g_diState.lRz);
    //OutputDebugString(buf);

    if (g_diState.lZ > 32767) // clutch
    {
        g_knobToggleType = TOGGLE_PEDAL_CLUTCH;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Clutch pedal bound!\n"));
    }
    else if (g_diState.lY > 32767) // brake
    {
        g_knobToggleType = TOGGLE_PEDAL_BRAKE;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Brake pedal bound!\n"));
    }
    else if (g_diState.lRz > 32767) // accelerator
    {
        g_knobToggleType = TOGGLE_PEDAL_ACCEL;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Accelerator pedal bound!\n"));
    }
}

inline int ApplyKnobMotion(int delta, float sensitivity) {
    if (!knobAccelerationEnabled)
        return int(delta * sensitivity);

    // Base acceleration: small effect from delta
    float baseAccel = 1.0f + fabs(delta) * 0.01f; // reduced from 0.02 to soften

    // Nonlinear sensitivity factor: compress low end, boost high end
    float sensFactor = powf(sensitivity, 0.7f); // tweak exponent for feel

    // Final acceleration
    float accel = baseAccel * sensFactor;

    return int(delta * accel);
}





// ---------------- Window Proc ----------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        ComputeLayout(hwnd);

        ComputeIntersections();
        for (auto& inter : intersections)
        {
            char dbg[128];
            sprintf_s(dbg, "[DEBUG] diamond intersection center=(%d,%d) radius=%d\n",
                inter.x, inter.y, inter.radius);
            OutputDebugStringA(dbg);
        }

        if (!rawInputInitialized)
            InitRawInput(hwnd);
        if (!InitVJoy())
        {
            PostQuitMessage(1);
            return 0;
        }
        {
            int vJoyButtonCount = GetVJoyButtonCount(vjoyDeviceId);
            char dbg[128];
            sprintf_s(dbg, "[vJoy] Device %d has %d buttons\n", vjoyDeviceId, vJoyButtonCount);
            OutputDebugStringA(dbg);
        }
        // ----- ADD DIRECTINPUT INITIALIZATION HERE -----
        if (!InitDirectInput(hwnd))
        {
            OutputDebugStringA("[DirectInput] Failed to initialize\n");
        }
        else
        {
            if (!SetupJoystick(hwnd))
            {
                OutputDebugStringA("[DirectInput] No joystick/pedals detected\n");
            }
        }
        InitGamepads();
        SDL_InitSubSystem(SDL_INIT_EVENTS); // make sure events are enabled
        StartThrottleBrakeThread();

        SetTimer(hwnd, 1, 10, nullptr); // 10 ms = ~100 FPS max achievable
        RefreshProcessList();  // <-- ADD THIS LINE HERE
        tooltips.resize(60);
        // Update check thread - now safely scoped
        std::thread updateThread([]() {
            CheckForUpdates();
            });
        updateThread.detach();

        // SetTimer(hwnd, 1, 16, nullptr); // 60FPS
        break;
    }

    case WM_INPUT:
    {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

        BYTE* lpb = new BYTE[dwSize];
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
        {
            RAWINPUT* raw = (RAWINPUT*)lpb;
            ProcessRawInput(raw); // <-- handles keyboard/mouse → vJoy mapping

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

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                RAWMOUSE& rm = raw->data.mouse;

                // Update right button state
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                {
                    isRightButtonHeld = true;
                }
                if (rm.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                {
                    isRightButtonHeld = false;
                }

                // Only call UpdateVJoyFromMouse if right button is not held
                if (!isRightButtonHeld)
                {
                    UpdateVJoyFromMouse(rm, raw->header.hDevice);
                    HandleScrollInput(rm);
                }
            }
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                HANDLE device = raw->header.hDevice;
                bool match = false;

                if (g_selectedDevice == NULL)
                {
                    match = true; // All mice: accept every input
                }
                else
                {
                    // Specific device selected
                    for (auto& m : g_mouseDevices)
                    {
                        if (m.hDevice == g_selectedDevice)
                        {
                            // For touchpad, hDevice may be 0
                            if (m.isTouchpad ? device == 0 : device == g_selectedDevice)
                            {
                                match = true;
                                break;
                            }
                        }
                    }
                }

                if (!match)
                {
                    delete[] lpb;
                    return 0; // ignore input from other mice
                }

                RAWMOUSE& rm = raw->data.mouse;

                // <<< ADD HERE >>>
                lastDx += rm.lLastX;
                lastDy += rm.lLastY;

                // delete this lol
            }
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                HANDLE device = raw->header.hDevice;
                bool match = false;

                if (g_selectedDevice == NULL)
                {
                    match = true; // All mice: accept every input
                }
                else
                {
                    // Specific device selected
                    for (auto& m : g_mouseDevices)
                    {
                        if (m.hDevice == g_selectedDevice)
                        {
                            // For touchpad, hDevice may be 0
                            if (m.isTouchpad ? device == 0 : device == g_selectedDevice)
                            {
                                match = true;
                                break;
                            }
                        }
                    }
                }

                if (!match)
                {
                    delete[] lpb;
                    return 0; // ignore input from other mice
                }
                // --- MOUSE BINDING CAPTURE ---
                RAWMOUSE& rm = raw->data.mouse;

                // Only process if user is setting an input from the panel
                if (!inputBeingSet.empty() && !showVJoyPicker)
                {
                    int idx = std::stoi(inputBeingSet); // which inputMap entry we're editing
                    inputMap[idx].type = MOUSE;

                    // Detect which mouse button was pressed
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        inputMap[idx].code = 1;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        inputMap[idx].code = 2;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        inputMap[idx].code = 3;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        inputMap[idx].code = 4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        inputMap[idx].code = 5;
                    else
                        return 0; // unknown button, ignore

                    inputBeingSet.clear(); // finished binding
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                if (toggleInputBeingSet && !togglePedalBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_knobToggleType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    toggleInputBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

                if (reverseUnlockBeingSet)
                {
                    if (rm.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_LEFT;
                    else if (rm.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_RIGHT;
                    else if (rm.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_MIDDLE;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_4_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON4;
                    else if (rm.ulButtons & RI_MOUSE_BUTTON_5_DOWN)
                        g_reverseUnlockType = TOGGLE_MOUSE_BUTTON5;
                    else
                        return 0;

                    reverseUnlockBeingSet = false;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }

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
                    else
                    {
                        // --- Check diamond intersection ---
                        bool insideIntersection = IsInsideIntersection(knobPos.x, knobPos.y);

                        // Lerp factor for smooth movement into rails (only applies when not snapped)
                        const float lerpSpeed = 0.2f;
                        const float lerpSpeedHorizontal = 0.05f;
                        const float lerpSpeedVertical = 0.07f;
                        int railCount = is16GearSet ? 5 : 4;

                        // --- Snap detection ---
                        std::string snappedGear = "";
                        for (auto& kv : activeMap)
                        {
                            POINT g = kv.second;
                            std::string gear = kv.first;

                            double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) + (knobPos.y - g.y) * (knobPos.y - g.y));
                            if (dist < gearSnapInThreshold)
                            {
                                snappedGear = gear;
                                break;
                            }
                        }

                        bool isSnapped = !snappedGear.empty();



                        // --- CLUTCH STATE CALCULATION (Global for this frame) ---
                        // "Engaged" means pedal pressed (disengaged engine) for shifting
                        // We calculate this regardless of clutchLockGear enabled state for robust access
                        bool isClutchEngaged = false;
                        if (useScrollClutch || true) // Always check if we have joyRx data
                        {
                            float cNorm = 0.0f;
                            if (useHalfClutch)
                            {
                                LONG mid = (axisMin + axisMax) / 2;
                                cNorm = (float)(joyRx - mid) / (float)(axisMax - mid);
                            }
                            else
                            {
                                cNorm = (float)(joyRx - axisMin) / (float)(axisMax - axisMin);
                            }
                            cNorm = max(0.0f, min(1.0f, cNorm));
                            
                            isClutchEngaged = (cNorm > 0.2f);
                        }

                        // --- CLUTCH LOCK LOGIC: Check State ---
                        if (clutchLockGear && useScrollClutch)
                        {
                            if (!isClutchEngaged)
                            {
                                if (lockedInGear && !activeGear.empty())
                                {
                                    // Force stay in gear
                                    snappedGear = activeGear;
                                    isSnapped = true;
                                }
                                else
                                {
                                    // Force stay in neutral
                                    snappedGear = "";
                                    isSnapped = false;
                                }
                            }
                        }

                        // --- Handle snapped state ---
                        // --- Handle snapped state ---
                        if (isSnapped)
                        {
                            const int maxHorizontalOffset = 20;            // max pixels allowed from snapped X
                            knobPos.x += int(dx * effectiveSens * 0.2f);

                            // Clamp around snapped gear X
                            int snappedX = activeMap[snappedGear].x;
                            if (knobPos.x < snappedX - maxHorizontalOffset)
                                knobPos.x = snappedX - maxHorizontalOffset;
                            if (knobPos.x > snappedX + maxHorizontalOffset)
                                knobPos.x = snappedX + maxHorizontalOffset;

                            // Allow full vertical movement UNLESS clutch locked
                            if (clutchLockGear && useScrollClutch && !isClutchEngaged && lockedInGear)
                            {
                                // Restrict Y movement to gear radius
                                int targetY = activeMap[snappedGear].y;
                                int newY = knobPos.y + int(dy * effectiveSens);
                                if (abs(newY - targetY) > gearRadius)
                                {
                                    if (newY > targetY) newY = targetY + gearRadius;
                                    else newY = targetY - gearRadius;
                                }
                                knobPos.y = newY;
                            }
                            else
                            {
                                int newY = knobPos.y + int(dy * effectiveSens);
                                ClampMovementForReverseLock(newY, knobPos.x);
                                knobPos.y = newY;
                            }

                            // --- DEBUG: Log snappedGear ---
                            char buf[256];
                            //sprintf_s(buf, "Snapped detected: snappedGear='%s'\n", snappedGear.c_str());
                            //OutputDebugStringA(buf);

                            // Always set gear key when snapped
                            SetGearKey(snappedGear);

                            // Update activeGear and lock state
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
                            // --- Not snapped: regular rail movement with lerp corrections ---

                            if (currentRail == HORIZONTAL)
                            {
                                LONG proposedX = knobPos.x + ApplyKnobMotion(dx, effectiveSens);
                                ClampHorizontalMovementForReverseLock(proposedX, knobPos.y);
                                knobPos.x = proposedX;


                                if (insideIntersection)
                                {
                                    int dY = ApplyKnobMotion(dy, effectiveSens);
                                    
                                    // Safeguard: if in neutral, trying to move vertically, and clutch locked -> block it early
                                    if (clutchLockGear && useScrollClutch && !isClutchEngaged && abs(dY) > 0)
                                    {
                                         dY = 0;
                                    }
                                    int newY = knobPos.y + dY;
                                    ClampMovementForReverseLock(newY, knobPos.x);
                                    knobPos.y = newY;


                                    for (int i = 0; i < railCount; ++i)
                                    {
                                        int railXPos = railX[i].x;
                                        if (abs(knobPos.x - railXPos) < enterVerticalThreshold && abs(dy) > 0)
                                        {
                                            bool movingDown = dy > 0;
                                            bool movingUp = dy < 0;
                                            bool canMoveVertically = false;

                                            for (auto& kv : activeMap)
                                            {
                                                if (kv.second.x == railXPos)
                                                {
                                                    bool isReverse = (kv.first == "R");
                                                    bool reverseLocked = reverseLockEnabled && !IsReverseUnlockActive_Fix();

                                                    if (movingDown && kv.second.y == bottomY) {
                                                        if (!isReverse || !reverseLocked)
                                                            canMoveVertically = true;
                                                    }
                                                    if (movingUp && kv.second.y == topY) {
                                                        if (!isReverse || !reverseLocked)
                                                            canMoveVertically = true;
                                                    }
                                                }
                                            }

                                            if (canMoveVertically)
                                            {
                                                // Prevent vertical entry if clutch is locked and not engaged
                                                if (clutchLockGear && useScrollClutch && !isClutchEngaged)
                                                    canMoveVertically = false;
                                            }

                                            if (canMoveVertically)
                                            {
                                                currentRail = VERTICAL;
                                                currentVerticalIndex = i;
                                            }
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    // Soft horizontal lock: blend mouse input with center pull
                                    int newY = knobPos.y + int(dy * effectiveSens * 0.2f);
                                    newY += int((centerY - newY) * 0.05f);

                                    // FIX: Also clamp vertical movement when outside intersection ("Wide" or "Diagonal" state)
                                    // This prevents bypassing the lock by strictly moving diagonally from the side
                                    ClampMovementForReverseLock_Fix(newY, knobPos.x);

                                    knobPos.y = newY;
                                }
                            }
                            else if (currentRail == VERTICAL && currentVerticalIndex != -1)
                            {
                                int newY = knobPos.y + int(dy * effectiveSens);

                                // --- REVERSE LOCK CLAMPING (Unified) ---
                                ClampMovementForReverseLock_Fix(newY, knobPos.x);

                                knobPos.y = newY;

                                // Define railCenter for current vertical rail
                                int railCenter = railX[currentVerticalIndex].x;

                                // Calculate proposed X with motion and centering
                                LONG proposedX = knobPos.x + int(ApplyKnobMotion(dx, effectiveSens) * 0.2f);
                                proposedX += int((railCenter - proposedX) * 0.07f);

                                // Clamp lateral movement to prevent bypassing Reverse Lock from the adjacent rail
                                ClampHorizontalMovementForReverseLock_Fix(proposedX, knobPos.x, knobPos.y);
                                
                                knobPos.x = proposedX;

                                if (IsInsideIntersection(knobPos.x, knobPos.y) && abs(dx) > 0)
                                {
                                    currentRail = HORIZONTAL;
                                    currentVerticalIndex = -1;
                                }
                            }

                            // --- Sticky Authorization Reset ---
                            if (g_reverseAuthorizedOnRail)
                            {
                                // Reset if any gear OTHER than Reverse is active
                                if (!activeGear.empty() && activeGear != "R")
                                {
                                    g_reverseAuthorizedOnRail = false;
                                }
                                else
                                {
                                    for (auto& kv : activeMap)
                                    {
                                        if (kv.first == "R")
                                        {
                                            int reverseX = kv.second.x;
                                            int reverseY = kv.second.y;
                                            
                                            // Horizontal Reset: Knob moved too far from R column
                                            if (abs(knobPos.x - reverseX) > 45)
                                                g_reverseAuthorizedOnRail = false;
                                                
                                            // Vertical Reset (ONLY for Shared Rails)
                                            bool isSharedRail = false;
                                            for (auto& other : activeMap)
                                            {
                                                if (other.first != "R" && other.second.x == reverseX)
                                                {
                                                    isSharedRail = true;
                                                    break;
                                                }
                                            }

                                            if (isSharedRail)
                                            {
                                                // If R is at top, and we move past center towards the bottom gear
                                                if (reverseY == topY && knobPos.y > centerY + 10)
                                                    g_reverseAuthorizedOnRail = false;
                                                // If R is at bottom, and we move past center towards the top gear
                                                if (reverseY == bottomY && knobPos.y < centerY - 10)
                                                    g_reverseAuthorizedOnRail = false;
                                            }
                                                
                                            break;
                                        }
                                    }
                                }
                            }

                            // --- Rigid Boundary Check ---
                            // This ensures that stateless "jumps" (e.g. fast diagonal moves) 
                            // cannot bypass the component-based clamping.
                            EnforceReverseLockBoundary(knobPos);

                            // --- Snap to gear if close ---
                            for (auto& kv : activeMap)
                            {
                                POINT g = kv.second;
                                std::string gear = kv.first;

                                double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) + (knobPos.y - g.y) * (knobPos.y - g.y));
                                bool inside = dist < gearSnapInThreshold;

                                if (gearInsideRadius.find(gear) == gearInsideRadius.end())
                                    gearInsideRadius[gear] = false;

                                if (inside)
                                {
                                    if (!gearInsideRadius[gear])
                                    {
                                        activeGear = gear;
                                        lockedInGear = true;
                                        SetGearKey(gear); // ADD THIS LINE

                                    }

                                    // Smoothly move knob toward gear
                                    if (lockedInGear)
                                    {
                                        knobPos.x = int(knobPos.x + (g.x - knobPos.x) * snapSpeed);
                                        knobPos.y = int(knobPos.y + (g.y - knobPos.y) * snapSpeed);
                                    }

                                    gearInsideRadius[gear] = true;
                                }
                                else
                                {
                                    gearInsideRadius[gear] = false;
                                    if (activeGear == gear)
                                    {
                                        activeGear = "";
                                        lockedInGear = false;
                                        ReleaseGearKey();
                                    }
                                }
                            }
                        }
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
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        // Handle gear keybind
        if (!keybindBeingSet.empty() && !showVJoyPicker)
        {
            // Set new binding
            gearInputMap[keybindBeingSet] = { KEYBOARD, (WORD)wParam };
            keybindBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // Handle input panel keybind
        if (!inputBeingSet.empty() && !showVJoyPicker)
        {
            int idx = std::stoi(inputBeingSet);
            inputMap[idx].type = KEYBOARD;
            inputMap[idx].code = (WORD)wParam;
            inputBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (toggleInputBeingSet)
        {
            g_knobToggleKey = (int)wParam;
            g_knobToggleType = TOGGLE_KEYBOARD;
            toggleInputBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        if (reverseUnlockBeingSet)
        {
            g_reverseUnlockKey = (int)wParam;
            g_reverseUnlockType = TOGGLE_KEYBOARD;
            reverseUnlockBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }


        else if (wParam == VK_TAB)
        {
            ToggleBorderless(hwnd);
        }
        if (creatingNewProfile) {
            switch (wParam) {
            case VK_LEFT:
                if (profileTextSelectionStart > 0) {
                    profileTextSelectionStart--;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_RIGHT:
                if (profileTextSelectionStart < newProfileName.length()) {
                    profileTextSelectionStart++;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_HOME:
                profileTextSelectionStart = 0;
                profileTextSelectionEnd = 0;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_END:
                profileTextSelectionStart = newProfileName.length();
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_ESCAPE:
                creatingNewProfile = false;
                profileTextSelected = false;
                newProfileName = "New Profile";
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }
        break;
    case WM_CHAR:
    {
        if (creatingNewProfile) {
            if (wParam == VK_RETURN) {
                // Enter pressed - create the profile
                if (!newProfileName.empty()) { // Removed the "New Profile" check
                    CreateNewProfile(hwnd);
                    creatingNewProfile = false;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam == VK_BACK) {
                // Backspace - delete selected text or character before cursor
                if (!newProfileName.empty()) {
                    if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                        // Delete selected text
                        newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    else if (profileTextSelectionStart > 0) {
                        // Delete character before cursor
                        newProfileName.erase(profileTextSelectionStart - 1, 1);
                        profileTextSelectionStart--;
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam >= 32 && wParam <= 126) {
                // Printable characters - replace selection or insert
                char newChar = (char)wParam;

                if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                    // Replace selected text
                    newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                    profileTextSelectionEnd = profileTextSelectionStart;
                }

                // Insert new character
                newProfileName.insert(profileTextSelectionStart, 1, newChar);
                profileTextSelectionStart++;
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            return 0;
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };

        // Handle ALL dropdown clicks first and block any other processing when dropdowns are open
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            bool handledInDropdown = false;

            // Handle profile dropdown clicks
            if (profileDropdownOpen) {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = profileButtonRect.top + 25; // Base Y position without scroll offset

                RECT dropdownRect = {
                    profileButtonRect.left,
                    listY + settingsScrollOffset,
                    profileButtonRect.right,
                    listY + settingsScrollOffset + (int)((listItemHeight + itemGap) * profileNames.size())
                };

                if (PtInRect(&dropdownRect, pt)) {
                    // Check if clicking on existing profiles
                    for (size_t i = 0; i < profileNames.size(); ++i) {
                        RECT itemRect = {
                            dropdownRect.left,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            dropdownRect.top + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };

                        if (PtInRect(&itemRect, pt)) {
                            SwitchProfile((int)i, hwnd);
                            profileDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                // If we clicked anywhere (inside or outside dropdown), close it and block further processing
                if (!handledInDropdown) {
                    profileDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when profile dropdown was open
            }

            // Handle gear layout dropdown clicks
            if (gearLayoutDropdownOpen)
            {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
                int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();

                RECT dropdownRect = {
                    gearLayoutButtonRect.left,
                    listY,
                    gearLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < gearLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left,
                            listY + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentGearLayout = (int)i;
                            gearLabelOverride = gearLayouts[i];
                            gearLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    gearLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when gear layout dropdown was open
            }

            // Handle h-shifter layout dropdown clicks
            if (hShifterLayoutDropdownOpen)
            {
                int listItemHeight = 25;
                int itemGap = 5;
                int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
                int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();

                RECT dropdownRect = {
                    hShifterLayoutButtonRect.left,
                    listY,
                    hShifterLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left,
                            listY + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right,
                            listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentHShifterLayout = hShifterLayouts[i].id;
                            layoutType = currentHShifterLayout;
                            hShifterLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    hShifterLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when h-shifter dropdown was open
            }

            return 0; // Safety return
        }
        if (creatingNewProfile) {
            RECT adjustedProfileButtonRect = {
                profileButtonRect.left,
                profileButtonRect.top + settingsScrollOffset,
                profileButtonRect.right,
                profileButtonRect.bottom + settingsScrollOffset
            };
            RECT adjustedCreateProfileButtonRect = {
                createProfileButtonRect.left,
                createProfileButtonRect.top + settingsScrollOffset,
                createProfileButtonRect.right,
                createProfileButtonRect.bottom + settingsScrollOffset
            };

            // Check if click is NOT in the profile creation area
            if (!PtInRect(&adjustedProfileButtonRect, pt) && !PtInRect(&adjustedCreateProfileButtonRect, pt)) {
                // Clicked outside profile creation area - create the profile
                if (!newProfileName.empty()) {
                    CreateNewProfile(hwnd);
                }
                creatingNewProfile = false;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0; // Block further processing
            }
            // If click is inside profile creation area, continue with normal processing
        }

        // Only process these if NO dropdowns are open
        RECT adjustedProfileButtonRect = {
            profileButtonRect.left,
            profileButtonRect.top + settingsScrollOffset,
            profileButtonRect.right,
            profileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedProfileButtonRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }

        RECT adjustedCreateProfileButtonRect = {
            createProfileButtonRect.left,
            createProfileButtonRect.top + settingsScrollOffset,
            createProfileButtonRect.right,
            createProfileButtonRect.bottom + settingsScrollOffset
        };
        if (PtInRect(&adjustedCreateProfileButtonRect, pt)) {
            creatingNewProfile = true;
            newProfileName = "New Profile";
            profileTextSelected = true;
            profileTextSelectionStart = 0;
            profileTextSelectionEnd = newProfileName.length();
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }


        // --- Block clicks through dropdown ---
        RECT togglePanelRectHit = {
            (LONG)togglePanelRectUnified.X,
            (LONG)togglePanelRectUnified.Y,
            (LONG)(togglePanelRectUnified.X + togglePanelRectUnified.Width),
            (LONG)(togglePanelRectUnified.Y + togglePanelRectUnified.Height) };
        RECT inputPanelRectHit = {
            (LONG)inputPanelRectUnified.X,
            (LONG)inputPanelRectUnified.Y,
            (LONG)(inputPanelRectUnified.X + inputPanelRectUnified.Width),
            (LONG)(inputPanelRectUnified.Y + inputPanelRectUnified.Height) };

        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();

            RECT dropdownRect = {
                gearLayoutButtonRect.left,
                listY,
                gearLayoutButtonRect.right,   // <-- use button right, not +150
                listY + totalHeight
            };


            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < gearLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        currentGearLayout = (int)i;
                        gearLabelOverride = gearLayouts[i];
                        gearLayoutDropdownOpen = false;
                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        ComputeLayout(hwnd);
                        ComputeIntersections();
                        break;
                    }
                }
                return 0;
            }
            else
            {
                gearLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();

            RECT dropdownRect = {
                hShifterLayoutButtonRect.left,
                listY,
                hShifterLayoutButtonRect.right, // <-- match button width
                listY + totalHeight
            };


            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        currentHShifterLayout = hShifterLayouts[i].id;
                        layoutType = currentHShifterLayout;
                        hShifterLayoutDropdownOpen = false;
                        InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                        ComputeLayout(hwnd);
                        ComputeIntersections();
                        break;
                    }
                }
                return 0;
            }
            else
            {
                hShifterLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }

        if (showTogglePanel && PtInRect(&g_toggleKeyRect, pt))
        {
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            {
                // Shift+Click = enter pedal binding mode
                togglePedalBeingSet = true;
                toggleInputBeingSet = false;
                OutputDebugString(_T("Shift+Click -> Listening for pedal...\n"));
            }
            else
            {
                // Normal click = keyboard binding
                toggleInputBeingSet = true;
                togglePedalBeingSet = false;
                OutputDebugString(_T("Click -> Listening for keyboard key...\n"));
            }

            InvalidateRect(hwnd, nullptr, TRUE);
        }
        if (showTogglePanel && PtInRect(&g_assistButtonRect, pt))
        {
            assistButtonBeingSet = true;
            OutputDebugString(_T("Assist Button Clicked -> Listening for controller input...\n"));
            InvalidateRect(hwnd, nullptr, TRUE);
        }

        if (showTogglePanel && PtInRect(&reverseUnlockKeyRect, pt))
        {
            reverseUnlockBeingSet = true;
            OutputDebugString(_T("Reverse Unlock Clicked -> Listening for input...\n"));
            InvalidateRect(hwnd, nullptr, TRUE);
        }



        if (!isBorderless)
        {
            // Close button
            if (PtInRect(&g_CloseButtonRect, pt))
            {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            }

            // Minimize button functionality (for Maximize/Restore button)
            if (PtInRect(&g_MaxButtonRect, pt))
            {
                ShowWindow(hwnd, SW_MINIMIZE);
                break;
            }




            // Drag window by title bar (excluding buttons)
            if (pt.y >= 0 && pt.y < 30 && pt.x < g_MaxButtonRect.left)
            {
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                break;
            }
        }
        // Update button click (add debug output)
        if (updateAvailable && !isBorderless && PtInRect(&updateButtonRect, pt))
        {
            char debugBuf[256];
            sprintf_s(debugBuf, "[UPDATE CLICK] Button clicked! Mouse: (%d,%d), Button: (%d,%d)-(%d,%d)\n",
                pt.x, pt.y,
                updateButtonRect.left, updateButtonRect.top,
                updateButtonRect.right, updateButtonRect.bottom);
            OutputDebugStringA(debugBuf);

            std::wstring currentVersionW(currentVersion.begin(), currentVersion.end());
            std::wstring latestVersionW(latestVersion.begin(), latestVersion.end());

            std::wstring message =
                L"A new version is available! Would you like to update now?\n\n"
                L"Current version: " + currentVersionW + L"\n" +
                L"Latest version: " + latestVersionW + L"\n\n" +
                L"Your settings will be preserved.";

            int result = MessageBoxW(hwnd, message.c_str(), L"Update Available", MB_YESNO | MB_ICONINFORMATION);
            if (result == IDYES)
            {
                OutputDebugStringA("[UPDATE] User chose to update - starting update process...\n");
                PerformUpdate();
            }
            else
            {
                OutputDebugStringA("[UPDATE] User chose to skip update\n");
            }
            return 0; // Important: return here to prevent click from passing through
        }
        if (showInputPanel && PtInRect(&inputPanelRectHit, pt))
        {
            int rowHeight = 36; // same as drawing
            int rowSpacing = 6;
            int yStart = (int)inputPanelRectUnified.Y + 10 + 28; // top padding + subtitle

            int index = (pt.y - yStart) / (rowHeight + rowSpacing);
            if (index >= 0 && index < (int)inputMap.size())
            {
                inputBeingSet = std::to_string(index);

                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    showVJoyPicker = true;
                    vJoyPickerInput = inputBeingSet;
                    InvalidateRect(hwnd, nullptr, TRUE);
                    UpdateWindow(hwnd);
                }
            }
        }

        // --- Keybind panel click ---
        if (showKeybindPanel &&
            mouseX >= panelRect.X && mouseX <= panelRect.X + panelRect.Width &&
            mouseY >= panelRect.Y && mouseY <= panelRect.Y + panelRect.Height)
        {
            // Match drawing offsets
            int rowHeight = 36;            // same as in draw
            int rowSpacing = 6;            // same as in draw
            int yStart = panelRect.Y + 70 + rightPanelScrollOffset; // matches drawing start

            // Prepare sorted key list same as in draw
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

            // Compute clicked row dynamically including spacing
            int index = (mouseY - yStart) / (rowHeight + rowSpacing);

            if (index >= 0 && index < (int)sortedKeys.size())
            {
                keybindBeingSet = sortedKeys[index];

                // Shift+Click → show vJoy picker
                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    showVJoyPicker = true;
                    vJoyPickerGear = keybindBeingSet;
                    InvalidateRect(hwnd, nullptr, TRUE);
                    UpdateWindow(hwnd);
                }
            }
        }

        // --- vJoy Picker click ---
        // --- vJoy Picker click ---
        if (showVJoyPicker && PtInRect(&g_vJoyPickerRect, pt))
        {
            for (int i = 0; i < g_vJoyButtonRects.size(); ++i)
            {
                if (PtInRect(&g_vJoyButtonRects[i], pt))
                {
                    char buf[256];

                    // --- Only update gearInputMap if vJoyPickerGear is set ---
                    if (!vJoyPickerGear.empty())
                    {
                        gearInputMap[vJoyPickerGear] = { VJOY_BUTTON, (WORD)(i + 1) };
                        sprintf_s(buf, "gearInputMap: Bound vJoy button %d to gear '%s'\n", i + 1, vJoyPickerGear.c_str());
                        OutputDebugStringA(buf);
                        vJoyPickerGear.clear(); // reset after use
                    }

                    // --- Only update inputMap secondary if vJoyPickerInput is set ---
                    if (!vJoyPickerInput.empty())
                    {
                        int idx = std::stoi(vJoyPickerInput);

                        // Update only secondary vJoy button, keep primary as-is
                        inputMap[idx].vjoyButton = i + 1;

                        sprintf_s(buf, "inputMap (secondary): Bound vJoy button %d to inputMap index %d\n", i + 1, idx);
                        OutputDebugStringA(buf);

                        vJoyPickerInput.clear(); // reset after use
                        inputBeingSet.clear();   // also reset inputBeingSet
                    }

                    showVJoyPicker = false;  // hide picker
                    keybindBeingSet.clear(); // reset old gear keybind
                    InvalidateRect(hwnd, nullptr, FALSE);
                    break;
                }
            }
        }

        else
        {
            // showVJoyPicker = false; // click outside picker closes it
        }

        if (showSettingsPanel)
        {
            RECT r; // temporary rect for PtInRect

            r = GetScrolledRect(knobSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingKnobSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(gearSliderRect);
            if (PtInRect(&r, pt))
            {
                isDraggingGearRadius = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(sensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSensSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(diagSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingDiagSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapInSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapInSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(noReverseToggleRect);
            if (PtInRect(&r, pt))
            {
                noReverseLayout = !noReverseLayout;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                ComputeLayout(hwnd);
                ComputeIntersections();
            }
            r = GetScrolledRect(hideHighGearsToggleRect);
            if (PtInRect(&r, pt))
            {
                hideHighGears = !hideHighGears;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw

                ComputeLayout(hwnd); // if needed
            }
            r = GetScrolledRect(useScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useScrollClutch = !useScrollClutch;           // toggle the flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            r = GetScrolledRect(reverseLockToggleRect);
            if (PtInRect(&r, pt))
            {
                reverseLockEnabled = !reverseLockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // DLL INJECTION CLICK 

            // --- DLL Injection Controls ---
            r = GetScrolledRect(processComboRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                if (!g_processList.empty()) {
                    static size_t currentIndex = 0;
                    currentIndex = (currentIndex + 1) % g_processList.size();
                    g_selectedProcessId = g_processList[currentIndex];
                }
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(refreshButtonRect);
            if (PtInRect(&r, pt))
            {
                RefreshProcessList();
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(autoInjectButtonRect);
            if (PtInRect(&r, pt))
            {
                g_autoInjectEnabled = !g_autoInjectEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Toggle Mouse Block checkbox
            r = GetScrolledRect(mouseBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_mouseBlockEnabled = !g_mouseBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Toggle XInput Block checkbox
            r = GetScrolledRect(xinputBlockCheckboxRect);
            if (PtInRect(&r, pt))
            {
                g_xinputBlockEnabled = !g_xinputBlockEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(smoothScrollSlider);
            if (PtInRect(&r, pt))
            {
                draggingSmoothScrollSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(brakeresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingBrakeResistanceSlider = true;
                SetCapture(hwnd);
            }

            r = GetScrolledRect(accelerationresistanceSlider);
            if (PtInRect(&r, pt))
            {
                draggingAccelerationResistanceSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(snapSpeedSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSnapSpeedSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(layoutScaleSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingLayoutSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(useAxisSmoothingToggle);
            if (PtInRect(&r, pt))
            {
                useAxisSmoothing = !useAxisSmoothing;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Add this with your other slider dragging handlers:
            r = GetScrolledRect(axisSmoothingFactorSlider);
            if (PtInRect(&r, pt))
            {
                draggingAxisSmoothingFactorSlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(neutralToggleRect);
            if (PtInRect(&r, pt))
            {
                bool wasEnabled = isNeutralEnabled;
                isNeutralEnabled = !isNeutralEnabled;

                // If Neutral was just disabled, release the Neutral key
                if (wasEnabled && !isNeutralEnabled && neutralHeld)
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
                        SetBtn(false, vjoyDeviceId, gi.code);
                    }

                    // Stop Neutral glow animation
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();
                    }

                    neutralHeld = false;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(showYBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showYBar = !showYBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(invertScrollToggleRect);
            if (PtInRect(&r, pt))
            {
                invertScrollClutchAxis = !invertScrollClutchAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Binding Mode For R-Axis toggle
            r = GetScrolledRect(bindingModeForAxisToggle);
            if (PtInRect(&r, pt))
            {
                bindingModeForRAxis = !bindingModeForRAxis;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(realisticKnobToggleRect);
            if (PtInRect(&r, pt))
            {
                realisticKnob = !realisticKnob;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
                InvalidateRect(hwnd, NULL, TRUE); // redraw main window to update knob appearance
            }
            // Clutch Lock Gear Toggle
            r = GetScrolledRect(clutchLockGearToggleRect);
            if (PtInRect(&r, pt))
            {
                clutchLockGear = !clutchLockGear;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            // Invert Assist Axes toggle
            r = GetScrolledRect(invertAssistToggleRect);
            if (PtInRect(&r, pt))
            {
                invertAssistAxes = !invertAssistAxes;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(yBarAlphaSlider);
            if (PtInRect(&r, pt))
            {
                draggingYBarAlphaSlider = true;
                SetCapture(hwnd);
            }

            // Add this after the showYBar toggle click handling:
            r = GetScrolledRect(yBarFixedTransToggle);
            if (PtInRect(&r, pt))
            {
                useYbarFixedTransparency = !useYbarFixedTransparency;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Use LT as Clutch toggle
            r = GetScrolledRect(useLTAsClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useLTAsClutch = !useLTAsClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showXBarToggleRect);
            if (PtInRect(&r, pt))
            {
                showXBar = !showXBar;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(knobAccelToggleRect);
            if (PtInRect(&r, pt))
            {
                knobAccelerationEnabled = !knobAccelerationEnabled; // toggle boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);    // redraw
            }
            r = GetScrolledRect(useXInputToggleRect);
            if (PtInRect(&r, pt))
            {
                useXInput = !useXInput;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(halfScrollClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                useHalfClutch = !useHalfClutch;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(optimizationToggleRect);
            if (PtInRect(&r, pt))
            {
                disableSmartRedraws = !disableSmartRedraws; // Toggle the actual flag
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            r = GetScrolledRect(controllerSensSliderRect);
            if (PtInRect(&r, pt))
            {
                controllerDraggingSensSlider = true;
                SetCapture(hwnd);
            }
            // --- Use Right Stick for Knob Toggle ---
            r = GetScrolledRect(useRightStickToggleRect);
            if (PtInRect(&r, pt))
            {
                useRightStick = !useRightStick; // toggle the boolean
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(disableRealKnobMovementToggleRect);
            if (PtInRect(&r, pt))
            {
                disableRealKnobMovement = !disableRealKnobMovement;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            // Add this with your other toggle click handlers:
            r = GetScrolledRect(useThrottleBrakeToggleRect);
            if (PtInRect(&r, pt))
            {
                useThrottleBrakeAxes = !useThrottleBrakeAxes;

                // If disabling, reset throttle/brake to prevent stuck inputs
                if (!useThrottleBrakeAxes)
                {
                    ResetThrottleBrake();
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }
            r = GetScrolledRect(dynamicTransparencyToggleRect);
            if (PtInRect(&r, pt))
            {
                dynamicTransparencyEnabled = !dynamicTransparencyEnabled;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(minTransparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingMinTransparencySlider = true;
                SetCapture(hwnd);
            }
            r = GetScrolledRect(transparencyFadeDelaySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencyFadeDelaySlider = true;
                SetCapture(hwnd);

            }

            r = GetScrolledRect(transparencySliderRect);
            if (PtInRect(&r, pt))
            {
                draggingTransparencySlider = true;
                SetCapture(hwnd);
            }
            // Steering Sensitivity
            r = GetScrolledRect(steeringSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(steeringDegreesSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingSteeringDegreesSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Acc/Brake Sensitivity
            r = GetScrolledRect(accBrakeSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingAccBrakeSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(scrollSensSliderRect);
            if (PtInRect(&r, pt))
            {
                draggingScrollSensSlider = true;
                SetCapture(hwnd);

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(showClutchToggleRect);
            if (PtInRect(&r, pt))
            {
                showClutchIndicator = !showClutchIndicator;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Steering toggle ---
            r = GetScrolledRect(mouseSteeringToggleRect);
            if (PtInRect(&r, pt))
            {
                mouseSteeringEnabled = !mouseSteeringEnabled;

                if (IsKnobToggleActive())
                {
                    mouseTrackingEnabled = false;
                    knobMovementEnabled = true;
                }
                else
                {
                    mouseTrackingEnabled = mouseSteeringEnabled;
                    knobMovementEnabled = !mouseSteeringEnabled;
                }

                InvalidateRect(hwnd, &settingsPanelRect, FALSE); // redraw
            }

            // --- Mouse Device Selector ---
            r = GetScrolledRect(deviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t idx = 0;
                if (g_selectedDevice)
                {
                    for (size_t i = 0; i < g_mouseDevices.size(); ++i)
                    {
                        if (g_mouseDevices[i].hDevice == g_selectedDevice)
                        {
                            idx = i + 1;
                            break;
                        }
                    }
                }
                if (idx >= g_mouseDevices.size())
                {
                    g_selectedDevice = NULL; // All Mice
                }
                else
                {
                    g_selectedDevice = g_mouseDevices[idx].hDevice;
                }

                InvalidateRect(hwnd, &settingsPanelRect, TRUE); // redraw
            }
            r = GetScrolledRect(gamepadComboRect);
            if (PtInRect(&r, pt)) // `pt` is mouse click position
            {
                if (!g_gamepads.empty()) {
                    // cycle through only real gamepads
                    g_selectedGamepadIndex = (g_selectedGamepadIndex + 1) % g_gamepads.size();
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }


            // --- Steering Device Selector ---
            r = GetScrolledRect(steeringDeviceComboRect);
            if (PtInRect(&r, pt))
            {
                size_t index = 0;
                if (g_selectedSteeringDevice)
                {
                    for (index = 0; index < g_mouseDevices.size(); index++)
                    {
                        if (g_mouseDevices[index].hDevice == g_selectedSteeringDevice)
                        {
                            break;
                        }
                    }
                    index = (index + 1) % (g_mouseDevices.size() + 1); // +1 for "All Mice"
                }
                else
                {
                    index = 0; // start at first device if currently on "All Mice"
                }

                if (index < g_mouseDevices.size())
                {
                    g_selectedSteeringDevice = g_mouseDevices[index].hDevice;
                }
                else
                {
                    g_selectedSteeringDevice = NULL; // "All Mice"
                }
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            r = GetScrolledRect(gearLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                gearLayoutDropdownOpen = !gearLayoutDropdownOpen;
                hShifterLayoutDropdownOpen = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            r = GetScrolledRect(hShifterLayoutButtonRect);
            if (PtInRect(&r, pt))
            {
                hShifterLayoutDropdownOpen = !hShifterLayoutDropdownOpen;
                gearLayoutDropdownOpen = false; // Close other dropdown
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }

        break;
    }

    case WM_MOUSEMOVE:
    {
        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        POINT pt = { mouseX, mouseY };
        POINT adjustedPt = { mouseX, mouseY - settingsScrollOffset };

        // --- DISABLE TOOLTIP WHEN DROPDOWNS ARE OPEN ---
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            // Cancel any active tooltip timers and hide current tooltip
            if (currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            g_showTooltip = false;

            // Don't break - continue to process dropdown hovers and sliders
        }
        else
        {
            // Only process tooltips when dropdowns are closed
            // Check hover for all tooltips
            TooltipInfo* hoveredTooltip = nullptr;

            for (auto& tooltip : tooltips) {
                if (PtInRect(&tooltip.bounds, adjustedPt)) {  // ← Use adjustedPt here
                    hoveredTooltip = &tooltip;
                    break;
                }
            }

            // Start timer for new hover, cancel for others
            if (hoveredTooltip && hoveredTooltip != currentTooltip) {
                // Cancel previous timer
                if (currentTooltip) {
                    KillTimer(hwnd, currentTooltip->timerId);
                    currentTooltip->show = false;
                }

                // Start new timer
                currentTooltip = hoveredTooltip;
                currentTooltip->timerId = SetTimer(hwnd, 1001, 500, nullptr); // 500ms delay
            }
            else if (!hoveredTooltip && currentTooltip) {
                // No hover, hide current tooltip
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }

            // Check if mouse is over the special tooltip (g_tooltipBounds)
            g_showTooltip = (mouseX >= g_tooltipBounds.left && mouseX <= g_tooltipBounds.right &&
                mouseY >= g_tooltipBounds.top && mouseY <= g_tooltipBounds.bottom);

            // Only redraw if tooltip state changed
            static bool lastTooltipState = false;
            if (g_showTooltip != lastTooltipState) {
                lastTooltipState = g_showTooltip;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }

        // Reset hover states (this should happen regardless of dropdown state)
        int prevGearHover = hoveredGearLayoutIndex;
        int prevHShifterHover = hoveredHShifterLayoutIndex;
        hoveredGearLayoutIndex = -1;
        hoveredHShifterLayoutIndex = -1;
        int prevProfileHover = hoveredProfileIndex; // Track previous hover state
        hoveredProfileIndex = -1; // Reset hover state
        // Check for gear layout dropdown hover
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();

            RECT dropdownRect = {
                gearLayoutButtonRect.left,
                listY,
                gearLayoutButtonRect.right,
                listY + totalHeight
            };

            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < gearLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredGearLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }

        // Check for h-shifter layout dropdown hover
        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();

            RECT dropdownRect = {
                hShifterLayoutButtonRect.left,
                listY,
                hShifterLayoutButtonRect.right,
                listY + totalHeight
            };

            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredHShifterLayoutIndex = (int)i;
                        break;
                    }
                }
            }
        }
        if (profileDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = profileButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size();

            RECT dropdownRect = {
                profileButtonRect.left,
                listY,
                profileButtonRect.right,
                listY + totalHeight
            };

            if (PtInRect(&dropdownRect, pt))
            {
                for (size_t i = 0; i < profileNames.size(); ++i)
                {
                    RECT itemRect = {
                        dropdownRect.left,
                        listY + (LONG)(i * (listItemHeight + itemGap)),
                        dropdownRect.right,
                        listY + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                    };
                    if (PtInRect(&itemRect, pt))
                    {
                        hoveredProfileIndex = (int)i;
                        break;
                    }
                }
            }
        }
        // Redraw if hover state changed
        bool needRedraw = (prevGearHover != hoveredGearLayoutIndex) ||
            (prevHShifterHover != hoveredHShifterLayoutIndex) ||
            (prevProfileHover != hoveredProfileIndex); // Add this comparison

        if (needRedraw) {
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }

        // ----- Knob Radius Slider -----
        if (draggingKnobSlider)
        {
            int width = knobSliderRect.right - knobSliderRect.left;
            float t = float(mouseX - knobSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            knobRadius = knobRadiusMin + int(t * (knobRadiusMax - knobRadiusMin));
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Gear Radius Slider -----
        if (isDraggingGearRadius)
        {
            int width = gearSliderRect.right - gearSliderRect.left;
            float t = float(mouseX - gearSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            gearRadius = gearRadiusMin + int(t * (gearRadiusMax - gearRadiusMin));

            // ✅ Add this line here:
            gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
            ComputeIntersections();

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // Max slider (0..255 range independent of minAlpha)
        if (draggingTransparencySlider)
        {
            int width = transparencySliderRect.right - transparencySliderRect.left;
            float t = float(mouseX - transparencySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            maxAlpha = BYTE(t * 255.0f);
            maxAlpha = max(maxAlpha, minAlpha + 1); // enforce > min
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (draggingMinTransparencySlider)
        {
            int width = minTransparencySliderRect.right - minTransparencySliderRect.left;
            float t = float(mouseX - minTransparencySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            minAlpha = BYTE(t * 255.0f);
            minAlpha = min(minAlpha, maxAlpha - 1); // enforce < max
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingTransparencyFadeDelaySlider)
        {
            int width = transparencyFadeDelaySliderRect.right - transparencyFadeDelaySliderRect.left;
            float t = float(mouseX - transparencyFadeDelaySliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Round to nearest 10 for +10 increments
            transparencyFadeDelay = DWORD(round(t * 2000.0f / 10.0f) * 10.0f); // range: 0–2000 ms in +10 increments
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // Add this with your other slider update handlers:
        if (draggingAxisSmoothingFactorSlider)
        {
            int width = axisSmoothingFactorSlider.right - axisSmoothingFactorSlider.left;
            float t = float(mouseX - axisSmoothingFactorSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            axisSmoothingFactor = t; // 0.0 to 1.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Sensitivity Slider -----
        if (draggingSensSlider)
        {
            int width = sensSliderRect.right - sensSliderRect.left;
            float t = float(mouseX - sensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            knobSensitivity = knobSensitivityMin + t * (knobSensitivityMax - knobSensitivityMin);
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Diagonal Assist Slider -----
        if (draggingDiagSlider)
        {
            int width = diagSliderRect.right - diagSliderRect.left;
            float t = float(mouseX - diagSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            diagonalAssist = diagMin + t * (diagMax - diagMin);

            // Recompute thresholds
            enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
            for (auto& inter : intersections)
            {
                inter.radius = int(baseIntersectionRadius * diagonalAssist);
            }

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // ----- Snap-In Threshold Slider -----
        if (draggingSnapInSlider)
        {
            int width = snapInSliderRect.right - snapInSliderRect.left;
            float t = float(mouseX - snapInSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            gearSnapInMultiplier = snapInMin + t * (snapInMax - snapInMin);
            gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier); // update actual threshold
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSteeringSlider)
        {
            int width = steeringSensSliderRect.right - steeringSensSliderRect.left;
            float t = float(mouseX - steeringSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            steeringSensitivity = 0.1f + t * 4.9f; // example: 0.1 – 5.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSteeringDegreesSlider)
        {
            int width = steeringDegreesSliderRect.right - steeringDegreesSliderRect.left;
            float t = float(mouseX - steeringDegreesSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Map t (0–1) to 90–900°
            float rawDegrees = 90.0f + t * (900.0f - 90.0f);

            // --- Quantize to 10° steps ---
            maxSteeringDegrees = round(rawDegrees / 10.0f) * 10.0f;

            InvalidateRect(hwnd, nullptr, FALSE);
        }


        // Acc/Brake Sensitivity
        if (draggingAccBrakeSlider)
        {
            int width = accBrakeSensSliderRect.right - accBrakeSensSliderRect.left;
            float t = float(mouseX - accBrakeSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            accBrakeSensitivity = 0.1f + t * 19.9f; // 0.1 – 20.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingScrollSensSlider)
        {
            int width = scrollSensSliderRect.right - scrollSensSliderRect.left;
            float t = float(mouseX - scrollSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            scrollClutchSens = t * 10.0f; // 0–10
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // ----- Snap Speed Slider -----
        if (draggingSnapSpeedSlider)
        {
            int width = snapSpeedSliderRect.right - snapSpeedSliderRect.left;
            float t = float(mouseX - snapSpeedSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            snapSpeed = snapSpeedMin + t * (snapSpeedMax - snapSpeedMin);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingSmoothScrollSlider)
        {
            int width = smoothScrollSlider.right - smoothScrollSlider.left;
            float t = float(mouseX - smoothScrollSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            smoothScrollSpeed = 1.0f + t * 19.0f; // 1.0-20.0 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingBrakeResistanceSlider)
        {
            int width = brakeresistanceSlider.right - brakeresistanceSlider.left;
            float t = float(mouseX - brakeresistanceSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            brakeresistanceFactor = t * 50.0f; // 0-50 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (draggingAccelerationResistanceSlider)
        {
            int width = accelerationresistanceSlider.right - accelerationresistanceSlider.left;
            float t = float(mouseX - accelerationresistanceSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            accelerationResistanceFactor = t * 50.0f; // 0-50 range
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (controllerDraggingSensSlider)
        {
            int width = controllerSensSliderRect.right - controllerSensSliderRect.left;
            float t = float(mouseX - controllerSensSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            controllerSensSliderValue = t;
            controllerSensMultiplier = 0.1f + t * 2.0f; // map 0-1 -> 0.1x to 2x
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        if (draggingYBarAlphaSlider)
        {
            int width = yBarAlphaSlider.right - yBarAlphaSlider.left;
            float t = float(mouseX - yBarAlphaSlider.left) / float(width);
            t = max(0.0f, min(1.0f, t));

            // Convert percentage (0-100) back to 0-255 range
            int percentage = (int)(t * 100.0f);
            yBarAlpha = (int)((percentage / 100.0f) * 255.0f);

            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // ----- Layout Scale Slider -----
        if (draggingLayoutSlider)
        {
            int width = layoutScaleSliderRect.right - layoutScaleSliderRect.left;
            float t = float(mouseX - layoutScaleSliderRect.left) / float(width);
            t = max(0.0f, min(1.0f, t));
            layoutScale = 1.0f + t * (3.0f - 1.0f); // min: 1.0, max: 3.0
            ComputeLayout(hwnd);                    // recompute rails & positions

            ComputeIntersections();

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        break;
    }

    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        draggingKnobSlider = false;
        draggingSensSlider = false;
        draggingDiagSlider = false;
        draggingSnapInSlider = false;
        draggingSnapSpeedSlider = false;
        draggingLayoutSlider = false;
        isDraggingGearRadius = false;
        draggingTransparencySlider = false;
        controllerDraggingSensSlider = false;
        draggingSteeringSlider = false;
        draggingAccBrakeSlider = false;
        draggingMinTransparencySlider = false;
        draggingScrollSensSlider = false;
        draggingSteeringDegreesSlider = false;
        draggingTransparencyFadeDelaySlider = false;
        draggingAxisSmoothingFactorSlider = false;
        draggingYBarAlphaSlider = false;  // Add this
        draggingSmoothScrollSlider = false;
        draggingBrakeResistanceSlider = false;
        draggingAccelerationResistanceSlider = false;

        break;
    }
    case WM_CAPTURECHANGED:
    {
        // If capture was lost for any reason, stop all dragging
        draggingKnobSlider = false;
        draggingSensSlider = false;
        draggingDiagSlider = false;
        draggingSnapInSlider = false;
        draggingSnapSpeedSlider = false;
        draggingLayoutSlider = false;
        isDraggingGearRadius = false;
        draggingTransparencySlider = false;
        controllerDraggingSensSlider = false;
        draggingSteeringSlider = false;
        draggingAccBrakeSlider = false;
        draggingMinTransparencySlider = false;
        draggingScrollSensSlider = false;
        draggingSteeringDegreesSlider = false;
        draggingTransparencyFadeDelaySlider = false;
        draggingAxisSmoothingFactorSlider = false;
        draggingYBarAlphaSlider = false;  // Add this

        break;
    }
    case WM_TIMER:
    {
        // ADD THESE TWO FLAGS AT THE VERY TOP
        if (!isBorderless) {
            if (wParam == 1001 && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = true;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            if ((gearLayoutDropdownOpen || hShifterLayoutDropdownOpen) && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }
        UpdateKnobMovement(hwnd);
        UpdateAutoInjection();
        UpdateSmoothScroll();
        if (knobDisabledByF9)
        {
            knobMovementEnabled = IsKnobToggleActive();
        }

        // Update controller positions
        static POINT lastControllerKnobPos = knobPos;
        static POINT lastControllerGhostKnobPos = ghostKnobPos;

        POINT prevKnobPos = knobPos;
        POINT prevGhostKnobPos = ghostKnobPos;
        UpdateKnobFromXInput(hwnd);
        UpdateVJoyFromXInput();

        bool controllerKnobMoved = (knobPos.x != prevKnobPos.x || knobPos.y != prevKnobPos.y);
        bool controllerGhostMoved = (ghostKnobPos.x != prevGhostKnobPos.x || ghostKnobPos.y != prevGhostKnobPos.y);

        PollDirectInput();
        PollPedalBinding(hwnd);

        // === UPDATE KEYBIND GLOW ANIMATIONS (ALWAYS PROCESS) ===
        bool anyGlowAnimationChanged = false;
        DWORD currentTime = GetTickCount();

        for (auto& kv : keybindAnimations) {
            auto& animation = kv.second;
            float oldAlpha = animation.glowAlpha;

            if (animation.isHeld) {
                // Key is held down - immediately go to full brightness and stay there
                animation.glowAlpha = MAX_GLOW_ALPHA;
                animation.isActive = false;
            }
            else {
                // Key is not held - handle fade in/out
                if (animation.isActive) {
                    // Fade in
                    float elapsed = (float)(currentTime - animation.activationTime);
                    animation.glowAlpha = min(MAX_GLOW_ALPHA, elapsed / GLOW_FADE_IN_TIME);

                    if (animation.glowAlpha >= MAX_GLOW_ALPHA) {
                        animation.glowAlpha = MAX_GLOW_ALPHA;
                        animation.isActive = false;
                        animation.activationTime = currentTime;
                    }
                }
                else {
                    // Fade out
                    float elapsed = (float)(currentTime - animation.activationTime);
                    if (elapsed < GLOW_FADE_OUT_TIME) {
                        animation.glowAlpha = max(0.0f, MAX_GLOW_ALPHA - (elapsed / GLOW_FADE_OUT_TIME));
                    }
                    else {
                        animation.glowAlpha = 0.0f;
                    }
                }
            }

            if (abs(animation.glowAlpha - oldAlpha) > 0.01f) {
                anyGlowAnimationChanged = true;
            }
        }

        // Clean up completed animations
        std::vector<std::string> toRemove;
        for (auto& kv : keybindAnimations) {
            if (!kv.second.isHeld && !kv.second.isActive && kv.second.glowAlpha <= 0.0f) {
                toRemove.push_back(kv.first);
            }
        }
        for (auto& key : toRemove) {
            keybindAnimations.erase(key);
        }

        // --- TRACK ALL VISUAL STATES FOR SMART REDRAW ---
        static float lastKnobFlash = knobFlash;
        static bool lastGreyOutState = !knobMovementEnabled;
        static bool lastKnobMovementEnabled = knobMovementEnabled;
        static float lastClutchNorm = 0.0f;
        static bool lastVJoyMouseEnabled = vJoyMouseEnabled;
        static std::string lastActiveGear = activeGear;

        // Ghost knob tracking
        static POINT lastGhostKnobPos = ghostKnobPos;
        static std::string lastGhostSnappedGear = ghostSnappedGear;
        static bool lastUseAssistPointer = useAssistPointer;

        // Indicator bar tracking
        static bool lastMouseSteeringEnabled = mouseSteeringEnabled;
        static bool lastShowYBar = showYBar;
        static bool lastShowXBar = showXBar;
        static bool lastShowClutchIndicator = showClutchIndicator;
        static int lastJoyY = joyY;
        static int lastJoyX = joyX;
        static int lastJoyRx = joyRx;
        static bool lastAssistButtonState = false;
        static bool assistButtonChanged = false;
        // X-bar specific tracking
        static POINT lastKnobPosForXBar = knobPos;
        static int lastDrawRailCount = 0;

        // Calculate current rail count for X-bar positioning
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

        // Calculate current clutch normalization
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

        // Update flash state
        bool anyPressed = false;
        for (auto& kv : buttonPressedState)
        {
            if (kv.second) {
                anyPressed = true;
                break;
            }
        }

        if (anyPressed)
            knobFlash = 0.5f;
        else
            knobFlash = max(0.0f, knobFlash - knobFlashFade);

        // --- CHECK ALL VISUAL CHANGES THAT REQUIRE REDRAW ---
        bool flashChanged = (abs(knobFlash - lastKnobFlash) > 0.01f);
        bool greyOutChanged = (!knobMovementEnabled != lastGreyOutState);
        bool knobEnabledChanged = (knobMovementEnabled != lastKnobMovementEnabled);
        bool clutchEffectChanged = (abs(currentClutchNorm - lastClutchNorm) > 0.01f);
        bool vJoyStateChanged = (vJoyMouseEnabled != lastVJoyMouseEnabled);
        bool activeGearChanged = (activeGear != lastActiveGear);
        static POINT lastBorderlessKnobPos = knobPos;
        bool borderlessKnobMoved = (knobPos.x != lastBorderlessKnobPos.x || knobPos.y != lastBorderlessKnobPos.y);
        lastBorderlessKnobPos = knobPos;
        // Ghost knob states
        bool ghostMoved = (ghostKnobPos.x != lastGhostKnobPos.x || ghostKnobPos.y != lastGhostKnobPos.y);
        bool ghostGearChanged = (ghostSnappedGear != lastGhostSnappedGear);
        bool ghostVisibilityChanged = (useAssistPointer != lastUseAssistPointer);

        // Indicator bar states
        bool mouseSteeringEnabledChanged = (mouseSteeringEnabled != lastMouseSteeringEnabled);
        bool showYBarChanged = (showYBar != lastShowYBar);
        bool showXBarChanged = (showXBar != lastShowXBar);
        bool showClutchIndicatorChanged = (showClutchIndicator != lastShowClutchIndicator);

        // Input value changes for indicator bars
        bool joyYChanged = (joyY != lastJoyY);
        bool joyXChanged = (joyX != lastJoyX);
        bool joyRxChanged = (joyRx != lastJoyRx);

        // X-bar specific redraw check
        bool xBarNeedsRedraw = joyXChanged || showXBarChanged || mouseSteeringEnabledChanged ||
            (knobPos.x != lastKnobPosForXBar.x || knobPos.y != lastKnobPosForXBar.y) ||
            (currentDrawRailCount != lastDrawRailCount);

        // Check if any visual state changed enough to warrant redraw
        bool needsFullRedraw = flashChanged || greyOutChanged || knobEnabledChanged ||
            clutchEffectChanged || vJoyStateChanged || activeGearChanged ||
            ghostMoved || ghostGearChanged || ghostVisibilityChanged ||
            mouseSteeringEnabledChanged || showYBarChanged || showClutchIndicatorChanged ||
            joyYChanged || joyRxChanged || controllerKnobMoved ||
            controllerGhostMoved || anyGlowAnimationChanged;

        // === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
// === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
        // In your WM_TIMER case, replace the borderless section with this:

        if (isBorderless)
        {
            if (disableSmartRedraws)
            {
                // SIMPLE CONSTANT REDRAW - NO SMART LOGIC
                InvalidateRect(hwnd, nullptr, FALSE);

            }
            else
            {
                // YOUR EXISTING SMART REDRAW LOGIC GOES HERE (everything below)
                if (knobMovementEnabled)
                {
                    // --- KNOB MOVEMENT ENABLED ---
            // ADD THIS FLAG
                    bool disableSmartFPS = true;

                    DWORD currentTime = GetTickCount();
                    float moveDistance = CalculateKnobMovementIntensity();

                    // SYNC ALL SYSTEMS TO SAME FPS TIMING (EXCEPT FLASH)
                    bool shouldRedrawThisFrame = false;
                    int syncFPS = currentRedrawFPS;

                    // 1. FLASH EFFECT - ALWAYS UNLIMITED FPS, OUTSIDE SMART SYSTEM
                    if (flashChanged && knobFlash > 0.0f && !realisticKnob) // ADD: && !realisticKnob
                    {
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }
                    }
                    if (vJoyStateChanged)
                    {
                        // When vJoy state changes, redraw knob immediately for visual feedback
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        // Also redraw any active gear text on the knob
                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }



                        // Update tracking immediately
                        lastVJoyMouseEnabled = vJoyMouseEnabled;
                    }
                    // 2. GHOST KNOB MOVEMENT - SEPARATE SMART FPS SYSTEM
                    // 2. GHOST KNOB MOVEMENT - SEPARATE SMART FPS SYSTEM
                    if ((ghostMoved || ghostGearChanged) && useAssistPointer)
                    {
                        // === UNLIMITED FPS - NO THROTTLING ===
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);

                        // Optional: Still track for debugging if needed
                        ghostKnobMoveDistance += 1.0f;
                        lastGhostKnobMoveTime = GetTickCount();
                    }

                    // 3. REAL KNOB MOVEMENT FPS control (your existing code)
                     // 3. REAL KNOB MOVEMENT FPS control (your existing code)
                    if (moveDistance > 0.1f)
                    {
                        knobMoveDistance += moveDistance;

                        // MODIFY THIS PART
                        if (disableSmartFPS)
                        {
                            shouldRedrawThisFrame = true;
                            syncFPS = 0;
                        }
                        else
                        {
                            if (knobMoveDistance < 1.0f) syncFPS = 5;
                            else if (knobMoveDistance < 3.0f) syncFPS = 10;
                            else syncFPS = 0;

                            if (syncFPS > 0)
                            {
                                DWORD minRedrawInterval = 1000 / syncFPS;
                                shouldRedrawThisFrame = (currentTime - lastKnobMoveTime) >= minRedrawInterval;
                            }
                            else
                            {
                                shouldRedrawThisFrame = true;
                            }
                        }

                        if (shouldRedrawThisFrame)
                        {
                            knobMoveDistance = 0.0f;
                            lastKnobMoveTime = currentTime;

                            RECT knobRedrawRect = CalculateKnobRedrawArea();
                            InvalidateRect(hwnd, &knobRedrawRect, FALSE);
                        }

                        lastKnobPos = knobPos;
                    }
                    // Update assist button tracking
                    assistButtonChanged = (assistButtonHeld != lastAssistButtonState);
                    lastAssistButtonState = assistButtonHeld;
                    if (assistButtonChanged && !assistButtonHeld)
                    {
                        // Assist button was just released - hide ghost knob immediately
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);
                        char debugMsg[256];
                        sprintf_s(debugMsg, "ASSIST RELEASED 1: assistButtonHeld=%d, useAssistPointer=%d",
                            assistButtonHeld, useAssistPointer);
                        OutputDebugStringA(debugMsg);
                    }
                    // 4. Other events use the SAME FPS timing (EXCLUDING FLASH AND GHOST)
                    bool hasCriticalEvent = activeGearChanged || greyOutChanged || knobEnabledChanged || assistButtonChanged;
                    if (hasCriticalEvent)
                    {
                        // Gear changes - keep FPS throttling
                        if (activeGearChanged && shouldRedrawThisFrame)
                        {
                            if (!lastActiveGearState.empty() && lastActiveGearState != "N")
                            {
                                RECT oldGearRect = CalculateSingleGearRedrawArea(lastActiveGearState);
                                InvalidateRect(hwnd, &oldGearRect, FALSE);
                            }

                            if (!activeGear.empty() && activeGear != "N")
                            {
                                RECT newGearRect = CalculateSingleGearRedrawArea(activeGear);
                                InvalidateRect(hwnd, &newGearRect, FALSE);
                            }

                            lastActiveGearState = activeGear;
                        }

                        // Greyout changes - IMMEDIATE REDRAW (no FPS throttling)
                        if (greyOutChanged || knobEnabledChanged)
                        {
                            RECT allGearsRect = CalculateAllGearsRedrawArea();
                            InvalidateRect(hwnd, &allGearsRect, FALSE);
                            lastKnobMovementEnabledState = knobMovementEnabled;
                        }
                    }
                }
                else
                {
                    // --- KNOB MOVEMENT DISABLED ---
                    // But ghost knob might still be active!

                    // GHOST KNOB MOVEMENT WHEN DISABLED - SEPARATE SMART FPS SYSTEM
                    // GHOST KNOB MOVEMENT WHEN DISABLED - SEPARATE SMART FPS SYSTEM
                    assistButtonChanged = (assistButtonHeld != lastAssistButtonState);
                    lastAssistButtonState = assistButtonHeld;
                    if (vJoyStateChanged)
                    {
                        // When vJoy state changes, redraw knob immediately for visual feedback
                        RECT knobRedrawRect = CalculateKnobRedrawArea();
                        InvalidateRect(hwnd, &knobRedrawRect, FALSE);

                        // Also redraw any active gear text on the knob
                        if (!activeGear.empty())
                        {
                            RECT gearRect = CalculateSingleGearRedrawArea(activeGear);
                            InvalidateRect(hwnd, &gearRect, FALSE);
                        }

                        // Update tracking immediately
                        lastVJoyMouseEnabled = vJoyMouseEnabled;
                    }

                    // GHOST KNOB MOVEMENT WHEN DISABLED - UNLIMITED FPS
                    if ((ghostMoved || ghostGearChanged) && useAssistPointer)
                    {
                        // === UNLIMITED FPS - NO THROTTLING ===
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);

                        // Optional: Still track for debugging if needed
                        ghostKnobMoveDistance += 1.0f;
                        lastGhostKnobMoveTime = GetTickCount();
                    }

                    if (assistButtonChanged && !assistButtonHeld)
                    {
                        // Assist button was just released - hide ghost knob immediately
                        RECT ghostRedrawRect = CalculateGhostKnobRedrawArea();
                        InvalidateRect(hwnd, &ghostRedrawRect, FALSE);
                        char debugMsg[256];
                        sprintf_s(debugMsg, "ASSIST RELEASED: assistButtonHeld=%d, useAssistPointer=%d",
                            assistButtonHeld, useAssistPointer);
                        OutputDebugStringA(debugMsg);
                    }

                    // 1. Handle gear greying out when knob gets disabled
                    if (greyOutChanged || knobEnabledChanged)
                    {
                        // Redraw ALL gears when knob gets disabled/enabled
                        RECT allGearsRect = CalculateAllGearsRedrawArea();
                        if (allGearsRect.right > allGearsRect.left) // Valid rect
                            InvalidateRect(hwnd, &allGearsRect, FALSE);

                        lastKnobMovementEnabledState = knobMovementEnabled;
                    }


                }
                // --- CLUTCH KNOB GLOW (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (useScrollClutch && !realisticKnob) // ADD: && !realisticKnob
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastClutchKnobRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Check if knob needs redrawing due to clutch or vJoy changes
                        if (ShouldRedrawClutchKnob())
                        {
                            RECT clutchKnobRedrawArea = CalculateClutchKnobRedrawArea();
                            if (clutchKnobRedrawArea.right > clutchKnobRedrawArea.left)
                                InvalidateRect(hwnd, &clutchKnobRedrawArea, FALSE);

                            lastClutchKnobRedrawTime = currentTime; // Update timer only when actually redrawing
                        }
                    }
                }
                // --- RAIL COLOR CHANGES (CLUTCH DEPENDENT) ---
                if (useScrollClutch)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastRailRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Check if rails need redrawing due to clutch changes
                        if (ShouldRedrawRails())
                        {
                            RECT railRedrawArea = CalculateRailRedrawArea();
                            if (railRedrawArea.right > railRedrawArea.left)
                                InvalidateRect(hwnd, &railRedrawArea, FALSE);

                            lastRailRedrawTime = currentTime; // Update timer only when actually redrawing
                        }
                    }
                }
                // --- X-BAR MOVEMENT (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (showXBar && mouseSteeringEnabled)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastXBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
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

                        int fullWidth = railX[drawRailCount - 1].x - railX[0].x;
                        int xBarWidth = static_cast<int>(fullWidth * 0.7f);
                        int xCenter = railX[0].x + (railX[drawRailCount - 1].x - railX[0].x) / 2;
                        int xBarYOffset = 30;
                        int currentXBarY = max(bottomY + 50, knobPos.y + knobRadius + xBarYOffset);
                        int xBarLeft = xCenter - xBarWidth / 2;

                        float normalizedX = (float)(joyX - axisMinX) / (float)(axisMaxX - axisMinX);
                        normalizedX = max(0.0f, min(1.0f, normalizedX));
                        int currentIndicatorX = xBarLeft + (int)(normalizedX * xBarWidth);

                        bool xBarPositionChanged = (currentXBarY != lastXBarY);
                        bool indicatorMoved = (currentIndicatorX != lastIndicatorX);

                        // If anything changed, redraw the combined area to prevent trails
                        if (xBarPositionChanged || indicatorMoved || xBarNeedsRedraw)
                        {
                            RECT xBarCompleteArea = CalculateXBarCompleteRedrawArea();
                            if (xBarCompleteArea.right > xBarCompleteArea.left)
                                InvalidateRect(hwnd, &xBarCompleteArea, FALSE);

                            lastXBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastXBarY = currentXBarY;
                        lastIndicatorX = currentIndicatorX;
                    }
                }


                // --- Y-BAR MOVEMENT if fixed transparency disabled ---
                if (showYBar && mouseSteeringEnabled && !useYbarFixedTransparency)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastYBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
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

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 20;
                        int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

                        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
                        normalizedY = max(0.0f, min(1.0f, normalizedY));
                        int currentFillHeight = (int)(barHeight * normalizedY);

                        bool yBarPositionChanged = (currentYBarX != lastYBarX);
                        bool fillHeightChanged = (currentFillHeight != lastYBarFillHeight);

                        // If anything changed, redraw the combined area to prevent trails
                        if (yBarPositionChanged || fillHeightChanged)
                        {
                            RECT yBarCompleteArea = CalculateYBarCompleteRedrawArea();
                            if (yBarCompleteArea.right > yBarCompleteArea.left)
                                InvalidateRect(hwnd, &yBarCompleteArea, FALSE);

                            lastYBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastYBarX = currentYBarX;
                        lastYBarFillHeight = currentFillHeight;
                    }
                }
                // if fixed transparency enabled
                if (showYBar && mouseSteeringEnabled && useYbarFixedTransparency)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastYBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
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

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 20;
                        int currentYBarX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

                        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
                        normalizedY = max(0.0f, min(1.0f, normalizedY));
                        int currentFillHeight = (int)(barHeight * normalizedY);

                        bool yBarPositionChanged = (currentYBarX != lastYBarX);
                        bool fillHeightChanged = (currentFillHeight != lastYBarFillHeight);

                        // If anything changed, redraw the Y-bar window
                        if (yBarPositionChanged || fillHeightChanged || joyYChanged || showYBarChanged)
                        {
                            if (isBorderless && g_yBarHwnd)
                            {
                                // REDRAW BOTH CURRENT AND PREVIOUS AREAS TO PREVENT TRAILING
                                RECT currentYBarArea = CalculateYBarRedrawArea();
                                RECT previousYBarArea;

                                // Calculate previous area based on last position
                                previousYBarArea.left = lastYBarX - 5;
                                previousYBarArea.top = centerY - barHeight / 2 - 5;
                                previousYBarArea.right = lastYBarX + barWidth + 5;
                                previousYBarArea.bottom = centerY + barHeight / 2 + 5;

                                // Combine both areas
                                RECT combinedArea;
                                combinedArea.left = min(currentYBarArea.left, previousYBarArea.left);
                                combinedArea.top = min(currentYBarArea.top, previousYBarArea.top);
                                combinedArea.right = max(currentYBarArea.right, previousYBarArea.right);
                                combinedArea.bottom = max(currentYBarArea.bottom, previousYBarArea.bottom);

                                // Redraw the combined area
                                InvalidateRect(g_yBarHwnd, &combinedArea, FALSE);
                            }
                            else
                            {
                                // Fallback: redraw in main window (windowed mode)
                                RECT yBarCompleteArea = CalculateYBarCompleteRedrawArea();
                                if (yBarCompleteArea.right > yBarCompleteArea.left)
                                    InvalidateRect(hwnd, &yBarCompleteArea, FALSE);
                            }

                            lastYBarRedrawTime = currentTime;
                        }

                        // Update tracking variables
                        lastYBarX = currentYBarX;
                        lastYBarFillHeight = currentFillHeight;
                    }
                }
                // --- CLUTCH BAR MOVEMENT (ALWAYS ACTIVE REGARDLESS OF KNOB STATE) ---
                if (showClutchIndicator && useScrollClutch)
                {
                    DWORD currentTime = GetTickCount();

                    // Throttle to 30FPS
                    if (currentTime - lastClutchBarRedrawTime >= BAR_REDRAW_INTERVAL)
                    {
                        // Calculate current positions
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

                        int barWidth = 8;
                        int barHeight = bottomY - topY;
                        int lastRailX = railX[drawRailCount - 1].x;
                        int xBarGap = 40;
                        int currentClutchBarX = lastRailX + 50;
                        currentClutchBarX = max(currentClutchBarX, knobPos.x + knobRadius + xBarGap);
                        if (showYBar && mouseSteeringEnabled)
                            currentClutchBarX = max(currentClutchBarX, lastRailX + 50 + xBarGap);

                        // Calculate current fill position
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
                        int currentFillHeight = (int)(barHeight * normalizedRx);

                        bool clutchBarPositionChanged = (currentClutchBarX != lastClutchBarX);
                        bool fillHeightChanged = (currentFillHeight != lastClutchBarFillHeight);

                        // If anything changed, redraw the combined area to prevent trails
                        if (clutchBarPositionChanged || fillHeightChanged)
                        {
                            RECT clutchBarCompleteArea = CalculateClutchBarCompleteRedrawArea();
                            if (clutchBarCompleteArea.right > clutchBarCompleteArea.left)
                                InvalidateRect(hwnd, &clutchBarCompleteArea, FALSE);

                            lastClutchBarRedrawTime = currentTime; // Update timer only when actually redrawing
                        }

                        // Update tracking variables
                        lastClutchBarX = currentClutchBarX;
                        lastClutchBarFillHeight = currentFillHeight;
                    }
                }
            }
        }
        else
        {
            // WINDOWED MODE: ALWAYS FULL REDRAW
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // Update all tracked states
        lastKnobFlash = knobFlash;
        lastGreyOutState = !knobMovementEnabled;
        lastKnobMovementEnabled = knobMovementEnabled;
        lastClutchNorm = currentClutchNorm;
        lastVJoyMouseEnabled = vJoyMouseEnabled;
        lastActiveGear = activeGear;

        lastGhostKnobPos = ghostKnobPos;
        lastGhostSnappedGear = ghostSnappedGear;
        lastUseAssistPointer = useAssistPointer;

        lastMouseSteeringEnabled = mouseSteeringEnabled;
        lastShowYBar = showYBar;
        lastShowXBar = showXBar;
        lastShowClutchIndicator = showClutchIndicator;
        lastJoyY = joyY;
        lastJoyX = joyX;
        lastJoyRx = joyRx;

        lastKnobPosForXBar = knobPos;
        lastDrawRailCount = currentDrawRailCount;

        // Dynamic transparency (only for borderless mode)
// Dynamic transparency (only for borderless mode)
        if (isBorderless && !useXInput && isTransparent && dynamicTransparencyEnabled)
        {
            static DWORD lastActiveTime = 0;
            static POINT lastKnobPosTrans = knobPos;
            static DWORD lastAlphaUpdate = 0;
            static DWORD lastBrightnessCheck = 0;
            static BYTE dynamicMinAlpha = minAlpha; // Separate variable for dynamic adjustment
            static BYTE lastAppliedMinAlpha = minAlpha; // Track what's currently applied to window
            static BYTE lastUserMinAlpha = minAlpha; // Track user's slider value

            DWORD now = GetTickCount();

            // Update if user changed the slider
            if (lastUserMinAlpha != minAlpha) {
                lastUserMinAlpha = minAlpha;
                dynamicMinAlpha = minAlpha; // Reset to user's value
                lastAppliedMinAlpha = minAlpha;
                lastBrightnessCheck = 0; // Force recalc
                lastAlphaUpdate = 0; // Force redraw
            }

            // Check background brightness at 2 FPS (every 500ms) - OPTIMIZED
// Check background brightness at 2 FPS (every 500ms) - OPTIMIZED
            if (now - lastBrightnessCheck > 1500)
            {
                float brightness = GetHShifterBackgroundBrightnessDebug(hwnd);

                // Linear mapping: brightness 0-255 maps to brightnessFactor 0.2-1.0
                float brightnessFactor = 0.2f + (brightness / 255.0f) * 0.8f;
                brightnessFactor = max(0.2f, min(1.0f, brightnessFactor));

                // Calculate new dynamic minAlpha - will always be between 20% and 100% of minAlpha
                BYTE newDynamicMinAlpha = (BYTE)(minAlpha * brightnessFactor);
                newDynamicMinAlpha = max(10, min(150, newDynamicMinAlpha));

                // Add to history
                minAlphaHistory.push_back(newDynamicMinAlpha);
                if (minAlphaHistory.size() > HISTORY_SIZE)
                    minAlphaHistory.erase(minAlphaHistory.begin());

                // Check if we should update (only if significant change or history is stable)
                bool shouldUpdate = false;

                if (minAlphaHistory.size() == HISTORY_SIZE)
                {
                    // Calculate average of recent values
                    int sum = 0;
                    for (BYTE val : minAlphaHistory) sum += val;
                    BYTE recentAverage = sum / minAlphaHistory.size();

                    // Check if the new value is significantly different from recent average
                    if (abs((int)newDynamicMinAlpha - (int)recentAverage) >= CHANGE_THRESHOLD)
                    {
                        shouldUpdate = true;
                    }
                    // Also check if all recent values are similar (stable state)
                    else
                    {
                        bool allSimilar = true;
                        for (BYTE val : minAlphaHistory)
                        {
                            if (abs((int)val - (int)newDynamicMinAlpha) > (CHANGE_THRESHOLD / 2))
                            {
                                allSimilar = false;
                                break;
                            }
                        }
                        // If values are stable and different from current dynamicMinAlpha, update
                        if (allSimilar && abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD)
                        {
                            shouldUpdate = true;
                        }
                    }
                }
                else
                {
                    // Not enough history yet, use simple threshold
                    shouldUpdate = (abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD);
                }

                if (shouldUpdate)
                {
                    dynamicMinAlpha = newDynamicMinAlpha;

                    char alphaMsg[256];
                    sprintf_s(alphaMsg, "DYNAMIC ALPHA: Brightness=%.1f, UserMinAlpha=%d, DynamicMinAlpha=%d (Factor=%.1f)\n",
                        brightness, minAlpha, dynamicMinAlpha, brightnessFactor);
                    OutputDebugStringA(alphaMsg);

                    // Force alpha update on next frame since dynamicMinAlpha changed
                    lastAlphaUpdate = 0;
                }

                lastBrightnessCheck = now;
            }

            bool shiftHeld = IsKnobToggleActive();
            float deadzone = 2.0f;

            bool knobMoved = (abs(knobPos.x - lastKnobPosTrans.x) > deadzone) ||
                (abs(knobPos.y - lastKnobPosTrans.y) > deadzone);

            if (knobMoved || shiftHeld)
            {
                currentAlpha = maxAlpha;
                lastActiveTime = now;
                SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                lastAppliedMinAlpha = dynamicMinAlpha; // Track what's applied
            }
            else
            {
                if (now - lastActiveTime >= transparencyFadeDelay)
                {
                    // Limit to 15 FPS (66ms between updates) - OPTIMIZED
                    if (now - lastAlphaUpdate > 60)
                    {
                        // Use dynamicMinAlpha for the fade calculation, NOT the global minAlpha
                        BYTE newAlpha = BYTE(dynamicMinAlpha + (currentAlpha - dynamicMinAlpha) * alphaDecay);

                        // Only redraw if alpha actually changed OR if dynamicMinAlpha changed
                        if (newAlpha != currentAlpha || dynamicMinAlpha != lastAppliedMinAlpha)
                        {
                            currentAlpha = newAlpha;
                            lastAppliedMinAlpha = dynamicMinAlpha; // Track what we're applying
                            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                        }
                        lastAlphaUpdate = now;
                    }
                }
            }
            lastKnobPosTrans = knobPos;
        }

        // Settings panel scroll
        float oldOffset = settingsScrollOffsetF;
        settingsScrollOffsetF += (settingsScrollTarget - settingsScrollOffsetF) * settingsScrollSpeed;

        if (abs(settingsScrollOffsetF - oldOffset) > 0.1f)
        {
            settingsScrollOffset = (int)settingsScrollOffsetF;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }

        // Right panel scroll (Keybind, Input, Toggle)
        float oldRightOffset = rightPanelScrollOffsetF;
        rightPanelScrollOffsetF += (rightPanelScrollTarget - rightPanelScrollOffsetF) * rightPanelScrollSpeed;

        if (abs(rightPanelScrollOffsetF - oldRightOffset) > 0.1f)
        {
            rightPanelScrollOffset = (int)rightPanelScrollOffsetF;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        // Redraw update button if available
// Redraw update button if available (ONE-TIME REDRAW)
        static bool lastUpdateAvailable = false;
        if (updateAvailable && !lastUpdateAvailable)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        lastUpdateAvailable = updateAvailable;

        break;
    }
    case WM_SIZE: // <-- new case added here
        ComputeLayout(hwnd);

        ComputeIntersections();

        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right;
        int height = rc.bottom;

        // Always use double buffering
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        if (isBorderless)
        {
            // BORDERLESS MODE: Draw elements without any background
            // No background clearing - completely transparent
            DrawBorderless(memDC, width, height);
        }
        else
        {
            // WINDOWED MODE: Draw full interface
            DrawShifterGDIPlus(hwnd, memDC);
        }

        // Blit to screen
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_NCHITTEST:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int border = 8; // resize border thickness
        const int titleHeight = 30;

        // Top-left corner
        if (pt.x < border && pt.y < border)
            return HTTOPLEFT;
        // Top-right corner
        if (pt.x >= rc.right - border && pt.y < border)
            return HTTOPRIGHT;
        // Bottom-left corner
        if (pt.x < border && pt.y >= rc.bottom - border)
            return HTBOTTOMLEFT;
        // Bottom-right corner
        if (pt.x >= rc.right - border && pt.y >= rc.bottom - border)
            return HTBOTTOMRIGHT;

        // Top edge
        if (pt.y < border)
            return HTTOP;
        // Bottom edge
        if (pt.y >= rc.bottom - border)
            return HTBOTTOM;
        // Left edge
        if (pt.x < border)
            return HTLEFT;
        // Right edge
        if (pt.x >= rc.right - border)
            return HTRIGHT;

        // Custom title bar area (excluding buttons)
        if (!isBorderless && pt.y < titleHeight && pt.x < g_MaxButtonRect.left)
            return HTCAPTION;

        // ADD THIS: Bottom drag area (bottom 30 pixels)
        if (!isBorderless && pt.y >= rc.bottom - 30)
            return HTCAPTION;

        return HTCLIENT;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 1250; // minimum width
        mmi->ptMinTrackSize.y = 830;  // minimum height
        // optional: maximum size
        // mmi->ptMaxTrackSize.x = 1920;
        // mmi->ptMaxTrackSize.y = 1080;
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        POINT pt;
        GetCursorPos(&pt);           // get mouse position in screen coords
        ScreenToClient(hwnd, &pt);   // convert to client coords

        // Only scroll if mouse is inside the settings panel
        if (PtInRect(&settingsPanelRect, pt))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam); // ±120
            settingsScrollTarget += delta / 2.0f;       // adjust scroll speed

            // Clamp target
            if (settingsScrollTarget < -settingsScrollMax)
                settingsScrollTarget = -settingsScrollMax;
            if (settingsScrollTarget > 0)
                settingsScrollTarget = 0;

            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }
        // Handle scrolling for the right panel (Keybind, Input, Toggle)
        else if (pt.x >= (int)panelRect.X && pt.x <= (int)(panelRect.X + panelRect.Width) &&
                 pt.y >= (int)panelRect.Y && pt.y <= (int)(panelRect.Y + panelRect.Height))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            rightPanelScrollTarget += delta; // More aggressive: full delta instead of delta / 2.0f

            // Clamp target
            if (rightPanelScrollTarget < -rightPanelScrollMax)
                rightPanelScrollTarget = -rightPanelScrollMax;
            if (rightPanelScrollTarget > 0)
                rightPanelScrollTarget = 0;

            InvalidateRect(hwnd, NULL, FALSE); // Redraw the whole right side area
        }
    }
    break;


    case WM_DESTROY:
        // ============================================================================
        // UNINJECT ALL DLLs FROM TARGET PROCESSES BEFORE EXITING
        // ============================================================================

        // Uninject from currently selected process
        if (g_selectedProcessId != 0) {
            if (g_lastInjectedMouseProcessId != 0) {
                UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
                OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected\n");
            }
            if (g_lastInjectedXinputProcessId != 0) {
                UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
                OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected\n");
            }
        }

        // Also clean up from any previously injected processes
        if (g_lastInjectedMouseProcessId != 0 && g_lastInjectedMouseProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
            OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected from previous process\n");
        }
        if (g_lastInjectedXinputProcessId != 0 && g_lastInjectedXinputProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
            OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected from previous process\n");
        }

        // Reset injection tracking
        g_lastInjectedMouseProcessId = 0;
        g_lastInjectedXinputProcessId = 0;
        DestroyYBarWindow();

        // If neutral key was held, release it
        if (neutralHeld)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = neutralKey;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }

        // --- RELEASE ALL HELD BUTTONS BEFORE EXITING ---
        for (int btn = 1; btn <= vJoyButtonCount; ++btn)
        {
            SetBtn(FALSE, vjoyDeviceId, btn); // Release button
        }
        RelinquishVJD(vjoyDeviceId); // Release vJoy device

        OutputDebugString(L"[vJoy] All buttons released and device relinquished.\n");

        // ----- RELEASE DIRECTINPUT RESOURCES -----
        if (g_pJoystick)
        {
            g_pJoystick->Unacquire();
            g_pJoystick->Release();
            g_pJoystick = nullptr;
        }
        if (g_pDI)
        {
            g_pDI->Release();
            g_pDI = nullptr;
        }
        // ----------------------------------------

        // Save configuration if needed
        SaveConfig();
        StopThrottleBrakeThread();

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ---------------- Main ----------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    using namespace Gdiplus;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // 1️⃣ Register window class

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"HShifterWinClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int winWidth = 1250;
    int winHeight = 830;

    int posX = (screenWidth - winWidth) / 2;
    int posY = (screenHeight - winHeight) / 2;
    // 2️⃣ Create window
    hwndMain = CreateWindowEx(
        0, // no WS_EX_TOPMOST here
        wc.lpszClassName,
        L"MouseShifter",
        WS_POPUP | WS_VISIBLE, // <-- no WS_OVERLAPPEDWINDOW, no default title bar
        posX, posY, winWidth, winHeight,
        nullptr, nullptr, hInst, nullptr);
    // ✨ Edit ATS controls immediately
    processAllFiles(true);

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    InitializeProfiles();

    // 3️⃣ **Load saved config**
    LoadConfig();
    LoadGearLayoutsFromIni(L"gearlayouts.ini");

    // 4️⃣ Compute layout now that settings are loaded
    ComputeLayout(hwndMain);

    ComputeIntersections();

    // 5️⃣ Initialize raw input (if needed)
    InitRawInput(hwndMain);
    InvalidateRect(hwndMain, nullptr, TRUE);
    UpdateWindow(hwndMain);
    // 6️⃣ Message loop
    MSG msg;
    bool running = true;

    while (running)
    {
        // --- Handle Windows messages (non-blocking) ---
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                running = false;
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // --- Handle SDL controller add/remove events ---
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                RefreshGamepads();
                InvalidateRect(hwndMain, nullptr, TRUE); // 🔥 force redraw
            }
        }

        // --- (optional) You can call per-frame updates here ---
        // e.g., UpdateKnobFromXInput(hwndMain);
        Sleep(1); // tiny sleep to avoid 100% CPU
    }


    // 7️⃣ **Save config before exit**
    SaveConfig();
    // ✨ Revert ATS controls to backup before closing
    processAllFiles(false);
    if (pFactory)
        pFactory->Release();
    GdiplusShutdown(gdiplusToken);

    return 0;
}

// ==== EXTRACTED MODULES (UNITY BUILD) ====
#include "Updater.cpp"
#include "Config.cpp"
