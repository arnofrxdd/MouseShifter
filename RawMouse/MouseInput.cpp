DWORD WINAPI ThrottleBrakeThread(LPVOID lpParam)
{
    while (throttleBrakeThreadRunning)
    {
        // SKIP if using XInput OR if throttle/brake axes are disabled
        if (useXInput || !useThrottleBrakeAxes)
        {
            Sleep(16);
            continue;
        }

        EnterCriticalSection(&throttleBrakeCS);

        // Throttle control (Z axis) - W key
        bool throttlePressed = (GetAsyncKeyState(0x57) & 0x8000) != 0;

        // Brake control (Rz axis) - S key  
        bool brakePressed = (GetAsyncKeyState(0x53) & 0x8000) != 0;

        // Update throttle (Z axis)
        if (throttlePressed)
        {
            joyZ += (LONG)((axisMax - axisMin) * throttleSensitivity / 100.0f);
            joyZ = min(axisMax, joyZ);
        }
        else
        {
            // Return to 0 when released
            if (joyZ > axisMin)
            {
                joyZ -= (LONG)((axisMax - axisMin) * releaseSpeed / 100.0f);
                joyZ = max(axisMin, joyZ);
            }
        }

        // Update brake (Rz axis)
        if (brakePressed)
        {
            joyRz += (LONG)((axisMax - axisMin) * brakeSensitivity / 100.0f);
            joyRz = min(axisMax, joyRz);
        }
        else
        {
            // Return to 0 when released
            if (joyRz > axisMin)
            {
                joyRz -= (LONG)((axisMax - axisMin) * releaseSpeed / 100.0f);
                joyRz = max(axisMin, joyRz);
            }
        }

        // ALWAYS push to vJoy (no matter what UI is doing)
        SetAxis(joyZ, vjoyDeviceId, HID_USAGE_Z);
        SetAxis(joyRz, vjoyDeviceId, HID_USAGE_RZ);

        LeaveCriticalSection(&throttleBrakeCS);

        // Run at ~60 FPS for smooth input
        Sleep(16);
    }
    return 0;
}
void StartThrottleBrakeThread()
{
    if (throttleBrakeThreadRunning) return;

    InitializeCriticalSection(&throttleBrakeCS);
    throttleBrakeThreadRunning = true;
    throttleBrakeThread = CreateThread(
        NULL,
        0,
        ThrottleBrakeThread,
        NULL,
        0,
        &throttleBrakeThreadId
    );

    if (throttleBrakeThread)
    {
        // Set lower priority so it doesn't interfere with UI
        SetThreadPriority(throttleBrakeThread, THREAD_PRIORITY_BELOW_NORMAL);
    }
}

void StopThrottleBrakeThread()
{
    throttleBrakeThreadRunning = false;

    if (throttleBrakeThread)
    {
        WaitForSingleObject(throttleBrakeThread, 1000);
        CloseHandle(throttleBrakeThread);
        throttleBrakeThread = NULL;
    }

    DeleteCriticalSection(&throttleBrakeCS);
}

void ResetThrottleBrake()
{
    EnterCriticalSection(&throttleBrakeCS);
    joyZ = axisMin;
    joyRz = axisMin;
    SetAxis(joyZ, vjoyDeviceId, HID_USAGE_Z);
    SetAxis(joyRz, vjoyDeviceId, HID_USAGE_RZ);
    LeaveCriticalSection(&throttleBrakeCS);
}

void UpdateVJoyFromMouse(RAWMOUSE& rm, HANDLE hDevice)
{
    // Persistent toggle flags
    static DWORD f9PressedTime = 0; // timestamp when F9 was pressed

    // Track previous key states
    static bool wasF9Pressed = false;

    // Current key states
    bool isF9Pressed = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;

    // ----- F11 toggle -----

    // ----- F9 hold detection -----
    if (isF9Pressed)
    {
        if (!wasF9Pressed)
            f9PressedTime = GetTickCount(); // record when F9 was first pressed

        // check if held ≥ 3 seconds (3000 ms)
        if (GetTickCount() - f9PressedTime >= 3000)
        {
            // reset axes to center
            LONG defaultX = (axisMin + axisMax) / 2;
            LONG defaultY = (axisMin + axisMax) / 2;

            joyX = defaultX;
            joyY = defaultY;

            SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
            SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
        }
    }
    wasF9Pressed = isF9Pressed;
    // ----- Scroll → Rx axis test -----



    // ----- Mouse steering disabled -----
// Respect both global steering AND knob override
    if (!mouseSteeringEnabled)
    {
        LONG defaultX = (axisMin + axisMax) / 2;
        LONG defaultY = (axisMin + axisMax) / 2;

        joyX = defaultX;
        joyY = defaultY;

        SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
        SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
        return;
    }

    // ----- Mouse tracking temporarily disabled (knob toggle) -----
    if (!mouseTrackingEnabled)
    {
        // Do nothing (keep last joyX/joyY)
        return;
    }


    // ----- vJoy mouse disabled (F11) -----
    if (!vJoyMouseEnabled)
        return; // stop processing but do not reset axes

    // ----- Process selected device -----
    bool process = (g_selectedSteeringDevice == NULL || g_selectedSteeringDevice == hDevice);
    if (!process)
        return;

    float initialBoost = 8.0f; // 8× boost

    // Define center and max steering offset
    LONG centerX = (axisMax + axisMin) / 2;
    LONG axisRange = axisMax - axisMin;
    LONG steeringMaxOffset = (LONG)((maxSteeringDegrees / fullWheelDegrees) * (axisRange / 2));

    // ----- INTERDEPENDENT AXIS SMOOTHING -----


    float finalSteeringSensitivity = steeringSensitivity * initialBoost;
    float finalAccBrakeSensitivity = accBrakeSensitivity * initialBoost;

    // Apply interdependent smoothing if enabled
    if (useAxisSmoothing && axisSmoothingFactor > 0.0f)
    {
        // Calculate movement magnitude for both axes
        float xMovement = fabs((float)rm.lLastX * steeringSensitivity * initialBoost);
        float yMovement = fabs((float)rm.lLastY * accBrakeSensitivity * initialBoost);

        // Normalize movement to 0-1 range
        float maxPossibleMovement = 50.0f; // Adjust based on your typical max delta
        float xMoveFactor = min(xMovement / maxPossibleMovement, 1.0f);
        float yMoveFactor = min(yMovement / maxPossibleMovement, 1.0f);

        // Apply interdependent sensitivity reduction using single factor
        finalSteeringSensitivity *= (1.0f - (yMoveFactor * axisSmoothingFactor));
        finalAccBrakeSensitivity *= (1.0f - (xMoveFactor * axisSmoothingFactor));
    }

    // Compute new positions with adjusted sensitivities
    LONG newJoyX = joyX + (LONG)(rm.lLastX * finalSteeringSensitivity);
    LONG newJoyY = joyY + (LONG)(rm.lLastY * finalAccBrakeSensitivity);

    // ----- BRAKE RESISTANCE FACTOR -----
    if (brakeresistanceFactor > 0.0f && rm.lLastY > 0) // Only apply resistance when moving downward (-Y direction, brakes)
    {
        // Calculate how far we are into the brake range (0.0 to 1.0)
        // Center is where brakes start, axisMax is full brakes
        LONG centerY = (axisMin + axisMax) / 2;
        float brakePosition = (float)(newJoyY - centerY) / (float)(axisMax - centerY);

        // Clamp brake position between 0 and 1
        brakePosition = max(0.0f, min(1.0f, brakePosition));

        // Apply resistance: higher resistance as brake position increases
        float resistance = brakeresistanceFactor * brakePosition;
        float resistanceMultiplier = 1.0f / (1.0f + resistance);

        // Only apply resistance to downward movement
        LONG actualMovement = newJoyY - joyY;
        if (actualMovement > 0) // Moving downward
        {
            LONG resistedMovement = (LONG)(actualMovement * resistanceMultiplier);
            newJoyY = joyY + resistedMovement;
        }
    }

    // ----- ACCELERATION RESISTANCE FACTOR -----
    if (accelerationResistanceFactor > 0.0f && rm.lLastY < 0) // Only apply resistance when moving upward (+Y direction, acceleration)
    {
        // Calculate how far we are into the acceleration range (0.0 to 1.0)
        // Center is where acceleration starts, axisMin is full acceleration
        LONG centerY = (axisMin + axisMax) / 2;
        float accelerationPosition = (float)(centerY - newJoyY) / (float)(centerY - axisMin);

        // Clamp acceleration position between 0 and 1
        accelerationPosition = max(0.0f, min(1.0f, accelerationPosition));

        // Apply resistance: higher resistance as acceleration position increases
        float resistance = accelerationResistanceFactor * accelerationPosition;
        float resistanceMultiplier = 1.0f / (1.0f + resistance);

        // Only apply resistance to upward movement
        LONG actualMovement = joyY - newJoyY; // Positive when moving upward
        if (actualMovement > 0) // Moving upward
        {
            LONG resistedMovement = (LONG)(actualMovement * resistanceMultiplier);
            newJoyY = joyY - resistedMovement;
        }
    }

    // Clamp X to steering limit (hard lock)
    joyX = max(centerX - steeringMaxOffset, min(centerX + steeringMaxOffset, newJoyX));

    // Clamp Y to axis range
    joyY = max(axisMin, min(axisMax, newJoyY));

    // Push to vJoy
    SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
    SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
}
// Add these global variables at the top of your file
static LONG targetJoyRx = (axisMin + axisMax) / 2;  // Start at center

// New function to handle scroll input
void HandleScrollInput(RAWMOUSE& rm)
{
    if (useScrollClutch && (rm.usButtonFlags & RI_MOUSE_WHEEL))
    {
        SHORT wheelDelta = (SHORT)rm.usButtonData; // +120 / -120

        float scrollBoost = 2.0f;
        float step = scrollClutchSens * ((float)(axisMax - axisMin) / 100.0f) * scrollBoost;

        // Update target position
        if (invertScrollClutchAxis)
            targetJoyRx -= (LONG)((wheelDelta / 120.0f) * step);
        else
            targetJoyRx += (LONG)((wheelDelta / 120.0f) * step);

        // ---- HALF-AXIS MODE ----
        if (useHalfClutch)
        {
            LONG mid = (axisMin + axisMax) / 2;
            targetJoyRx = max(mid, min(axisMax, targetJoyRx));
        }

        // Clamp target
        targetJoyRx = max(axisMin, min(axisMax, targetJoyRx));
    }
}

// New function to update smooth scroll (call from WM_TIMER)
void UpdateSmoothScroll()
{
    if (joyRx != targetJoyRx)
    {
        // Smooth interpolation
        joyRx = joyRx + (LONG)((targetJoyRx - joyRx) / smoothScrollSpeed);

        // Snap to target when very close
        if (abs(targetJoyRx - joyRx) < 5)
            joyRx = targetJoyRx;

        // Clamp final
        joyRx = max(axisMin, min(axisMax, joyRx));
        SetAxis(joyRx, vjoyDeviceId, HID_USAGE_RX);
    }
}
