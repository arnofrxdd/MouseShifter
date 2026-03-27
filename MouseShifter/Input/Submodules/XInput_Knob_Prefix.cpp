

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