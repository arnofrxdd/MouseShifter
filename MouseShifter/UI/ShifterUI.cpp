void DrawShifterGDIPlus(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right;
    int height = rc.bottom;
    int clientWidth = rc.right - rc.left;
    int clientHeight = rc.bottom - rc.top;
    int panelWidth = 300; // width of the keybind panel
    int panelMargin = 10; // distance from right edge
    panelRect = Gdiplus::Rect(
        clientWidth - panelWidth - panelMargin, // X = right-anchored
        panelMargin + 30,                       // leave room for title bar
        panelWidth,                             // Width
        clientHeight - panelMargin * 2          // Height
    );
    settingsPanelRect.left = panelMargin;
    settingsPanelRect.top = panelMargin;
    settingsPanelRect.right = panelMargin + panelWidth;
    settingsPanelRect.bottom = clientHeight - panelMargin;
    // --- Double buffering: create memory HDC and bitmap ---
    HDC memDC = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Initialize GDI+ graphics on memory DC
    // --- Initialize GDI+ graphics on memory DC
    Gdiplus::Graphics graphics(memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // Make the window semi-transparent and blurred
// Windowed mode background
    bool blurSucceeded = EnableWin11Blur(hwnd); // returns true/false
    bool canBlur = EnableWin11Blur(hwnd) && IsTransparencyEnabled();

    if (canBlur)
    {
        graphics.Clear(Gdiplus::Color(150, 8, 8, 8)); // semi-transparent
    }
    else
    {
        graphics.Clear(Gdiplus::Color(255, 8, 8, 8)); // fully opaque fallback
    }

#include "Handlers/ShifterUI_Drawing.cpp"
#include "Handlers/ShifterUI_SidePanels.cpp"
#include "Handlers/ShifterUI_Settings.cpp"
#include "Handlers/ShifterUI_Overlays.cpp"
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // --- Cleanup ---
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
