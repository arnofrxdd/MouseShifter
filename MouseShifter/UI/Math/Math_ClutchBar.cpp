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
