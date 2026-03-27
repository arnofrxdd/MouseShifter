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

