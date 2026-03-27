#include <mutex>
#include <chrono>

std::mutex rumbleMutex;
std::chrono::steady_clock::time_point lastRumbleTime;

void SafeRumble(int left, int right, int durationMs)
{
    std::lock_guard<std::mutex> lock(rumbleMutex);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRumbleTime).count();

    if (elapsed < 60)
        return; // debounce rumble spam

    for (auto& g : g_gamepads)
    {
        if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, 0, 0, 0);
            SDL_HapticStopAll(g.haptic);
        }

        if (g.haptic)
        {
            SDL_HapticRumblePlay(g.haptic, (std::max)(left, right) / 65535.0f, durationMs);
        }
        else if (g.controller)
        {
            SDL_GameControllerRumble(g.controller, left, right, durationMs);
        }
    }

    lastRumbleTime = now;
}



void UpdateKnobFromXInput(HWND hwnd)
{
    if (!useXInput)
        return;
    float dx = 0.0f, dy = 0.0f;
    g = nullptr; // reset at start
    if (PlayStationMode && g_selectedGamepadIndex >= 0 && g_selectedGamepadIndex < g_gamepads.size()) {
        SDL_GameControllerUpdate();
        g = &g_gamepads[g_selectedGamepadIndex]; // use global pointer



        int LX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_LEFTX);
        int LY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_LEFTY);
        int RX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int RY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTY);

        // Deadzone
        const float DEADZONE = 1000.0f;
        auto fixAxis = [DEADZONE](int v) { return (abs(v) < DEADZONE) ? 0 : (float)v; };
        float fx = fixAxis(useRightStick ? RX : LX);
        float fy = fixAxis(useRightStick ? RY : LY);
        if (invertPlayStationYAxis) fy = -fy;

        // Apply acceleration
        const float BASE_SENSITIVITY = 0.25f;
        const float ACCEL_FACTOR = 5.5f;
        float magnitude = sqrt(fx * fx + fy * fy);
        if (magnitude > DEADZONE) {
            float normX = fx / magnitude;
            float normY = fy / magnitude;
            float adjustedMag = pow(magnitude / 32767.0f, ACCEL_FACTOR) * BASE_SENSITIVITY * controllerSensMultiplier * 50.0f;
            dx = normX * adjustedMag;
            dy = normY * adjustedMag;
        }
        else {
            dx = 0.0f;
            dy = 0.0f;
        }



        if (assistButtonBeingSet && g) { // only check buttons if g is valid
            for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; ++b) {
                if (SDL_GameControllerGetButton(g->controller, (SDL_GameControllerButton)b)) {
                    assistButton = b;              // store SDL button index, not a mask
                    assistButtonBeingSet = false;
                    break;                         // stop after first detected button
                }
            }
        }

    }










    // --- Check assist buttons (RB / LB) ---
    if (g) {
        assistButtonHeld = SDL_GameControllerGetButton(
            g->controller,
            (SDL_GameControllerButton)assistButton
        );
    }



    // If assist just pressed, initialize ghost knob at real knob
    if (invertAssistAxes)
    {
        // We don't want ghost knob at all
        useAssistPointer = false; // ignore ghost knob completely
    }
    else
    {
        // original logic
        if (assistButtonHeld && !useAssistPointer)
        {
            ghostKnobPos = knobPos;
            ghostSnappedGear = "";
        }
        useAssistPointer = assistButtonHeld;
    }



    // PlayStationMode already calculated dx/dy above, so we use those values

    // --- Unified knob handling (ghost + real) ---
    POINT* targetKnob = &knobPos; // default to real knob

    if (invertAssistAxes && disableRealKnobMovement)
    {
        // Real knob always moves, unless assist button is held
        if (assistButtonHeld)
        {
            dx = 0.0f;
            dy = 0.0f;
        }
    }
    else
    {
        // original logic for non-inverted
        if (useAssistPointer)
            targetKnob = &ghostKnobPos;

        if (disableRealKnobMovement)
        {
            targetKnob = &ghostKnobPos;
            if (!useAssistPointer)
            {
                dx = 0.0f;
                dy = 0.0f;
            }
        }
    }

    // --- Snap detection for ghost / targetKnob ---
// --- Snap detection for ghost / targetKnob ---
    auto& activeMap = lowerGearPositions;
    std::string snappedGear = "";

    if (layoutType == 11) // PRNDL layout
    {
        // For PRNDL, only consider vertical distance for snapping
        float closestDist = FLT_MAX;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;

            // For PRNDL, only vertical distance matters
            double dist = abs(targetKnob->y - g.y);
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                snappedGear = gear;
                closestDist = dist;
            }
        }
    }
    else
    {
        // Regular diamond layout snapping
        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
                (targetKnob->y - g.y) * (targetKnob->y - g.y));
            if (dist < gearSnapInThreshold)
            {
                snappedGear = gear;
                break;
            }
        }
    }

    bool isSnapped = !snappedGear.empty();

    if (isSnapped)
    {
        const int maxHorizontalOffset = 20;
        POINT g = activeMap[snappedGear];
        targetKnob->x += (int)roundf(dx * 0.2f);
        if (targetKnob->x < g.x - maxHorizontalOffset)
            targetKnob->x = g.x - maxHorizontalOffset;
        if (targetKnob->x > g.x + maxHorizontalOffset)
            targetKnob->x = g.x + maxHorizontalOffset;
        targetKnob->y += (int)roundf(dy);

        // --- Only update gear if targetKnob is the real knob ---
        // --- Only update gear if targetKnob is the real knob ---
        if (!useAssistPointer)
        {
            SetGearKey(snappedGear);
            activeGear = snappedGear;
            lockedInGear = true;

            // RELEASE NEUTRAL immediately
            if (neutralHeld)
            {
                GearInput gi = gearInputMap["N"];
                if (gi.type == KEYBOARD)
                {
                    INPUT input = {};
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = gi.code;
                    input.ki.dwFlags = KEYEVENTF_KEYUP;
                    SendInput(1, &input, sizeof(INPUT));
                }
                else if (gi.type == VJOY_BUTTON)
                {
                    SetBtn(false, vjoyDeviceId, gi.code);
                }
                neutralHeld = false;
                if (!isBorderless) {
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();

                        char debugBuf[256];
                        sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                            keybindAnimations["N"].isHeld);
                        OutputDebugStringA(debugBuf);

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                }
            }
        }
    }

    else
    {
        if (layoutType == 11) // PRNDL layout
        {
            // PRNDL: Vertical movement only with strong horizontal centering
            targetKnob->y += (int)roundf(dy);
            targetKnob->x += (int)roundf(dx * 0.1f); // Minimal horizontal movement
            targetKnob->x += (int)roundf((railX[0].x - targetKnob->x) * 0.3f); // Strong center pull

            // Reset rail state for PRNDL (no rail switching)
            currentRail = VERTICAL;
            currentVerticalIndex = 0;
        }
        else
        {
            // --- Not snapped: regular rail + lerp logic ---
            bool insideIntersection = IsInsideIntersection(targetKnob->x, targetKnob->y);
            if (currentRail == HORIZONTAL)
            {
                targetKnob->x += (int)roundf(dx);

                if (insideIntersection)
                {
                    targetKnob->y += (int)roundf(dy);

                    int railCount = is16GearSet ? 5 : 4;
                    for (int i = 0; i < railCount; ++i)
                    {
                        int railXPos = railX[i].x;
                        if (abs(targetKnob->x - railXPos) < enterVerticalThreshold && abs(dy) > 0)
                        {
                            bool movingDown = dy > 0;
                            bool movingUp = dy < 0;
                            bool canMoveVertically = false;

                            for (auto& kv : activeMap)
                            {
                                if (kv.second.x == railXPos)
                                {
                                    if (movingDown && kv.second.y == bottomY)
                                        canMoveVertically = true;
                                    if (movingUp && kv.second.y == topY)
                                        canMoveVertically = true;
                                }
                            }

                            if (canMoveVertically)
                            {
                                currentRail = VERTICAL;
                                currentVerticalIndex = i;
                            }
                            break;
                        }
                    }
                }
                else
                {
                    targetKnob->y += (int)roundf(dy * 0.2f);
                    targetKnob->y += (int)roundf((centerY - targetKnob->y) * 0.05f);
                }
            }
            else if (currentRail == VERTICAL && currentVerticalIndex != -1)
            {
                targetKnob->y += (int)roundf(dy);
                int railCenter = railX[currentVerticalIndex].x;
                targetKnob->x += (int)roundf(dx * 0.2f);
                targetKnob->x += (int)roundf((railCenter - targetKnob->x) * 0.07f);

                if (IsInsideIntersection(targetKnob->x, targetKnob->y) && abs(dx) > 0)
                {
                    currentRail = HORIZONTAL;
                    currentVerticalIndex = -1;
                }
            }
        }
    }

    // --- Clamp knob ---
    if (targetKnob->x < knobMinX)
        targetKnob->x = knobMinX;
    if (targetKnob->x > knobMaxX)
        targetKnob->x = knobMaxX;
    if (targetKnob->y < knobMinY)
        targetKnob->y = knobMinY;
    if (targetKnob->y > knobMaxY)
        targetKnob->y = knobMaxY;

    // --- Snap knob ---
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((targetKnob->x - g.x) * (targetKnob->x - g.x) +
            (targetKnob->y - g.y) * (targetKnob->y - g.y));
        if (dist < gearSnapInThreshold)
        {
            snappedGear = gear;
            break;
        }
    }

    // --- Handle ghost teleport if assist released ---
    // --- Animate real knob toward ghost knob if assist released ---
    if (!useAssistPointer && !ghostSnappedGear.empty())
    {
        POINT target = lowerGearPositions[ghostSnappedGear];

        // Linear interpolation for smooth movement
        float lerpFactor = 0.50f; // tweak for speed
        knobPos.x = knobPos.x + (int)roundf((target.x - knobPos.x) * lerpFactor);
        knobPos.y = knobPos.y + (int)roundf((target.y - knobPos.y) * lerpFactor);

        // Check if close enough to snap to target
        if (abs(knobPos.x - target.x) < 2 && abs(knobPos.y - target.y) < 2)
        {
            knobPos = target;
            SetGearKey(ghostSnappedGear);
            activeGear = ghostSnappedGear;
            lockedInGear = true;

            // Release neutral key now that we're at the gear
            if (neutralHeld)
            {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = gearInputMap["N"].code;
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                neutralHeld = false;
                if (!isBorderless) {
                    if (keybindAnimations.find("N") != keybindAnimations.end()) {
                        keybindAnimations["N"].isHeld = false;
                        keybindAnimations["N"].isActive = false;
                        keybindAnimations["N"].activationTime = GetTickCount();

                        char debugBuf[256];
                        sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                            keybindAnimations["N"].isHeld);
                        OutputDebugStringA(debugBuf);

                        InvalidateRect(hwnd, nullptr, FALSE);
                    }
                }
            }

            ghostSnappedGear = "";
        }
    }

    if (useAssistPointer)
    {
        // Handle snapping for ghost knob with hysteresis (like real knob)
        std::string gearToSnap = "";
        double closestDist = 1e9;

        for (auto& kv : activeMap)
        {
            POINT g = kv.second;
            std::string gear = kv.first;
            double dist = sqrt((ghostKnobPos.x - g.x) * (ghostKnobPos.x - g.x) +
                (ghostKnobPos.y - g.y) * (ghostKnobPos.y - g.y));
            if (dist < gearSnapInThreshold && dist < closestDist)
            {
                closestDist = dist;
                gearToSnap = gear;
            }
        }

        if (!ghostKnobSnapped && !gearToSnap.empty())
        {
            ghostKnobPos = activeMap[gearToSnap];
            ghostSnappedGear = gearToSnap;
            ghostKnobSnapped = true;       // you need to define this boolean similar to realKnobSnapped
// Replace: PulseVibrationAsync(0, 65535);
            SafeRumble(0, 65535, 100); // ghost knob
        }
        else if (ghostKnobSnapped && gearToSnap.empty())
        {
            ghostSnappedGear = "";
            ghostKnobSnapped = false;
        }
    }

    // --- Handle snapping for real knob with hysteresis ---
    std::string gearToSnap = "";
    double closestDist = 1e9;

    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        std::string gear = kv.first;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold && dist < closestDist)
        {
            closestDist = dist;
            gearToSnap = gear;
        }
    }

    if (!realKnobSnapped && !gearToSnap.empty())
    {
        knobPos = activeMap[gearToSnap];
        SetGearKey(gearToSnap);
        activeGear = gearToSnap;
        lockedInGear = true;
        realKnobSnapped = true;
        // Replace: PulseVibrationAsync(65535, 0);
        SafeRumble(65535, 0, 100); // real knob

        if (neutralHeld)
        {
            GearInput gi = gearInputMap["N"];
            if (gi.type == KEYBOARD)
            {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = gi.code;
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
            }
            else if (gi.type == VJOY_BUTTON)
            {
                SetBtn(false, vjoyDeviceId, gi.code);
            }
            neutralHeld = false;
            if (!isBorderless) {
                if (keybindAnimations.find("N") != keybindAnimations.end()) {
                    keybindAnimations["N"].isHeld = false;
                    keybindAnimations["N"].isActive = false;
                    keybindAnimations["N"].activationTime = GetTickCount();

                    char debugBuf[256];
                    sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                        keybindAnimations["N"].isHeld);
                    OutputDebugStringA(debugBuf);

                    InvalidateRect(hwnd, nullptr, FALSE);
                }
            }
        }
    }

    else if (realKnobSnapped && gearToSnap.empty())
    {
        ReleaseGearKey();
        activeGear = "";
        lockedInGear = false;
        realKnobSnapped = false;
    }

    // --- Handle Neutral for real knob ---
    bool inNeutral = true;
    for (auto& kv : activeMap)
    {
        POINT g = kv.second;
        double dist = sqrt((knobPos.x - g.x) * (knobPos.x - g.x) +
            (knobPos.y - g.y) * (knobPos.y - g.y));
        if (dist < gearSnapInThreshold)
        {
            inNeutral = false;
            break;
        }
    }

    if (inNeutral && isNeutralEnabled && !neutralHeld)
    {
        GearInput gi = gearInputMap["N"];
        if (gi.type == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = gi.code;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(true, vjoyDeviceId, gi.code);
        }
        if (!isBorderless) {
            if (keybindAnimations.find("N") == keybindAnimations.end()) {
                keybindAnimations["N"] = KeybindAnimation();
            }
            keybindAnimations["N"].isHeld = true;
            keybindAnimations["N"].isActive = false;
            keybindAnimations["N"].glowAlpha = MAX_GLOW_ALPHA;
            keybindAnimations["N"].activationTime = GetTickCount();
        }
        neutralHeld = true;
    }
    else if (!inNeutral && neutralHeld)
    {
        GearInput gi = gearInputMap["N"];
        if (gi.type == KEYBOARD)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = gi.code;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }
        else if (gi.type == VJOY_BUTTON)
        {
            SetBtn(false, vjoyDeviceId, gi.code);
        }
        // === STOP NEUTRAL GLOW ===
        if (!isBorderless) {
            if (keybindAnimations.find("N") != keybindAnimations.end()) {
                keybindAnimations["N"].isHeld = false;
                keybindAnimations["N"].isActive = false;
                keybindAnimations["N"].activationTime = GetTickCount();

                char debugBuf[256];
                sprintf_s(debugBuf, "=== STOP NEUTRAL GLOW in section [X]: isHeld=%d ===\n",
                    keybindAnimations["N"].isHeld);
                OutputDebugStringA(debugBuf);

                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        else {
            OutputDebugStringA("=== NEUTRAL GLOW NOT FOUND IN ANIMATIONS ===\n");
        }


        neutralHeld = false;
    }

    // --- DEBUG ---

    // --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
// --- Dynamic alpha transparency ---
    if (isBorderless && isTransparent && dynamicTransparencyEnabled)
    {
        static DWORD lastActiveTime = 0;
        static DWORD lastAlphaUpdate = 0;
        static DWORD lastBrightnessCheck = 0;
        static BYTE dynamicMinAlpha = minAlpha;
        static BYTE lastAppliedMinAlpha = minAlpha;
        static BYTE lastUserMinAlpha = minAlpha;

        // === ADD HISTORY TRACKING FOR CONTROLLER ===
        static std::vector<BYTE> minAlphaHistory;
        static const int HISTORY_SIZE = 3;
        static const BYTE CHANGE_THRESHOLD = 8;

        DWORD now = GetTickCount();

        // Update if user changed the slider
        if (lastUserMinAlpha != minAlpha) {
            lastUserMinAlpha = minAlpha;
            dynamicMinAlpha = minAlpha;
            lastAppliedMinAlpha = minAlpha;
            minAlphaHistory.clear(); // Clear history on user change
            lastBrightnessCheck = 0;
            lastAlphaUpdate = 0;
        }

        // Check background brightness at 2 FPS (every 500ms) - SAME AS MOUSE VERSION
        if (now - lastBrightnessCheck > 1500)
        {
            float brightness = GetHShifterBackgroundBrightnessDebug(hwnd);

            // Linear mapping: brightness 0-255 maps to brightnessFactor 0.2-1.0
            float brightnessFactor = 0.2f + (brightness / 255.0f) * 0.8f;
            brightnessFactor = max(0.2f, min(1.0f, brightnessFactor));

            BYTE newDynamicMinAlpha = (BYTE)(minAlpha * brightnessFactor);
            newDynamicMinAlpha = max(10, min(150, newDynamicMinAlpha));

            // Add to history
            minAlphaHistory.push_back(newDynamicMinAlpha);
            if (minAlphaHistory.size() > HISTORY_SIZE)
                minAlphaHistory.erase(minAlphaHistory.begin());

            // Smart update check using history (SAME LOGIC AS MOUSE)
            bool shouldUpdate = false;

            if (minAlphaHistory.size() == HISTORY_SIZE)
            {
                int sum = 0;
                for (BYTE val : minAlphaHistory) sum += val;
                BYTE recentAverage = sum / minAlphaHistory.size();

                // Only update if significant deviation from recent trend
                if (abs((int)newDynamicMinAlpha - (int)recentAverage) >= CHANGE_THRESHOLD)
                {
                    shouldUpdate = true;
                }
                else
                {
                    // Check if all recent values are similar (stable state)
                    bool allSimilar = true;
                    for (BYTE val : minAlphaHistory)
                    {
                        if (abs((int)val - (int)newDynamicMinAlpha) > (CHANGE_THRESHOLD / 2))
                        {
                            allSimilar = false;
                            break;
                        }
                    }
                    // If values are stable and different from current dynamicMinAlpha, update
                    if (allSimilar && abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD)
                    {
                        shouldUpdate = true;
                    }
                }
            }
            else
            {
                // Not enough history, use simple threshold
                shouldUpdate = (abs((int)newDynamicMinAlpha - (int)dynamicMinAlpha) >= CHANGE_THRESHOLD);
            }

            if (shouldUpdate)
            {
                dynamicMinAlpha = newDynamicMinAlpha;
                lastAlphaUpdate = 0; // Force redraw on next frame
            }

            lastBrightnessCheck = now;
        }

        // AGGRESSIVE 15 FPS LIMIT (66ms between updates)
        if (now - lastAlphaUpdate >= 66)
        {
            // Detect activity: LB/RB held OR slight knob movement
            bool inputActive = (abs(dx) > 0.1f || abs(dy) > 0.1f);
            if (!invertAssistAxes)
                inputActive = inputActive || assistButtonHeld;

            if (inputActive)
            {
                currentAlpha = maxAlpha;
                lastActiveTime = now; // reset timer
                SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                lastAppliedMinAlpha = dynamicMinAlpha; // Track what's applied
            }
            else if (now - lastActiveTime >= transparencyFadeDelay)
            {
                // Fade back to DYNAMIC minAlpha (not static minAlpha) smoothly
                BYTE newAlpha = BYTE(dynamicMinAlpha + (currentAlpha - dynamicMinAlpha) * alphaDecay);

                // Only update if alpha actually changed OR if dynamicMinAlpha changed
                if (newAlpha != currentAlpha || dynamicMinAlpha != lastAppliedMinAlpha)
                {
                    currentAlpha = newAlpha;
                    lastAppliedMinAlpha = dynamicMinAlpha; // Track what we're applying
                    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_ALPHA | LWA_COLORKEY);
                }
            }

            lastAlphaUpdate = now; // Update FPS timer
        }
    }


}
#include <chrono>

// Globals
std::chrono::steady_clock::time_point rbReleaseTime;
bool rbWasPressed = false;
const int RB_DELAY_MS = 40;
void UpdateVJoyFromXInput()
{
    if (g_selectedGamepadIndex < 0 || g_selectedGamepadIndex >= g_gamepads.size())
        return;

    Gamepad* g = &g_gamepads[g_selectedGamepadIndex];
    if (!g->controller)
        return;

    SDL_GameControllerUpdate();

    // --- LT as clutch ---
    if (useLTAsClutch && !useThrottleBrakeAxes)
    {
        // SDL axis range is -32768 → 32767 for triggers too, but SDL triggers usually 0–32767
        int ltValue = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);

        LONG zAxisValue;
        if (bindingModeForRAxis && ltValue > 1000) // Little bit of movement detection for clutch
        {
            zAxisValue = axisMax; // Full value when binding mode is active
        }
        else
        {
            zAxisValue = axisMin + (ltValue / 32767.0) * (axisMax - axisMin);
        }
        SetAxis(zAxisValue, vjoyDeviceId, HID_USAGE_Z);
    }

    if (!disableRealKnobMovement)
        return;

    // --- Assist button ---
    bool rbPressed = false;
    if (assistButton >= 0 && assistButton < SDL_CONTROLLER_BUTTON_MAX)
        rbPressed = SDL_GameControllerGetButton(g->controller, (SDL_GameControllerButton)assistButton) != 0;

    // --- Inverted assist axis logic ---
    if (invertAssistAxes)
    {
        if (!rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            return;
        }
        // else → assist pressed → continue normal stick processing
    }
    else
    {
        if (rbPressed)
        {
            joyRX = (axisMin + axisMax) / 2;
            joyRY = (axisMin + axisMax) / 2;
            SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
            SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
            rbWasPressed = true;
            return;
        }
        else if (rbWasPressed)
        {
            rbReleaseTime = std::chrono::steady_clock::now();
            rbWasPressed = false;
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if ((now - rbReleaseTime) < std::chrono::milliseconds(RB_DELAY_MS))
            return;
    }

    // --- Process right stick normally ---
    int sThumbRX = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTX);
    int sThumbRY = SDL_GameControllerGetAxis(g->controller, SDL_CONTROLLER_AXIS_RIGHTY);

    double normRX = 0.0, normRY = 0.0;
    const int DEADZONE = 8000; // adjust as needed

    if (bindingModeForRAxis)
    {
        // Binding mode: go to full value with little movement
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = (sThumbRX > 0) ? 1.0 : -1.0; // Full value
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = (sThumbRY > 0) ? 1.0 : -1.0; // Full value
    }
    else
    {
        // Normal mode
        if (sThumbRX > DEADZONE || sThumbRX < -DEADZONE)
            normRX = sThumbRX / 32767.0;
        if (sThumbRY > DEADZONE || sThumbRY < -DEADZONE)
            normRY = sThumbRY / 32767.0;
    }

    joyRX = (LONG)(axisMin + (normRX + 1.0) * 0.5 * (axisMax - axisMin));
    joyRY = (LONG)(axisMin + (normRY + 1.0) * 0.5 * (axisMax - axisMin));

    SetAxis(joyRX, vjoyDeviceId, HID_USAGE_RX);
    SetAxis(joyRY, vjoyDeviceId, HID_USAGE_RY);
}
