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
