    if (showVJoyPicker && vJoyButtonCount > 0)
    {
        int columns = 4;
        int rows = (vJoyButtonCount + columns - 1) / columns;
        
        int itemW = 85; 
        int itemH = 40;
        int gap = 10;
        int padding = 20;
        int titleHeight = 55;

        int panelWidth = columns * itemW + (columns - 1) * gap + 2 * padding;
        int panelHeight = titleHeight + rows * itemH + (rows - 1) * gap + padding;

        RECT parentRect;
        GetClientRect(hwnd, &parentRect);
        int left = (parentRect.right - panelWidth) / 2;
        int top = (parentRect.bottom - panelHeight) / 2;

        g_vJoyPickerRect = { left, top, left + panelWidth, top + panelHeight };
        RectF panelRectF((REAL)left, (REAL)top, (REAL)panelWidth, (REAL)panelHeight);

        // --- Dimmed Background ---
        SolidBrush dimBrush(Color(200, 0, 0, 0));
        graphics.FillRectangle(&dimBrush, 0, 0, parentRect.right, parentRect.bottom);

        // --- Picker Card ---
        GraphicsPath cardPath;
        float cr = 12.0f;
        cardPath.AddArc(panelRectF.X, panelRectF.Y, cr * 2, cr * 2, 180, 90);
        cardPath.AddArc(panelRectF.X + panelRectF.Width - cr * 2, panelRectF.Y, cr * 2, cr * 2, 270, 90);
        cardPath.AddArc(panelRectF.X + panelRectF.Width - cr * 2, panelRectF.Y + panelRectF.Height - cr * 2, cr * 2, cr * 2, 0, 90);
        cardPath.AddArc(panelRectF.X, panelRectF.Y + panelRectF.Height - cr * 2, cr * 2, cr * 2, 90, 90);
        cardPath.CloseFigure();

        SolidBrush cardBg(Color(255, 30, 30, 30));
        graphics.FillPath(&cardBg, &cardPath);
        graphics.DrawPath(&accentPen, &cardPath);

        // --- Title ---
        RectF titleRect(panelRectF.X, panelRectF.Y + 12, panelRectF.Width, 30);
        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(L"Select vJoy Button", -1, &headingFont, titleRect, &centerFormat, &accentBrush);

        // Separator
        graphics.DrawLine(&accentPen, panelRectF.X + 30, panelRectF.Y + 45, panelRectF.X + panelRectF.Width - 30, panelRectF.Y + 45);

        // --- Grid Buttons ---
        g_vJoyButtonRects.clear();
        POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
        
        Font btnFont(L"Segoe UI", 13, FontStyleBold, UnitPixel);

        for (int i = 0; i < vJoyButtonCount; ++i)
        {
            int col = i % columns;
            int row = i / columns;
            
            float bx = panelRectF.X + padding + col * (itemW + gap);
            float by = panelRectF.Y + titleHeight + row * (itemH + gap);
            
            RectF btnRectF(bx, by, (REAL)itemW, (REAL)itemH);
            RECT btnRectWin = { (int)bx, (int)by, (int)(bx + itemW), (int)(by + itemH) };
            g_vJoyButtonRects.push_back(btnRectWin);

            bool hovered = PtInRect(&btnRectWin, cursor);
            
            GraphicsPath bp;
            float br = 6.0f;
            bp.AddArc(btnRectF.X, btnRectF.Y, br * 2, br * 2, 180, 90);
            bp.AddArc(btnRectF.X + btnRectF.Width - br * 2, btnRectF.Y, br * 2, br * 2, 270, 90);
            bp.AddArc(btnRectF.X + btnRectF.Width - br * 2, btnRectF.Y + btnRectF.Height - br * 2, br * 2, br * 2, 0, 90);
            bp.AddArc(btnRectF.X, btnRectF.Y + btnRectF.Height - br * 2, br * 2, br * 2, 90, 90);
            bp.CloseFigure();

            SolidBrush btnBrush(hovered ? Color(55, 55, 55) : Color(42, 42, 42));
            graphics.FillPath(&btnBrush, &bp);
            
            Pen btnPen(hovered ? Color(0, 255, 170) : Color(65, 65, 65), hovered ? 1.5f : 1.0f);
            graphics.DrawPath(&btnPen, &bp);

            std::wstring numStr = std::to_wstring(i + 1);
            graphics.DrawString(numStr.c_str(), -1, &btnFont, btnRectF, &centerFormat, hovered ? &accentBrush : &valueBrush);
        }
    }
    if (showUpdateModal)
    {
        // --- Dimmed Background ---
        SolidBrush dimBrush(Color(200, 0, 0, 0));
        graphics.FillRectangle(&dimBrush, 0, 0, width, height);

        int modalWidth = 420;
        int modalHeight = 200;
        int modalX = (width - modalWidth) / 2;
        int modalY = (height - modalHeight) / 2;

        RectF modalRectF((REAL)modalX, (REAL)modalY, (REAL)modalWidth, (REAL)modalHeight);
        
        GraphicsPath modalPath;
        float mr = 12.0f;
        modalPath.AddArc(modalRectF.X, modalRectF.Y, mr * 2, mr * 2, 180, 90);
        modalPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y, mr * 2, mr * 2, 270, 90);
        modalPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        modalPath.AddArc(modalRectF.X, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        modalPath.CloseFigure();

        SolidBrush modalBg(Color(255, 30, 30, 30));
        graphics.FillPath(&modalBg, &modalPath);
        graphics.DrawPath(&accentPen, &modalPath);

        StringFormat centerAlign;
        centerAlign.SetAlignment(StringAlignmentCenter);
        centerAlign.SetLineAlignment(StringAlignmentCenter);

        RectF titleRect(modalRectF.X, modalRectF.Y + 30, modalRectF.Width, 30);
        std::wstring updateTitle = L"New Update Available: v" + std::wstring(latestVersion.begin(), latestVersion.end());
        graphics.DrawString(updateTitle.c_str(), -1, &headingFont, titleRect, &centerAlign, &accentBrush);

        RectF msgRect(modalRectF.X + 20, modalRectF.Y + 70, modalRectF.Width - 40, 40);
        SolidBrush msgBrush(Color(200, 200, 200));
        graphics.DrawString(L"A new version is available. Would you like to update MouseShifter now?", -1, &rowFont, msgRect, &centerAlign, &msgBrush);

        // Buttons
        float btnW = 120;
        float btnH = 40;
        float btnY = modalY + 130;
        float btnGap = 40;

        auto DrawUpdateBtn = [&](float x, float y, float w, float h, const wchar_t* text, bool hovered, bool isPrimary) {
            RectF r(x, y, w, h);
            GraphicsPath bp;
            float br_ = 8.0f;
            bp.AddArc(r.X, r.Y, br_ * 2, br_ * 2, 180, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y, br_ * 2, br_ * 2, 270, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 0, 90);
            bp.AddArc(r.X, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 90, 90);
            bp.CloseFigure();
            
            if (isPrimary) {
                graphics.FillPath(hovered ? &highlightBrush : &accentBrush, &bp);
            } else {
                graphics.FillPath(hovered ? &highlightBrush : &darkBrush, &bp);
            }
            graphics.DrawPath(&accentPen, &bp);
            graphics.DrawString(text, -1, &rowFont, r, &centerAlign, (isPrimary && !hovered) ? &darkBrush : &valueBrush);
            return RECT{(int)x, (int)y, (int)(x+w), (int)(y+h)};
        };

        POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
        g_modalActionRect = DrawUpdateBtn(modalX + (modalWidth - (btnW*2 + btnGap))/2, btnY, btnW, btnH, L"Yes, Update", PtInRect(&g_modalActionRect, cursor), true);
        g_modalCancelRect = DrawUpdateBtn(modalX + (modalWidth - (btnW*2 + btnGap))/2 + btnW + btnGap, btnY, btnW, btnH, L"Not Now", PtInRect(&g_modalCancelRect, cursor), false);
    }

    if (creatingNewProfile)
    {
        // --- Dimmed Background ---
        Gdiplus::SolidBrush dimBrush(Gdiplus::Color(180, 0, 0, 0)); 
        graphics.FillRectangle(&dimBrush, 0, 0, width, height);

        int modalWidth = 400;
        int modalHeight = 220;
        int modalX = (width - modalWidth) / 2;
        int modalY = (height - modalHeight) / 2;

        RectF modalRect((REAL)modalX, (REAL)modalY, (REAL)modalWidth, (REAL)modalHeight);
        
        // --- Modal Card ---
        GraphicsPath modalPath;
        float mr = 12.0f;
        modalPath.AddArc(modalRect.X, modalRect.Y, mr * 2, mr * 2, 180, 90);
        modalPath.AddArc(modalRect.X + modalRect.Width - mr * 2, modalRect.Y, mr * 2, mr * 2, 270, 90);
        modalPath.AddArc(modalRect.X + modalRect.Width - mr * 2, modalRect.Y + modalRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        modalPath.AddArc(modalRect.X, modalRect.Y + modalRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        modalPath.CloseFigure();

        SolidBrush modalBg(Color(255, 35, 35, 35));
        graphics.FillPath(&modalBg, &modalPath);
        graphics.DrawPath(&accentPen, &modalPath);

        // --- Title ---
        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);
        
        RectF titleRect(modalRect.X, modalRect.Y + 20, modalRect.Width, 30);
        graphics.DrawString(L"Create New Profile", -1, &headingFont, titleRect, &centerFormat, &accentBrush);

        // --- Input Field ---
        float inputW = modalWidth - 60;
        float inputH = 40;
        float inputX = modalX + 30;
        float inputY = modalY + 70;
        RectF inputRect(inputX, inputY, inputW, inputH);

        GraphicsPath inputPath;
        float ir = 6.0f;
        inputPath.AddArc(inputRect.X, inputRect.Y, ir * 2, ir * 2, 180, 90);
        inputPath.AddArc(inputRect.X + inputRect.Width - ir * 2, inputRect.Y, ir * 2, ir * 2, 270, 90);
        inputPath.AddArc(inputRect.X + inputRect.Width - ir * 2, inputRect.Y + inputRect.Height - ir * 2, ir * 2, ir * 2, 0, 90);
        inputPath.AddArc(inputRect.X, inputRect.Y + inputRect.Height - ir * 2, ir * 2, ir * 2, 90, 90);
        inputPath.CloseFigure();

        graphics.FillPath(&darkBrush, &inputPath);
        graphics.DrawPath(&accentPen, &inputPath);

        std::wstring nameW(newProfileName.begin(), newProfileName.end());
        RectF textRect = inputRect; textRect.X += 12; textRect.Width -= 24;
        graphics.DrawString(nameW.c_str(), -1, &rowFont, textRect, &centerFormat, &valueBrush);

        // --- Cursor ---
        static DWORD lastBlink = GetTickCount();
        static bool cursorVisible = true;
        if (GetTickCount() - lastBlink > 500) { cursorVisible = !cursorVisible; lastBlink = GetTickCount(); }

        if (cursorVisible) {
            CharacterRange range(0, (int)profileTextSelectionStart);
            StringFormat cursorFormat;
            cursorFormat.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces);
            cursorFormat.SetLineAlignment(StringAlignmentCenter);
            cursorFormat.SetAlignment(StringAlignmentCenter);
            cursorFormat.SetMeasurableCharacterRanges(1, &range);
            
            Region regions[1];
            graphics.MeasureCharacterRanges(nameW.c_str(), -1, &rowFont, textRect, &cursorFormat, 1, regions);
            RectF bounds;
            if (regions[0].GetBounds(&bounds, &graphics) == Ok) {
                graphics.FillRectangle(&accentBrush, bounds.GetRight(), bounds.Y + 8, 2.0f, bounds.Height - 16);
            } else {
                // If empty or measure failed, draw in center
                graphics.FillRectangle(&accentBrush, inputRect.X + inputRect.Width/2, inputRect.Y + 10, 2.0f, inputRect.Height - 20);
            }
        }

        // --- Buttons ---
        float btnW = (modalWidth - 80) / 2;
        float btnH = 35;
        float btnY = modalY + 140;

        auto DrawModalBtn = [&](float x, float y, float w, float h, const wchar_t* text, bool hovered, bool isPrimary) {
            RectF r(x, y, w, h);
            GraphicsPath bp;
            float br_ = 6.0f;
            bp.AddArc(r.X, r.Y, br_ * 2, br_ * 2, 180, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y, br_ * 2, br_ * 2, 270, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 0, 90);
            bp.AddArc(r.X, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 90, 90);
            bp.CloseFigure();
            
            if (isPrimary) {
                graphics.FillPath(hovered ? &highlightBrush : &accentBrush, &bp);
            } else {
                graphics.FillPath(hovered ? &highlightBrush : &darkBrush, &bp);
            }
            graphics.DrawPath(&accentPen, &bp);
            graphics.DrawString(text, -1, &rowFont, r, &centerFormat, (isPrimary && !hovered) ? &darkBrush : &valueBrush);
            
            return RECT{(int)x, (int)y, (int)(x+w), (int)(y+h)};
        };

        POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
        
        g_modalActionRect = DrawModalBtn(modalX + 30, btnY, btnW, btnH, L"Create", PtInRect(&g_modalActionRect, cursor), true);
        g_modalCancelRect = DrawModalBtn(modalX + 50 + btnW, btnY, btnW, btnH, L"Cancel", PtInRect(&g_modalCancelRect, cursor), false);
    }

    if (showResetConfirmation)
    {
        // --- Dimmed Background ---
        SolidBrush dimBrush(Color(200, 0, 0, 0));
        graphics.FillRectangle(&dimBrush, 0, 0, width, height);

        int modalWidth = 380;
        int modalHeight = 180;
        int modalX = (width - modalWidth) / 2;
        int modalY = (height - modalHeight) / 2;

        RectF modalRectF((REAL)modalX, (REAL)modalY, (REAL)modalWidth, (REAL)modalHeight);
        
        GraphicsPath modalPath;
        float mr = 12.0f;
        modalPath.AddArc(modalRectF.X, modalRectF.Y, mr * 2, mr * 2, 180, 90);
        modalPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y, mr * 2, mr * 2, 270, 90);
        modalPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        modalPath.AddArc(modalRectF.X, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        modalPath.CloseFigure();

        SolidBrush modalBg(Color(255, 35, 35, 35));
        graphics.FillPath(&modalBg, &modalPath);
        graphics.DrawPath(&accentPen, &modalPath);

        StringFormat centerAlign;
        centerAlign.SetAlignment(StringAlignmentCenter);
        centerAlign.SetLineAlignment(StringAlignmentCenter);

        RectF titleRect(modalRectF.X, modalRectF.Y + 25, modalRectF.Width, 30);
        graphics.DrawString(L"Reset All Keybindings?", -1, &headingFont, titleRect, &centerAlign, &accentBrush);

        RectF msgRect(modalRectF.X + 20, modalRectF.Y + 60, modalRectF.Width - 40, 40);
        SolidBrush subtitleBtnBrush(Color(180, 180, 180));
        graphics.DrawString(L"This will revert all gears to their factory default buttons.", -1, &rowFont, msgRect, &centerAlign, &subtitleBtnBrush);

        // Buttons
        float btnW = (modalWidth - 80) / 2;
        float btnH = 36;
        float btnY = modalY + 120;

        auto DrawResetModalBtn = [&](float x, float y, float w, float h, const wchar_t* text, bool hovered, bool isPrimary) {
            RectF r(x, y, w, h);
            GraphicsPath bp;
            float br_ = 6.0f;
            bp.AddArc(r.X, r.Y, br_ * 2, br_ * 2, 180, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y, br_ * 2, br_ * 2, 270, 90);
            bp.AddArc(r.X + r.Width - br_ * 2, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 0, 90);
            bp.AddArc(r.X, r.Y + r.Height - br_ * 2, br_ * 2, br_ * 2, 90, 90);
            bp.CloseFigure();
            
            if (isPrimary) {
                SolidBrush primaryBrush(Color(255, 180, 50, 50)); // Reddish for Reset
                graphics.FillPath(hovered ? &highlightBrush : &primaryBrush, &bp);
            } else {
                graphics.FillPath(hovered ? &highlightBrush : &darkBrush, &bp);
            }
            graphics.DrawPath(&accentPen, &bp);
            graphics.DrawString(text, -1, &rowFont, r, &centerAlign, &valueBrush);
            return RECT{(int)x, (int)y, (int)(x+w), (int)(y+h)};
        };

        POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
        g_modalActionRect = DrawResetModalBtn(modalX + 30, btnY, btnW, btnH, L"Yes, Reset", PtInRect(&g_modalActionRect, cursor), true);
        g_modalCancelRect = DrawResetModalBtn(modalX + 50 + btnW, btnY, btnW, btnH, L"Cancel", PtInRect(&g_modalCancelRect, cursor), false);
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
        
    // --- Visual Shifter Layout Gallery (Absolute Highest Priority) ---
    #include "UI/Overlays/ShifterUI_LayoutGallery.cpp"

    // --- Blit the offscreen bitmap to the window DC ---
