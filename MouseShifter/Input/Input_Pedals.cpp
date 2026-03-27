void PollPedalBinding(HWND hwnd)
{
    if (!togglePedalBeingSet || !g_hasPedals || !g_pJoystick)
        return;

    PollDirectInput();

    TCHAR buf[256];
    //_stprintf_s(buf, _T("Polling Pedals -> Clutch: %ld, Brake: %ld, Accel: %ld\n"),
        //g_diState.lZ, g_diState.lY, g_diState.lRz);
    //OutputDebugString(buf);

    if (g_diState.lZ > 32767) // clutch
    {
        g_knobToggleType = TOGGLE_PEDAL_CLUTCH;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Clutch pedal bound!\n"));
    }
    else if (g_diState.lY > 32767) // brake
    {
        g_knobToggleType = TOGGLE_PEDAL_BRAKE;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Brake pedal bound!\n"));
    }
    else if (g_diState.lRz > 32767) // accelerator
    {
        g_knobToggleType = TOGGLE_PEDAL_ACCEL;
        g_knobToggleKey = 0;
        togglePedalBeingSet = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        OutputDebugString(_T("Accelerator pedal bound!\n"));
    }
}

inline int ApplyKnobMotion(int delta, float sensitivity) {
    if (!knobAccelerationEnabled)
        return int(delta * sensitivity);

    // Base acceleration: small effect from delta
    float baseAccel = 1.0f + fabs(delta) * 0.01f; // reduced from 0.02 to soften

    // Nonlinear sensitivity factor: compress low end, boost high end
    float sensFactor = powf(sensitivity, 0.7f); // tweak exponent for feel

    // Final acceleration
    float accel = baseAccel * sensFactor;

    return int(delta * accel);
}





