void RefreshMouseDeviceList(HWND hwnd)
{
    g_mouseDevices.clear();
    UINT nDevices = 0;
    if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0) return;
    if (nDevices == 0) return;

    std::vector<RAWINPUTDEVICELIST> devs(nDevices);
    if (GetRawInputDeviceList(devs.data(), &nDevices, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) return;

    int usbMouseCounter = 1;
    for (auto& d : devs)
    {
        if (d.dwType != RIM_TYPEMOUSE) continue;

        UINT size = 0;
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, NULL, &size);
        if (size == 0) continue;
        std::wstring rawName(size, 0);
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, &rawName[0], &size);

        // --- Determine if this is a touchpad ---
        bool isTouchpad = (rawName.find(L"SYNA") != std::wstring::npos || rawName.find(L"TouchPad") != std::wstring::npos);

        // --- Friendly name formatting ---
        std::wstring friendlyName = isTouchpad ? L"Touchpad" : (L"USB Mouse " + std::to_wstring(usbMouseCounter++));
        g_mouseDevices.push_back({ d.hDevice, friendlyName, isTouchpad });
    }
}

void InitRawInput(HWND hwnd)
{
    RAWINPUTDEVICE rid[2];

    // Mouse
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;            // Mouse
    rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY; // receive input even when unfocused + device change notifications
    rid[0].hwndTarget = hwnd;

    // Keyboard
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;            // Keyboard
    rid[1].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY; // receive input even when unfocused
    rid[1].hwndTarget = hwnd;

    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
    rawInputInitialized = true;

    RefreshMouseDeviceList(hwnd);
}