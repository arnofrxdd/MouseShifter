void InitRawInput(HWND hwnd)
{
    RAWINPUTDEVICE rid[2];

    // Mouse
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;            // Mouse
    rid[0].dwFlags = RIDEV_INPUTSINK; // receive input even when unfocused
    rid[0].hwndTarget = hwnd;

    // Keyboard
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;            // Keyboard
    rid[1].dwFlags = RIDEV_INPUTSINK; // receive input even when unfocused
    rid[1].hwndTarget = hwnd;

    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
    rawInputInitialized = true;

    g_mouseDevices.clear();
    UINT nDevices = 0;
    GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST));
    if (nDevices == 0)
        return;

    std::vector<RAWINPUTDEVICELIST> devs(nDevices);
    GetRawInputDeviceList(devs.data(), &nDevices, sizeof(RAWINPUTDEVICELIST));

    int usbMouseCounter = 1;

    for (auto& d : devs)
    {
        if (d.dwType != RIM_TYPEMOUSE)
            continue;

        UINT size = 0;
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, NULL, &size);
        std::wstring rawName(size, 0);
        GetRawInputDeviceInfo(d.hDevice, RIDI_DEVICENAME, &rawName[0], &size);
        rawName.resize(size - 1);

        // --- Determine if this is a touchpad ---
        bool isTouchpad = (d.hDevice == 0 || rawName.find(L"SYNA") != std::wstring::npos || rawName.find(L"TouchPad") != std::wstring::npos);

        // --- Friendly name formatting ---
        std::wstring friendlyName;
        if (isTouchpad)
        {
            friendlyName = L"Touchpad";
        }
        else
        {
            // Give USB mice sequential numbers
            friendlyName = L"USB Mouse " + std::to_wstring(usbMouseCounter++);
        }

        g_mouseDevices.push_back({ d.hDevice, friendlyName, isTouchpad });

        // --- Debug print ---
        wchar_t buf[256];
        swprintf_s(buf, L"Mouse device handle: %p, formatted name: %s, raw name: %s\n",
            d.hDevice, friendlyName.c_str(), rawName.c_str());
        OutputDebugStringW(buf);
    }
}