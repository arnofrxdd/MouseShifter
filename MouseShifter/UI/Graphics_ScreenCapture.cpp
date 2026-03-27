float GetHShifterBackgroundBrightnessDebug(HWND hwnd)
{
    // Get the H-Shifter boundaries
    RECT hShifterRect = CalculateHShifterBoundaries();

    // Get the screen DC
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    // Create a bitmap to capture the area behind our H-Shifter
    int captureWidth = hShifterRect.right - hShifterRect.left;
    int captureHeight = hShifterRect.bottom - hShifterRect.top;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, captureWidth, captureHeight);
    SelectObject(hdcMem, hBitmap);

    // Copy the screen content from behind our H-Shifter area
    BitBlt(hdcMem, 0, 0, captureWidth, captureHeight,
        hdcScreen, hShifterRect.left, hShifterRect.top, SRCCOPY);

    // Analyze the bitmap for brightness
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int bytesPerPixel = bmp.bmBitsPixel / 8;
    int imageSize = bmp.bmWidth * bmp.bmHeight * bytesPerPixel;

    BYTE* bits = new BYTE[imageSize];
    GetBitmapBits(hBitmap, imageSize, bits);

    // Calculate average brightness (sample every 5th pixel for performance)
    long long totalBrightness = 0;
    int pixelCount = 0;

    for (int y = 0; y < bmp.bmHeight; y += 5) {
        for (int x = 0; x < bmp.bmWidth; x += 5) {
            int offset = (y * bmp.bmWidth + x) * bytesPerPixel;

            if (offset + 2 < imageSize) {
                BYTE blue = bits[offset];
                BYTE green = bits[offset + 1];
                BYTE red = bits[offset + 2];

                // Calculate pixel brightness (using luminance formula)
                float brightness = 0.299f * red + 0.587f * green + 0.114f * blue;
                totalBrightness += (long long)brightness;
                pixelCount++;
            }
        }
    }

    float avgBrightness = pixelCount > 0 ? (float)totalBrightness / pixelCount : 128.0f;

    // Cleanup
    delete[] bits;
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Debug output

    return avgBrightness;
}
// Add global to track previous position
static POINT lastRedrawKnobPos = knobPos;
static POINT lastRedrawGhostPos = ghostKnobPos;
static DWORD lastGhostKnobMoveTime = 0;
static float ghostKnobMoveDistance = 0.0f;
static int ghostRedrawFPS = 60;
// Add these global tracking variables for clutch knob glow
static float lastClutchKnobNorm = 0.0f;
static bool lastVJoyMouseEnabledState = vJoyMouseEnabled;
