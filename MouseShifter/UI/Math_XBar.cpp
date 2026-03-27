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


