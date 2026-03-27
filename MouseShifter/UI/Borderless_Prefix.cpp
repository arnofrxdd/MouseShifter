void DrawBorderless(HDC hdc, int width, int height)
{
    // We're drawing directly on the provided HDC which already has the background
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    // --- Draw static elements (rails and gears) ---
    DrawStaticShifterElements(graphics);
    DrawStaticGearElements(graphics);

