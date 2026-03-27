        // ADD THESE TWO FLAGS AT THE VERY TOP
        if (!isBorderless) {
            if (wParam == 1001 && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = true;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            if ((gearLayoutDropdownOpen || hShifterLayoutDropdownOpen) && currentTooltip) {
                KillTimer(hwnd, currentTooltip->timerId);
                currentTooltip->show = false;
                currentTooltip = nullptr;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
        }
        UpdateKnobMovement(hwnd);
        UpdateAutoInjection();
        UpdateSmoothScroll();
        if (knobDisabledByF9)
        {
            knobMovementEnabled = IsKnobToggleActive();
        }

        // Update controller positions
        static POINT lastControllerKnobPos = knobPos;
        static POINT lastControllerGhostKnobPos = ghostKnobPos;

        POINT prevKnobPos = knobPos;
        POINT prevGhostKnobPos = ghostKnobPos;
        UpdateKnobFromXInput(hwnd);
        UpdateVJoyFromXInput();

        bool controllerKnobMoved = (knobPos.x != prevKnobPos.x || knobPos.y != prevKnobPos.y);
        bool controllerGhostMoved = (ghostKnobPos.x != prevGhostKnobPos.x || ghostKnobPos.y != prevGhostKnobPos.y);

        PollDirectInput();
        PollPedalBinding(hwnd);

