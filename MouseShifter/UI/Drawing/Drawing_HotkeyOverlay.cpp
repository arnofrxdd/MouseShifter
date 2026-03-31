    // --- Hotkey overlay ---
    // --- Hotkey overlay ---

    auto DrawHotkeyOverlay = [&](Graphics& g)
    {
        int padding = 40;
        int offsetX = 270;
        int boxWidth = 280;
        int boxHeight = updateAvailable ? 135 : 110; 

        Rect overlayRect(padding + offsetX, padding, boxWidth, boxHeight);

        // Text
        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 14, FontStyleRegular, UnitPixel);
        Font subFont(&fontFamily, 13, FontStyleBold, UnitPixel);
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

        // --- Interactive Update Button ---
        if (updateAvailable && !showUpdateModal)
        {
            textPos.Y += 22;
            updateSubtleBtnRect = { (int)textPos.X, (int)textPos.Y, (int)textPos.X + 150, (int)textPos.Y + 20 };
            
            POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
            bool hovered = PtInRect(&updateSubtleBtnRect, cursor);

            SolidBrush updateBrush(hovered ? Color(255, 0, 255, 170) : Color(200, 0, 200, 150));
            g.DrawString(L"\x2b07 Click here to update", -1, &subFont, textPos, &updateBrush);
            
            if (hovered) {
                Pen p(Color(100, 0, 255, 170), 1.0f);
                g.DrawLine(&p, textPos.X, textPos.Y + 15, textPos.X + 135, textPos.Y + 15);
            }
        }
    };

