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
