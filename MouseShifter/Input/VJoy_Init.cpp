bool InitVJoy()
{
    if (!vJoyEnabled())
    {
        OutputDebugString(L"[vJoy] Driver not enabled!\n");
        MessageBox(nullptr, L"vJoy is not installed! Downloading now...", L"Error", MB_OK);

        // --- Download vJoy installer ---
        const wchar_t* url = L"https://cyfuture.dl.sourceforge.net/project/vjoystick/Beta%202.x/2.1.9.1-160719/vJoySetup.exe?viasf=1";
        const wchar_t* savePath = L"vJoy_Setup.exe";

        HRESULT hr = URLDownloadToFile(nullptr, url, savePath, 0, nullptr);
        if (SUCCEEDED(hr))
        {
            MessageBox(nullptr, L"vJoy downloaded! Please run the installer manually.", L"Download Complete", MB_OK);
            ShellExecute(nullptr, L"open", savePath, nullptr, nullptr, SW_SHOWNORMAL);
        }
        else
        {
            MessageBox(nullptr, L"Failed to download vJoy! Please download it from the official site.", L"Error", MB_OK | MB_ICONERROR);
        }

        return false;
    }

    VjdStat status = GetVJDStatus(vjoyDeviceId);

    switch (status)
    {
    case VJD_STAT_OWN:
        OutputDebugString(L"[vJoy] Device already owned.\n");
        break;
    case VJD_STAT_FREE:
        OutputDebugString(L"[vJoy] Device is free.\n");
        break;
    case VJD_STAT_BUSY:
        OutputDebugString(L"[vJoy] Device busy.\n");
        break;
    case VJD_STAT_MISS:
        OutputDebugString(L"[vJoy] Device missing.\n");
        break;
    case VJD_STAT_UNKN:
    default:
        OutputDebugString(L"[vJoy] Device status unknown.\n");
        break;
    }

    if (status != VJD_STAT_OWN && status != VJD_STAT_FREE)
    {
        MessageBox(nullptr, L"vJoy device not ready!", L"Error", MB_OK);
        return false;
    }

    // --- RELEASE ALL BUTTONS BEFORE STARTING ---
    vJoyButtonCount = GetVJDButtonNumber(vjoyDeviceId); // get number of buttons
    for (int btn = 1; btn <= vJoyButtonCount; ++btn)
    {
        SetBtn(FALSE, vjoyDeviceId, btn); // Release button
    }
    OutputDebugString(L"[vJoy] All buttons released before starting.\n");

    // Now acquire the device
    BOOL ok = AcquireVJD(vjoyDeviceId);

    // Get axis min/max
    GetVJDAxisMin(vjoyDeviceId, HID_USAGE_X, &axisMin);
    GetVJDAxisMax(vjoyDeviceId, HID_USAGE_X, &axisMax);

    // Center joystick axes
    joyX = joyY = (axisMax + axisMin) / 2;
    joyRx = (axisMax + axisMin) / 2;  // center Rx

    // Push initial positions to vJoy
    SetAxis(joyX, vjoyDeviceId, HID_USAGE_X);
    SetAxis(joyY, vjoyDeviceId, HID_USAGE_Y);
    SetAxis(joyRx, vjoyDeviceId, HID_USAGE_RX);


    if (ok)
    {
        OutputDebugString(L"[vJoy] Device acquired successfully.\n");

        // --- Store button count for later use ---
        char buf[128];
        sprintf_s(buf, "vJoyButtonCount=%d\n", vJoyButtonCount);
        OutputDebugStringA(buf);
    }
    else
    {
        OutputDebugString(L"[vJoy] Failed to acquire device!\n");
    }

    return ok != FALSE;
}