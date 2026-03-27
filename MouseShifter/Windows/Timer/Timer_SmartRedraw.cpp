        // === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
// === SEPARATE REDRAW LOGIC FOR BORDERLESS vs WINDOWED ===
        // In your WM_TIMER case, replace the borderless section with this:

        if (isBorderless)
        {
    #include "Windows/Timer/Timer_Redraw_Knob.cpp"
    #include "Windows/Timer/Timer_Redraw_Bars.cpp"
        }
        else
        {
            // WINDOWED MODE: ALWAYS FULL REDRAW
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        // Update all tracked states
        lastKnobFlash = knobFlash;
        lastGreyOutState = !knobMovementEnabled;
        lastKnobMovementEnabled = knobMovementEnabled;
        lastClutchNorm = currentClutchNorm;
        lastVJoyMouseEnabled = vJoyMouseEnabled;
        lastActiveGear = activeGear;

        lastGhostKnobPos = ghostKnobPos;
        lastGhostSnappedGear = ghostSnappedGear;
        lastUseAssistPointer = useAssistPointer;

        lastMouseSteeringEnabled = mouseSteeringEnabled;
        lastShowYBar = showYBar;
        lastShowXBar = showXBar;
        lastShowClutchIndicator = showClutchIndicator;
        lastJoyY = joyY;
        lastJoyX = joyX;
        lastJoyRx = joyRx;

        lastKnobPosForXBar = knobPos;
        lastDrawRailCount = currentDrawRailCount;


