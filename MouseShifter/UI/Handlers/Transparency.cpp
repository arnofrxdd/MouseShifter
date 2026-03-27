#include "Core/Globals/AppGlobals.h"
#include "Transparency.h"

// --- Definitions of transparency-related globals (Moved from AppGlobals.h) ---
bool isTransparent = false;
bool dynamicTransparencyEnabled = true;
BYTE currentAlpha = 200;
BYTE maxAlpha = 100;
BYTE minAlpha = 20;
DWORD transparencyFadeDelay = 300;
bool draggingTransparencyFadeDelaySlider = false;
bool draggingMinTransparencySlider = false;
int yBarAlpha = 180;
bool useYbarFixedTransparency = false;
HWND g_yBarHwnd = nullptr;

RECT transparencyFadeDelaySliderRect;
RECT dynamicTransparencyToggleRect;
RECT transparencySliderRect;
RECT minTransparencySliderRect;
RECT yBarFixedTransToggle;
RECT yBarAlphaSlider;

// --- Implementations (Moved from AppGlobals.h & MouseShifter.cpp) ---

bool EnableWin11Blur(HWND hwnd, bool acrylic)
{
    ACCENT_POLICY policy = {};
    policy.nAccentState = acrylic ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_ENABLE_BLURBEHIND;
    policy.nFlags = 0;
    policy.nColor = 0xCCFFFFFF;
    policy.nAnimationId = 0;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.nAttribute = 19; // WCA_ACCENT_POLICY
    data.pData = &policy;
    data.ulDataSize = sizeof(policy);

    typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
    static pSetWindowCompositionAttribute SetWindowCompositionAttribute =
        (pSetWindowCompositionAttribute)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");

    if (SetWindowCompositionAttribute)
        return SetWindowCompositionAttribute(hwnd, &data);

    return false;
}

bool IsTransparencyEnabled()
{
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(nullptr, &enabled)))
    {
        // This returns TRUE if colorization is active, but better:
    }

    DWORD value = 0;
    DWORD size = sizeof(DWORD);
    if (RegGetValue(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"EnableTransparency",
        RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
    {
        return value != 0;
    }

    return false; // assume disabled if registry query fails
}

void FadeLayeredWindow(HWND hwnd, BYTE startAlpha, BYTE endAlpha, int durationMs)
{
    const int steps = 20; // smoothness
    const int delay = durationMs / steps;

    for (int i = 0; i <= steps; ++i)
    {
        float t = i / (float)steps;
        BYTE alpha = (BYTE)(startAlpha + t * (endAlpha - startAlpha));
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), alpha, LWA_COLORKEY | LWA_ALPHA);
        Sleep(delay); // simple blocking; for non-blocking, use a timer
    }
}

void CreateYBarWindow(HWND parentHwnd)
{
    if (g_yBarHwnd) return;

    WNDCLASS wc = {};
    wc.lpfnWndProc = YBarWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"YBarWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    RECT parentRect;
    GetWindowRect(parentHwnd, &parentRect);

    g_yBarHwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"YBarWindow",
        L"YBar",
        WS_POPUP,
        parentRect.left, parentRect.top,
        parentRect.right - parentRect.left, parentRect.bottom - parentRect.top,
        parentHwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );

    SetLayeredWindowAttributes(g_yBarHwnd, RGB(0, 0, 0), yBarAlpha, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(g_yBarHwnd, SW_SHOW);
}

void DestroyYBarWindow()
{
    if (g_yBarHwnd)
    {
        DestroyWindow(g_yBarHwnd);
        g_yBarHwnd = nullptr;
    }
}

void UpdateYBarWindowPosition(HWND parentHwnd)
{
    if (!g_yBarHwnd) return;

    RECT parentRect;
    GetWindowRect(parentHwnd, &parentRect);
    SetWindowPos(g_yBarHwnd, nullptr,
        parentRect.left, parentRect.top,
        parentRect.right - parentRect.left, parentRect.bottom - parentRect.top,
        SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CALLBACK YBarWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right;
        int height = rc.bottom;

        // Double buffering for Y-bar window
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // Clear to transparent
        Gdiplus::Graphics graphics(memDC);
        graphics.Clear(Gdiplus::Color(0, 0, 0, 0));

        // Draw only Y-bar
        DrawYBarOnly(memDC, width, height);

        // Blit to screen
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent background erasure

    case WM_DESTROY:
        g_yBarHwnd = nullptr;
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void ToggleBorderless(HWND hwnd)
{
    isBorderless = !isBorderless;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (isBorderless)
    {
        // Remove window borders
        style &= ~(WS_OVERLAPPEDWINDOW);
        SetWindowLong(hwnd, GWL_STYLE, style);

        // Make window layered for transparency
        exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // Set topmost only when borderless
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Disable blur/acrylic explicitly
        ACCENT_POLICY policy = {};
        policy.nAccentState = ACCENT_DISABLED;
        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.nAttribute = 19; // WCA_ACCENT_POLICY
        data.pData = &policy;
        data.ulDataSize = sizeof(policy);

        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
        static pSetWindowCompositionAttribute SetWindowCompositionAttribute =
            (pSetWindowCompositionAttribute)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");

        if (SetWindowCompositionAttribute)
            SetWindowCompositionAttribute(hwnd, &data);

        currentAlpha = maxAlpha; // use slider-controlled max alpha
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), currentAlpha, LWA_COLORKEY | LWA_ALPHA);

        isTransparent = true;

        // Create Y-bar window with separate alpha
        if (useYbarFixedTransparency)
        {
            CreateYBarWindow(hwnd);
        }


        // Hide cursor
        ShowCursor(FALSE);

        // Save and hide panels
        prevShowInputPanel = showInputPanel;
        prevShowTogglePanel = showTogglePanel;
        prevShowSettingsPanel = showSettingsPanel;
        prevShowKeybindPanel = showKeybindPanel;
        showSettingsPanel = false;
        showKeybindPanel = false;
        showInputPanel = false;
        showTogglePanel = false;
        // Lock cursor inside window
        ClipCursor(nullptr);
    }
    else
    {
        // Destroy Y-bar window
        if (useYbarFixedTransparency)
        {
            DestroyYBarWindow();
        }

        // Restore standard window style
        style &= ~WS_CAPTION;
        style &= ~WS_THICKFRAME;
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLong(hwnd, GWL_STYLE, style);
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // Remove topmost when exiting borderless
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Show cursor
        ShowCursor(TRUE);
        ClipCursor(nullptr);

        // Restore panel visibility
        showSettingsPanel = prevShowSettingsPanel;
        showKeybindPanel = prevShowKeybindPanel;
        showInputPanel = prevShowInputPanel;
        showTogglePanel = prevShowTogglePanel;
        // Apply blur in windowed mode
        EnableWin11Blur(hwnd);
    }

    // Refresh window to apply style changes
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void DrawYBarOnly(HDC hdc, int width, int height)
{
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // --- Draw vertical Y-axis indicator (throttle/brake) ---
    if (showYBar && mouseSteeringEnabled)
    {
        float normalizedY = (float)(joyY - axisMin) / (float)(axisMax - axisMin);
        normalizedY = max(0.0f, min(1.0f, normalizedY));

        // Get rail count for positioning
        int drawRailCount = 0;
        switch (layoutType)
        {
        case 1: // Normal Layout
        case 3: // Reverse Bottom Layout (First rail)
        case 4: // Reverse Bottom Layout (Last rail)
        case 10: // Reverse Top Last Layout
            drawRailCount = is16GearSet ? 5 : 4;
            break;
        case 2: // No Reverse Layout
            drawRailCount = is16GearSet ? 4 : 3;
            break;
        case 5: // 5-Gear Only Layout
        case 6: // 5-Gear Reverse First Layout
        case 7: // 4-Gear Reverse Top Layout
        case 8: // 4-Gear Reverse Bottom Layout
        case 9: // 4-Gear Reverse Mixed Layout
            drawRailCount = 3; // Always 3 rails for these layouts
            break;
        case 11: // PRNDL Layout
            drawRailCount = 1; // Only one rail for PRNDL
            break;
        }

        int barWidth = 8;
        int barHeight = bottomY - topY;
        int lastRailX = railX[drawRailCount - 1].x;
        int yBarOffset = 20;
        int barY = centerY - barHeight / 2;
        int xBarGap = 20;
        int barX = max(lastRailX + 50, knobPos.x + knobRadius + xBarGap);

        // Compute dynamic color from green → red based on Y
        BYTE r = (BYTE)(normalizedY * 255);  // Increased to 255 for more vibrant colors
        BYTE g = (BYTE)((1.0f - normalizedY) * 255);  // Increased to 255 for more vibrant colors
        Gdiplus::Color dynamicColor(r, g, 50);

        // Background (dynamic) - only change the background color
        Gdiplus::SolidBrush barBg(dynamicColor);
        Gdiplus::Pen barOutline(Gdiplus::Color(200, 200, 200), 2);  // Brighter outline for better contrast
        Gdiplus::Rect barRect(barX, barY, barWidth, barHeight);
        graphics.FillRectangle(&barBg, barRect);
        graphics.DrawRectangle(&barOutline, barRect);

        // Filled portion (darker version of background color for subtle contrast)
        int filledHeight = (int)(barHeight * normalizedY);
        int fillY = barY;
        Gdiplus::SolidBrush barFill(Gdiplus::Color(max(0, r - 80), max(0, g - 80), 30));  // Darker version of dynamic color
        Gdiplus::Rect fillRect(barX, fillY, barWidth, filledHeight);
        graphics.FillRectangle(&barFill, fillRect);

        // Small horizontal line in middle (brighter for better visibility)
        int lineY = barY + barHeight / 2;
        Gdiplus::Pen linePen(Gdiplus::Color(255, 255, 255), 2);
        graphics.DrawLine(&linePen, barX, lineY, barX + barWidth, lineY);
    }
}
