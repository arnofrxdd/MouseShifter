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
