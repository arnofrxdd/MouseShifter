    if (showVJoyPicker && vJoyButtonCount > 0)
    {
        int panelWidth = 300;  // wider window
        int panelPadding = 20; // padding around buttons
        int buttonHeight = 28; // button height
        int buttonSpacing = 8; // space between buttons
        int titleHeight = 35;

        int panelHeight = titleHeight + panelPadding + vJoyButtonCount * (buttonHeight + buttonSpacing) + panelPadding;

        RECT parentRect;
        GetClientRect(hwnd, &parentRect);

        int left = max(0, (parentRect.right - panelWidth) / 2);
        int top = max(0, (parentRect.bottom - panelHeight) / 2);

        // Update global picker RECT
        g_vJoyPickerRect = { left, top, left + panelWidth, top + panelHeight };

        // GDI+ Rect for drawing
        Gdiplus::Rect panelRectGDI(left, top, panelWidth, panelHeight);

        // --- Background ---
        Gdiplus::SolidBrush bgBrush(Gdiplus::Color(40, 40, 40)); // dark gray
        graphics.FillRectangle(&bgBrush, panelRectGDI);

        // --- Title ---
        Gdiplus::FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font titleFont(&fontFamily, 18, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush titleBrush(Gdiplus::Color(0, 200, 136)); // accent color
        Gdiplus::RectF titleRectF((Gdiplus::REAL)left, (Gdiplus::REAL)top + 5, (Gdiplus::REAL)panelWidth, (Gdiplus::REAL)titleHeight);

        Gdiplus::StringFormat titleFormat;
        titleFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        titleFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        graphics.DrawString(L"Select vJoy Button", -1, &titleFont, titleRectF, &titleFormat, &titleBrush);

        // --- Buttons ---
        g_vJoyButtonRects.clear();
        Gdiplus::Font btnFont(&fontFamily, 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush btnTextBrush(Gdiplus::Color(220, 220, 220)); // light text

        int btnX = left + panelPadding;
        int btnW = panelWidth - 2 * panelPadding;
        int btnYStart = top + titleHeight + panelPadding;

        for (int i = 0; i < vJoyButtonCount; ++i)
        {
            int btnY = btnYStart + i * (buttonHeight + buttonSpacing);

            RECT btnRectWin = { btnX, btnY, btnX + btnW, btnY + buttonHeight };
            g_vJoyButtonRects.push_back(btnRectWin);

            Gdiplus::Rect btnRectGDI(btnX, btnY, btnW, buttonHeight);
            Gdiplus::SolidBrush btnBrush(Gdiplus::Color(60, 60, 60));
            graphics.FillRectangle(&btnBrush, btnRectGDI);

            // --- Centered button text ---
            wchar_t buf[32];
            swprintf_s(buf, L"vJoy Btn %d", i + 1);
            Gdiplus::RectF btnTextRectF((Gdiplus::REAL)btnRectGDI.X, (Gdiplus::REAL)btnRectGDI.Y,
                (Gdiplus::REAL)btnRectGDI.Width, (Gdiplus::REAL)btnRectGDI.Height);
            Gdiplus::StringFormat btnFormat;
            btnFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            btnFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

            graphics.DrawString(buf, -1, &btnFont, btnTextRectF, &btnFormat, &btnTextBrush);
        }
    }
    // --- Floating Update Button ---
// --- Floating Update Button Stub ---
    if (updateAvailable && !isBorderless)
    {
        // Position in middle-bottom (for potential click detection)
        int buttonWidth = 220;   // slightly wider
        int buttonHeight = 50;   // slightly taller
        int margin = 20;

        int centerX = width / 2;
        int buttonX = centerX - (buttonWidth / 2);
        int buttonY = height - buttonHeight - margin;

        updateButtonRect = {
            buttonX,
            buttonY,
            buttonX + buttonWidth,
            buttonY + buttonHeight
        };

        // Draw a simple text message with bigger, subtle yellow color
        FontFamily fontFamily(L"Segoe UI");
        Font font(&fontFamily, 16, FontStyleRegular, UnitPixel); // bigger font
        SolidBrush brush(Color(255, 255, 230, 150)); // subtle yellow

        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        RectF rectF((REAL)updateButtonRect.left, (REAL)updateButtonRect.top, (REAL)buttonWidth, (REAL)buttonHeight);
        graphics.DrawString(L"Update available, click here", -1, &font, rectF, &format, &brush);
    }

    if (!isBorderless)
    {
        int titleHeight = 24; // thinner title bar
        int buttonSize = 20;
        int buttonMargin = 6;

        // Rounded Close button (red)
        Gdiplus::RectF closeRectF(width - buttonMargin - buttonSize, (titleHeight - buttonSize) / 2, buttonSize, buttonSize);
        Gdiplus::SolidBrush closeBrush(Gdiplus::Color(220, 235, 70, 70));
        graphics.FillEllipse(&closeBrush, closeRectF);
        g_CloseButtonRect = { (int)closeRectF.X, (int)closeRectF.Y, (int)(closeRectF.X + closeRectF.Width), (int)(closeRectF.Y + closeRectF.Height) };

        // Rounded Maximize button (greenish)
        Gdiplus::RectF maxRectF(width - buttonMargin * 2 - buttonSize * 2, (titleHeight - buttonSize) / 2, buttonSize, buttonSize);
        Gdiplus::SolidBrush maxBrush(Gdiplus::Color(220, 70, 220, 70));
        graphics.FillEllipse(&maxBrush, maxRectF);
        g_MaxButtonRect = { (int)maxRectF.X, (int)maxRectF.Y, (int)(maxRectF.X + maxRectF.Width), (int)(maxRectF.Y + maxRectF.Height) };

        // Window title text
        Gdiplus::FontFamily fontFamily(L"Segoe UI");
        Gdiplus::Font titleFont(&fontFamily, 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        Gdiplus::SolidBrush textBrush(Gdiplus::Color(220, 200, 200, 200));
        Gdiplus::RectF textRect(0, 0, (REAL)width, (REAL)titleHeight); // full width
        Gdiplus::StringFormat sf;
        sf.SetAlignment(Gdiplus::StringAlignmentCenter);     // center horizontally
        sf.SetLineAlignment(Gdiplus::StringAlignmentCenter); // center vertically
        graphics.DrawString(L"MouseShifter", -1, &titleFont, textRect, &sf, &textBrush);
    }

    // --- Draw overlay if not borderless ---
    if (!isBorderless)
        DrawHotkeyOverlay(graphics);
    // --- Blit the offscreen bitmap to the window DC ---
