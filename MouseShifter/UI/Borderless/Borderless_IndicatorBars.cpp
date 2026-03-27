
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
