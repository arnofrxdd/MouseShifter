    if (showSettingsPanel)
    {
        using namespace Gdiplus;

        Graphics graphics(memDC);

        // --- Clip drawing to panel ---
        graphics.SetClip(Rect(
            settingsPanelRect.left,
            settingsPanelRect.top,
            settingsPanelRect.right - settingsPanelRect.left,
            settingsPanelRect.bottom - settingsPanelRect.top));

        // --- Smooth scroll toward target ---
        settingsScrollOffsetF += (settingsScrollTarget - settingsScrollOffsetF) * settingsScrollSpeed;
        settingsScrollOffset = (int)settingsScrollOffsetF;

        // --- Fonts & brushes ---
        FontFamily fontFamily(L"Segoe UI");
        Font titleFont(&fontFamily, 22, FontStyleBold, UnitPixel);
        Font rowFont(&fontFamily, 16, FontStyleRegular, UnitPixel);
        SolidBrush titleBrush(Color(0, 255, 136));
        SolidBrush labelBrush(Color(220, 220, 220));
        SolidBrush valueBrush(Color(200, 200, 200));

        int verticalSpacing = 88;    // spacing between sliders/toggles
        int nameControlSpacing = 32; // spacing between name and slider/control
        int startY = 90;
        int sliderHeight = 20;
        int toggleHeight = 30;
        int comboHeight = 28;

        // --- Panel Title ---
        RectF titleRectF(
            (REAL)settingsPanelRect.left,
            (REAL)(settingsPanelRect.top + 10 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left),
            30.0f);
        StringFormat titleFormat;
        titleFormat.SetAlignment(StringAlignmentCenter);
        titleFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(L"Settings", -1, &titleFont, titleRectF, &titleFormat, &titleBrush);

        // --- Slider Helper ---
        auto DrawSlider = [&](RECT rect, float t, const char* label, const char* valueText)
        {
            RectF sliderRectF((REAL)rect.left, (REAL)rect.top + settingsScrollOffset, (REAL)(rect.right - rect.left), (REAL)(rect.bottom - rect.top));

            // Slider background
            SolidBrush bgBrush(Color(50, 50, 50));
            graphics.FillRectangle(&bgBrush, sliderRectF);

            // Slider knob
            float knobX = rect.left + t * (rect.right - rect.left);
            RectF knobRect(knobX - 5, (REAL)rect.top + settingsScrollOffset, 10.0f, (REAL)(rect.bottom - rect.top));
            SolidBrush knobBrush(Color(0, 255, 136));
            graphics.FillRectangle(&knobBrush, knobRect);

            // Label
            PointF labelPos((REAL)rect.left, (REAL)(rect.top - nameControlSpacing + settingsScrollOffset));
            graphics.DrawString(std::wstring(label, label + strlen(label)).c_str(), -1, &rowFont, labelPos, &labelBrush);

            // Value
            StringFormat valueFormat;
            valueFormat.SetAlignment(StringAlignmentCenter);
            PointF valuePos((REAL)(rect.left + (rect.right - rect.left) / 2), (REAL)(rect.bottom + 2 + settingsScrollOffset));
            graphics.DrawString(std::wstring(valueText, valueText + strlen(valueText)).c_str(), -1, &rowFont, valuePos, &valueFormat, &valueBrush);
        };

