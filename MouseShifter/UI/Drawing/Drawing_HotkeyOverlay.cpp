    // --- Hotkey overlay ---
    // --- Hotkey overlay ---

    auto DrawHotkeyOverlay = [&](Graphics& g)
    {
        int padding = 40;
        int offsetX = 270;
        int boxWidth = 280;
        int boxHeight = 110; // increased to fit version line

        Rect overlayRect(padding + offsetX, padding, boxWidth, boxHeight);

        // Text
        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 14, FontStyleRegular, UnitPixel);
        SolidBrush textBrush(Color(150, 255, 255, 255)); // ARGB: 150 alpha, white

        PointF textPos(float(overlayRect.X + 10), float(overlayRect.Y + 10));
        g.DrawString(L"F12 - Enable/Disable Knob", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"F11 - Temporarily Disable Mouse Steering", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"F10 - Switch 12/16 Gearbox", -1, &font, textPos, &textBrush);

        textPos.Y += 18;
        g.DrawString(L"Tab - Enable/Disable Overlay", -1, &font, textPos, &textBrush);

        // Add version info
        textPos.Y += 18;
        std::wstring versionText = L"Version: " + std::wstring(currentVersion.begin(), currentVersion.end());
        g.DrawString(versionText.c_str(), -1, &font, textPos, &textBrush);
    };

