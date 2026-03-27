bool IsReverseUnlockActive();

void ClampMovementForReverseLock(int& newY, int currentX)
{
    if (reverseLockEnabled && !IsReverseUnlockActive())
    {
        POINT rPos = { 0,0 };
        bool rFound = false;

        // Check both maps to handle range switching
        auto findR = [&](const std::map<std::string, POINT>& map) {
            auto it = map.find("R");
            if (it != map.end()) { rPos = it->second; rFound = true; }
        };
        findR(lowerGearPositions);
        if (!rFound) findR(highGearPositions);

        if (rFound)
        {
            // Anti-Teleport Latch logic (Directional):
            // Only allow bypass if we are already "deep" in the Reverse side.
            // If Reverse is Top, we must be UP (y < centerY). If Reverse is Bottom, we must be DOWN (y > centerY).
            // This prevents bypassing from the OPPOSITE gear (which is also "far from center" but on the wrong side).
            bool safeSide = false;
            if (activeGear == "R")
            {
                safeSide = true;
            }
            else
            {
                int buffer = 15;
                if (rPos.y == topY && knobPos.y < centerY - buffer) safeSide = true;       // Already up in Reverse zone
                else if (rPos.y == bottomY && knobPos.y > centerY + buffer) safeSide = true; // Already down in Reverse zone
            }

            if (safeSide)
                return;

            // Check if any other gear has same X (same rail)
            bool hasOpposite = false;
            auto checkOpposite = [&](const std::map<std::string, POINT>& map) {
                for (auto& kv : map) {
                    if (kv.first != "R" && abs(kv.second.x - rPos.x) < 5) {
                        hasOpposite = true;
                        break;
                    }
                }
            };
            checkOpposite(lowerGearPositions);
            if (!hasOpposite) checkOpposite(highGearPositions);

            // If Reverse is on an ISOLATED rail (no opposite gear), and we are physically IN that rail
            // (meaning we passed the horizontal lock), then ALLOW vertical movement.
            // The Horizontal Lock is the guard; once you're in, you're in.
            if (!hasOpposite)
            {
                if (abs(currentX - rPos.x) < enterVerticalThreshold)
                    return; // Bypass vertical clamp
            }

            // Otherwise (Shared Rail, or not aligned), enforce strict vertical locking
             // Use WIDER threshold to catch diagonal/side entries (prevent sneaking in from the side)
             if (abs(currentX - rPos.x) < (enterVerticalThreshold * 3))
             {
                 if (rPos.y == topY) { // Reverse is at top
                     if (newY < centerY) newY = centerY;
                 }
                 else if (rPos.y == bottomY) { // Reverse is at bottom
                     if (newY > centerY) newY = centerY;
                 }
             }
        }
    }
}

void ClampHorizontalMovementForReverseLock(LONG& newX, int currentY)
{
    if (reverseLockEnabled && !IsReverseUnlockActive())
    {
        POINT rPos = { 0,0 };
        bool rFound = false;

        auto findR = [&](const std::map<std::string, POINT>& map) {
            auto it = map.find("R");
            if (it != map.end()) { rPos = it->second; rFound = true; }
        };
        findR(lowerGearPositions);
        if (!rFound) findR(highGearPositions);

        if (rFound)
        {
            // Check if any other gear has same X (same rail)
            bool hasOpposite = false;
            auto checkOpposite = [&](const std::map<std::string, POINT>& map) {
                for (auto& kv : map) {
                    if (kv.first != "R" && abs(kv.second.x - rPos.x) < 5) {
                        hasOpposite = true;
                        break;
                    }
                }
            };
            checkOpposite(lowerGearPositions);
            if (!hasOpposite) checkOpposite(highGearPositions);

            // If Reverse is on an ISOLATED rail (no opposite gear), ALWAYS clamp.
            // If Reverse SHARES a rail (hasOpposite), we normally allow entry (for the opposite gear).
            // BUT: If key is NOT held, and we are entering the "Reverse Zone" (vertical height), we must CLAMP.
            // This prevents "Flanking": moving diagonally into the Reverse side of a shared rail.
            bool shouldClamp = !hasOpposite;
            if (hasOpposite)
            {
                 // Check if we are in the "Danger Zone" vertically
                 if (rPos.y == topY && currentY < centerY) shouldClamp = true;       // Top Reverse: We are in upper half
                 else if (rPos.y == bottomY && currentY > centerY) shouldClamp = true; // Bottom Reverse: We are in lower half
            }

            if (shouldClamp)
            {
                // Find index of R rail
                int rRailIndex = -1;
                for (int i = 0; i < railCount; ++i) {
                    if (abs(railX[i].x - rPos.x) < 5) {
                        rRailIndex = i;
                        break;
                    }
                }

                if (rRailIndex != -1)
                {
                    // Find closest rail towards center (adjacent)
                    int adjX = centerX;
                    int minDiff = 99999;
                    bool foundAdj = false;

                    for (int i = 0; i < railCount; ++i)
                    {
                        if (i == rRailIndex) continue;

                        int diff = abs(railX[i].x - rPos.x);
                        if (diff < minDiff)
                        {
                            minDiff = diff;
                            adjX = railX[i].x;
                            foundAdj = true;
                        }
                    }

                    if (foundAdj)
                    {
                        // Check if we are already "inside" or closer to the Reverse rail than the Adjacent rail
                        // If so, we are already past the gate, so DO NOT clamp (latch logic)
                        // This prevents teleporting back if unlock is released while in/near Reverse
                        if (activeGear == "R" || abs(knobPos.x - rPos.x) < abs(knobPos.x - adjX))
                        {
                            return;
                        }

                        // Strict clamp to adjacent rail (with small buffer to allow entering the adjacent rail comfortably)
                        int railDist = abs(rPos.x - adjX);
                        int buffer = 25; 
                        
                        // Dynamic buffer safety: ensure buffer is strictly less than half distance to prevent crossing midpoint
                        // If buffer pushes us past midpoint, the "Are we inside?" check (dist < dist) flips to TRUE next frame
                        int maxSafeBuffer = (railDist / 2) - 2; 
                        if (maxSafeBuffer < 0) maxSafeBuffer = 0;
                        
                        if (buffer > maxSafeBuffer)
                            buffer = maxSafeBuffer;

                        if (rPos.x < adjX) // R is to the Left (e.g. -120), Adj is Right (e.g. -40)
                        {
                             // Prevent going left past Adj
                             if (newX < adjX - buffer)
                                 newX = adjX - buffer;
                        }
                        else // R is to the Right (e.g. 120), Adj is Left (e.g. 40)
                        {
                             // Prevent going right past Adj
                             if (newX > adjX + buffer)
                                 newX = adjX + buffer;
                        }                    }
                }
            }
        }
    }
}
