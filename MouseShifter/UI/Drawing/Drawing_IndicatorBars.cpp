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





