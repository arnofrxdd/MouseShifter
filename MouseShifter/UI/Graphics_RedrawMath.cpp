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
