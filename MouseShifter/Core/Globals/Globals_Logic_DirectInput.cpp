// --- Enum callback: pick first valid gamepad ---
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    // Skip vJoy devices
#ifdef UNICODE
    if (wcsstr(pdidInstance->tszProductName, L"vJoy") != nullptr)
#else
    if (strstr(pdidInstance->tszProductName, "vJoy") != nullptr)
#endif
        return DIENUM_CONTINUE;

    if (FAILED(g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pJoystick, NULL)))
        return DIENUM_CONTINUE;

    return DIENUM_STOP; // use first valid joystick/gamepad
}

// --- Initialize DirectInput ---
bool InitDirectInput(HWND hwnd)
{
    if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&g_pDI, NULL)))
        return false;

    g_pJoystick = nullptr;
    g_hasPedals = false;

    g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, hwnd, DIEDFL_ATTACHEDONLY);

    return g_pJoystick != nullptr;
}

// --- Setup the joystick/gamepad ---
bool SetupJoystick(HWND hwnd)
{
    if (!g_pJoystick)
        return false;

    g_pJoystick->SetDataFormat(&c_dfDIJoystick2);
    g_pJoystick->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    g_hasPedals = true;
    return true;
}

// --- Poll device ---
void PollDirectInput()
{
    if (!g_pJoystick)
        return;

    if (FAILED(g_pJoystick->Poll()))
        g_pJoystick->Acquire();

    g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &g_diState);
}
