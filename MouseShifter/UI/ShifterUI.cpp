void DrawShifterGDIPlus(HWND hwnd, HDC hdc)
{
    using namespace Gdiplus;
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

    // --- Modern Style Tokens (Shared across all handlers) ---
    static Gdiplus::FontFamily fontFamily(L"Segoe UI");
    static Gdiplus::Font titleFont(&fontFamily, 24, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    static Gdiplus::Font rowFont(&fontFamily, 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    static Gdiplus::Font headingFont(&fontFamily, 16, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    static Gdiplus::SolidBrush titleBrush(Gdiplus::Color(0, 255, 170));
    static Gdiplus::SolidBrush labelBrush(Gdiplus::Color(240, 240, 240));
    static Gdiplus::SolidBrush valueBrush(Gdiplus::Color(180, 180, 180));
    static Gdiplus::SolidBrush accentBrush(Gdiplus::Color(0, 255, 170));
    static Gdiplus::SolidBrush bgBrush(Gdiplus::Color(45, 45, 45));
    static Gdiplus::SolidBrush darkBrush(Gdiplus::Color(25, 25, 25));
    static Gdiplus::SolidBrush highlightBrush(Gdiplus::Color(40, 255, 255, 255));
    static Gdiplus::Pen accentPen(Gdiplus::Color(0, 255, 170), 2);
    static Gdiplus::Pen linePen(Gdiplus::Color(65, 65, 65), 1.0f);
    static Gdiplus::Pen boxPen(Gdiplus::Color(0, 255, 170), 2);

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
    // Draw Navigation Dock & Dropdowns (Background for modals)
#include "Components/ShifterUI_TopNavBar.cpp"
#include "Handlers/ShifterUI_SidePanels.cpp"
#include "Handlers/ShifterUI_Settings.cpp"
#include "Handlers/ShifterUI_Overlays.cpp"
#include "Components/ShifterUI_TopDropdowns.cpp"
    
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // --- Cleanup ---
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
