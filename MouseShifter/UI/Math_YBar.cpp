
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
