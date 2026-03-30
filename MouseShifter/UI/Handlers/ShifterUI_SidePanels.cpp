    if (showKeybindPanel || showInputPanel || showTogglePanel)
    {
        using namespace Gdiplus;

        // --- Clip drawing to panel area ---
        graphics.SetClip(panelRect);

        // Sidebar Background removed at user request

        FontFamily fontFamily(L"Segoe UI");
        Font titleFont(&fontFamily, 22, FontStyleBold, UnitPixel);
        Font subtitleFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
        Font rowFont(&fontFamily, 16, FontStyleRegular, UnitPixel);

        // --- Pulse Animation Factor ---
        float pulse = (sin(GetTickCount() * 0.006f) + 1.0f) / 2.0f; // Smooth pulse 0.0 to 1.0

        SolidBrush titleBrush(Color(0, 255, 170));
        SolidBrush subtitleBrush(Color(180, 180, 180));
        SolidBrush textBrush(Color(220, 220, 220));

        StringFormat centerAlign;
        centerAlign.SetAlignment(StringAlignmentCenter);
        centerAlign.SetLineAlignment(StringAlignmentCenter);
        StringFormat leftAlign;
        leftAlign.SetAlignment(StringAlignmentNear);       // horizontal left alignment
        leftAlign.SetLineAlignment(StringAlignmentCenter); // vertical centering
        // Unified starting position
        int y = panelRect.Y + 10 + rightPanelScrollOffset;
        int rightPanelContentBottom = y;

        const int rowHeight = 36;          // height of each interactive row
        const int rowSpacing = 6;          // vertical space between rows
        const int rowPadding = 6;          // padding inside row for text
        const int leftMargin = 10;         // left offset for text inside row
        const int gearColumnWidth = 60;    // width of gear/label column
        const int keyColumnWidth = 280;    // width of key/button column
        const int panelWidthUnified = 320; // width of input/toggle panels
        int panelWidth = panelRect.Width;

        // --- KEYBIND PANEL ---
        if (showKeybindPanel)
        {
            // Title and Reset All
            RectF titleRectF((REAL)panelRect.X + 15, (REAL)y, (REAL)panelWidth - 120, 30); // smaller width to avoid button
            graphics.DrawString(L"Key Bindings", -1, &titleFont, titleRectF, &leftAlign, &titleBrush);

            // Reset All Button (Top Right of section)
            float resetAllW = 95;   // slightly wider
            float resetAllH = 26;   // slightly taller
            float resetAllX = panelRect.X + panelRect.Width - resetAllW - 20;
            float resetAllY = y + 2;
            RectF resetAllRectF(resetAllX, resetAllY, resetAllW, resetAllH);
            resetAllButtonRect = { (int)resetAllX, (int)resetAllY, (int)(resetAllX + resetAllW), (int)(resetAllY + resetAllH) };

            POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
            bool resetAllHover = PtInRect(&resetAllButtonRect, cursor);

            GraphicsPath resetPath;
            float rr = 6.0f;
            resetPath.AddArc(resetAllRectF.X, resetAllRectF.Y, rr * 2, rr * 2, 180, 90);
            resetPath.AddArc(resetAllRectF.X + resetAllRectF.Width - rr * 2, resetAllRectF.Y, rr * 2, rr * 2, 270, 90);
            resetPath.AddArc(resetAllRectF.X + resetAllRectF.Width - rr * 2, resetAllRectF.Y + resetAllRectF.Height - rr * 2, rr * 2, rr * 2, 0, 90);
            resetPath.AddArc(resetAllRectF.X, resetAllRectF.Y + resetAllRectF.Height - rr * 2, rr * 2, rr * 2, 90, 90);
            resetPath.CloseFigure();

            // Premium Accent/Linear Gradient for Reset All
            Color accentRedStart = resetAllHover ? Color(200, 80, 40, 40) : Color(180, 40, 20, 20);
            Color accentRedEnd = resetAllHover ? Color(200, 50, 10, 10) : Color(180, 20, 5, 5);
            LinearGradientBrush resetBrush(resetAllRectF, accentRedStart, accentRedEnd, LinearGradientModeVertical);
            
            graphics.FillPath(&resetBrush, &resetPath);
            Pen resetPen(resetAllHover ? Color(200, 0, 255, 170) : Color(100, 0, 255, 170), 1.0f); // Teal border for consistency
            graphics.DrawPath(&resetPen, &resetPath);
            
            // Text with icon
            StringFormat resetSF;
            resetSF.SetAlignment(StringAlignmentCenter);
            resetSF.SetLineAlignment(StringAlignmentCenter);
            graphics.DrawString(L"\x21BB Reset All", -1, &subtitleFont, resetAllRectF, &resetSF, &textBrush);

            y += 35; // added 3px more breathing room

            // Subtitle
            RectF subtitleRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, 20);
            graphics.DrawString(L"Shift+LMB to pick vJoy buttons", -1, &subtitleFont, subtitleRectF, &centerAlign, &subtitleBrush);

            y += 30;

            // Clear map before repopulating
            gearResetBtnRects.clear();

            // Sorted gear keys
            std::vector<std::string> sortedKeys;
            for (auto& kv : gearInputMap)
                sortedKeys.push_back(kv.first);

            auto isNumber = [](const std::string& s)
            {
                if (s.empty())
                    return false;
                for (char c : s)
                    if (!isdigit(c))
                        return false;
                return true;
            };

            std::sort(sortedKeys.begin(), sortedKeys.end(), [&](const std::string& a, const std::string& b)
                {
                    bool aIsNum = isNumber(a);
                    bool bIsNum = isNumber(b);

                    if (aIsNum && bIsNum) return std::stoi(a) < std::stoi(b);
                    if (aIsNum) return true;
                    if (bIsNum) return false;
                    return a < b; });

            for (auto& gear : sortedKeys)
            {
                GearInput input = gearInputMap[gear];
                std::string keyNameStr = "Unknown";
                if (input.type == KEYBOARD) {
                    char buffer[64];
                    if (GetKeyNameTextA(MapVirtualKey(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyNameStr = buffer;
                }
                else if (input.type == MOUSE) {
                    switch (input.code) {
                    case 1: keyNameStr = "LMB"; break;
                    case 2: keyNameStr = "RMB"; break;
                    case 3: keyNameStr = "MMB"; break;
                    default: keyNameStr = "Mouse " + std::to_string(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON) {
                    keyNameStr = "vJoy Btn " + std::to_string(input.code);
                }

                bool isActive = (gear == activeGear && gear != "N");
                bool isBeingSet = (keybindBeingSet == gear);
                bool isHovered = (hoveredKeybindGear == gear);

                // --- Modern Row Card ---
                RectF rowRectF((REAL)panelRect.X + 8, (REAL)y, (REAL)panelWidth - 16, (REAL)rowHeight);
                float br = 8.0f;
                GraphicsPath rowPath;
                rowPath.AddArc(rowRectF.X, rowRectF.Y, br * 2, br * 2, 180, 90);
                rowPath.AddArc(rowRectF.X + rowRectF.Width - br * 2, rowRectF.Y, br * 2, br * 2, 270, 90);
                rowPath.AddArc(rowRectF.X + rowRectF.Width - br * 2, rowRectF.Y + rowRectF.Height - br * 2, br * 2, br * 2, 0, 90);
                rowPath.AddArc(rowRectF.X, rowRectF.Y + rowRectF.Height - br * 2, br * 2, br * 2, 90, 90);
                rowPath.CloseFigure();

                // --- Subtle Animation Logic ---
                float activeAlpha = 0.0f;
                if (keybindAnimations.find(gear) != keybindAnimations.end()) {
                    activeAlpha = keybindAnimations[gear].glowAlpha / MAX_GLOW_ALPHA; // 0.0 to 1.0 based on fade
                }

                // Background Gradient with Active Glow
                Color rowBgStart, rowBgEnd;
                if (isBeingSet) {
                    // Intense Pulsing Teal for Capture Mode (Pulse is still useful here)
                    rowBgStart = Color((BYTE)(40 + 40 * pulse), (BYTE)(40 + 60 * pulse), (BYTE)(30 + 50 * pulse));
                    rowBgEnd = Color((BYTE)(20 + 30 * pulse), (BYTE)(20 + 30 * pulse), (BYTE)(15 + 25 * pulse));
                } else if (activeAlpha > 0.01f) {
                    // Subtle Bright Highlight with Fade transition (NO Border)
                    rowBgStart = Color((BYTE)(20 + 50 * activeAlpha), (BYTE)(35 + 85 * activeAlpha), (BYTE)(30 + 70 * activeAlpha));
                    rowBgEnd = Color((BYTE)(10 + 25 * activeAlpha), (BYTE)(20 + 45 * activeAlpha), (BYTE)(15 + 35 * activeAlpha));
                } else {
                    rowBgStart = isHovered ? Color(45, 45, 45) : Color(28, 28, 28);
                    rowBgEnd = isHovered ? Color(35, 35, 35) : Color(24, 24, 24);
                }
                
                LinearGradientBrush rowBg(rowRectF, rowBgStart, rowBgEnd, LinearGradientModeVertical);
                graphics.FillPath(&rowBg, &rowPath);

                // Highlight border ONLY for Capture Mode or Hover (NO Border for Active State)
                if (isBeingSet || isHovered) {
                    Color borderCol;
                    if (isBeingSet) {
                        borderCol = Color((BYTE)(255 * pulse), 0, 255, 170);
                    } else {
                        borderCol = Color(60, 60, 60);
                    }
                    
                    Pen highlightPen(borderCol, isBeingSet ? 2.0f : 1.0f);
                    graphics.DrawPath(&highlightPen, &rowPath);
                }

                // --- Gear Label (Left) ---
                RectF gearRectF(rowRectF.X + 12, rowRectF.Y, 40, rowRectF.Height);
                std::wstring gearW(gear.begin(), gear.end());
                graphics.DrawString(gearW.c_str(), -1, &rowFont, gearRectF, &leftAlign, (isBeingSet || isActive) ? &accentBrush : &subtitleBrush);

                // --- Key Value (Right) ---
                std::string displayKey = keyNameStr;
                std::wstring iconW = L"";
                if (input.type == MOUSE) iconW = L"\x25C9";
                else if (input.type == KEYBOARD) iconW = L"\x2328";
                else if (input.type == VJOY_BUTTON) iconW = L"\x25CE";

                std::wstring keyW = iconW + L" " + std::wstring(displayKey.begin(), displayKey.end());
                RectF keyRectF(rowRectF.X + 60, rowRectF.Y, rowRectF.Width - 100, rowRectF.Height);
                graphics.DrawString(keyW.c_str(), -1, &rowFont, keyRectF, &leftAlign, (isBeingSet || isActive) ? &labelBrush : &textBrush);

                // --- Row Reset Button (↺) ---
                float resetX = rowRectF.X + rowRectF.Width - 35; // move slightly left
                float resetY = rowRectF.Y + (rowRectF.Height - 24) / 2;
                RectF resetBtnRectF(resetX, resetY, 26, 26);
                gearResetBtnRects[gear] = { (int)resetX, (int)resetY, (int)(resetX + 26), (int)(resetY + 26) };

                bool rowResetHover = (hoveredResetGear == gear);
                SolidBrush resetIconBrush(rowResetHover ? Color(255, 255, 80, 80) : Color(120, 150, 150, 150)); // Muted gray by default
                graphics.DrawString(L"\x21BB", -1, &rowFont, resetBtnRectF, &centerAlign, &resetIconBrush);

                y += rowHeight + rowSpacing;
            }
            rightPanelContentBottom = max(rightPanelContentBottom, y);
            y += 10; // extra spacing before next section
        }

        // --- INPUT PANEL ---
        if (showInputPanel)
        {
            int rectWidth = 320;
            inputPanelRectUnified = RectF((REAL)panelRect.X, (REAL)y, (REAL)rectWidth, 220);

            int yInput = (int)inputPanelRectUnified.Y + 10;

            RectF subtitleRect(inputPanelRectUnified.X, (REAL)yInput, (REAL)rectWidth, 20);
            graphics.DrawString(L"Keys & Mouse \x2192 vJoy (Shifter Togglers)", -1, &subtitleFont, subtitleRect, &leftAlign, &subtitleBrush);

            yInput += 28;

            StringFormat rowTextFormat;
            rowTextFormat.SetAlignment(StringAlignmentNear);
            rowTextFormat.SetLineAlignment(StringAlignmentCenter);

            for (size_t i = 0; i < inputMap.size(); ++i)
            {
                InputToVJoy& input = inputMap[i];
                std::wstring keyName = L"Unknown";

                if (input.type == KEYBOARD) {
                    wchar_t buffer[64];
                    if (GetKeyNameTextW(MapVirtualKeyW(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyName = buffer;
                }
                else if (input.type == MOUSE) {
                    switch (input.code) {
                    case 1: keyName = L"LMB"; break;
                    case 2: keyName = L"RMB"; break;
                    case 3: keyName = L"MMB"; break;
                    default: keyName = L"Mouse " + std::to_wstring(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON) {
                    keyName = L"vJoy Btn " + std::to_wstring(input.code);
                }

                bool isBeingSet = (inputBeingSet == std::to_string(i));
                bool isHovered = (hoveredInputIndex == (int)i);

                // Row background (Card)
                RectF rowRect((REAL)inputPanelRectUnified.X + 8, (REAL)yInput, (REAL)panelWidth - 16, (REAL)rowHeight);
                float br = 8.0f;
                GraphicsPath rowPath;
                rowPath.AddArc(rowRect.X, rowRect.Y, br * 2, br * 2, 180, 90);
                rowPath.AddArc(rowRect.X + rowRect.Width - br * 2, rowRect.Y, br * 2, br * 2, 270, 90);
                rowPath.AddArc(rowRect.X + rowRect.Width - br * 2, rowRect.Y + rowRect.Height - br * 2, br * 2, br * 2, 0, 90);
                rowPath.AddArc(rowRect.X, rowRect.Y + rowRect.Height - br * 2, br * 2, br * 2, 90, 90);
                rowPath.CloseFigure();

                Color rowBgStart, rowBgEnd;
                if (isBeingSet) {
                    rowBgStart = Color((BYTE)(40 + 40 * pulse), (BYTE)(40 + 60 * pulse), (BYTE)(30 + 50 * pulse));
                    rowBgEnd = Color((BYTE)(20 + 30 * pulse), (BYTE)(20 + 30 * pulse), (BYTE)(15 + 25 * pulse));
                } else {
                    rowBgStart = isHovered ? Color(45, 45, 45) : Color(28, 28, 28);
                    rowBgEnd = isHovered ? Color(35, 35, 35) : Color(24, 24, 24);
                }
                
                LinearGradientBrush rowBg(rowRect, rowBgStart, rowBgEnd, LinearGradientModeVertical);
                graphics.FillPath(&rowBg, &rowPath);

                if (isBeingSet || isHovered) {
                    Color borderCol = isBeingSet ? Color((BYTE)(255 * pulse), 0, 255, 170) : Color(60, 60, 60);
                    Pen highlightPen(borderCol, isBeingSet ? 2.0f : 1.0f);
                    graphics.DrawPath(&highlightPen, &rowPath);
                }

                std::wstring iconW = L"";
                if (input.type == MOUSE) iconW = L"\x25C9";
                else if (input.type == KEYBOARD) iconW = L"\x2328";
                else if (input.type == VJOY_BUTTON) iconW = L"\x25CE";

                std::wstring combinedText = iconW + L" " + keyName + L" \u2192 vJoy " + std::to_wstring(input.vjoyButton);
                graphics.DrawString(combinedText.c_str(), -1, &rowFont, rowRect, &centerAlign, isBeingSet ? &labelBrush : &textBrush);

                // --- Row Reset Button (↺) ---
                float resetX = rowRect.X + rowRect.Width - 35;
                float resetY = rowRect.Y + (rowRect.Height - 24) / 2;
                RectF resetBtnRectF(resetX, resetY, 26, 26);
                inputResetBtnRects[(int)i] = { (int)resetX, (int)resetY, (int)(resetX + 26), (int)(resetY + 26) };

                bool rowResetHover = (hoveredInputResetIndex == (int)i);
                SolidBrush resetIconBrush(rowResetHover ? Color(255, 255, 80, 80) : Color(120, 150, 150, 150)); // Muted gray by default
                graphics.DrawString(L"\x21BB", -1, &rowFont, resetBtnRectF, &centerAlign, &resetIconBrush);

                yInput += rowHeight + rowSpacing;
            }

            y = yInput + 10;
            rightPanelContentBottom = max(rightPanelContentBottom, y);
        }

        // --- TOGGLE PANEL ---
        // --- TOGGLE PANEL ---
        if (showTogglePanel)
        {
            int rectWidth = 320;
            togglePanelRectUnified = Rect(panelRect.X, y, panelRect.X + rectWidth, y + 200); // store globally
            int yToggle = togglePanelRectUnified.Y + 10;

            // --- Helper for Toggle Cards ---
            auto DrawToggleCard = [&](RectF cardRect, const wchar_t* label, const wchar_t* value, bool isBeingSet, bool isHovered) {
                float br = 8.0f;
                GraphicsPath cardPath;
                cardPath.AddArc(cardRect.X, cardRect.Y, br * 2, br * 2, 180, 90);
                cardPath.AddArc(cardRect.X + cardRect.Width - br * 2, cardRect.Y, br * 2, br * 2, 270, 90);
                cardPath.AddArc(cardRect.X + cardRect.Width - br * 2, cardRect.Y + cardRect.Height - br * 2, br * 2, br * 2, 0, 90);
                cardPath.AddArc(cardRect.X, cardRect.Y + cardRect.Height - br * 2, br * 2, br * 2, 90, 90);
                cardPath.CloseFigure();

                Color bgStart, bgEnd;
                if (isBeingSet) {
                    bgStart = Color((BYTE)(40 + 40 * pulse), (BYTE)(40 + 60 * pulse), (BYTE)(30 + 50 * pulse));
                    bgEnd = Color((BYTE)(20 + 30 * pulse), (BYTE)(20 + 30 * pulse), (BYTE)(15 + 25 * pulse));
                } else {
                    bgStart = isHovered ? Color(45, 45, 45) : Color(28, 28, 28);
                    bgEnd = isHovered ? Color(35, 35, 35) : Color(24, 24, 24);
                }
                
                LinearGradientBrush bg(cardRect, bgStart, bgEnd, LinearGradientModeVertical);
                graphics.FillPath(&bg, &cardPath);

                if (isBeingSet || isHovered) {
                    Color borderCol = isBeingSet ? Color((BYTE)(255 * pulse), 0, 255, 170) : Color(60, 60, 60);
                    Pen highlightPen(borderCol, isBeingSet ? 2.0f : 1.0f);
                    graphics.DrawPath(&highlightPen, &cardPath);
                }

                RectF labelRect(cardRect.X + 8, cardRect.Y + 4, cardRect.Width - 16, 12);
                graphics.DrawString(label, -1, &subtitleFont, labelRect, &leftAlign, &subtitleBrush);

                RectF valRect(cardRect.X + 8, cardRect.Y + 16, cardRect.Width - 16, cardRect.Height - 16);
                graphics.DrawString(value, -1, &rowFont, valRect, &centerAlign, isBeingSet ? &labelBrush : &textBrush);
            };

            // Knob Toggle
            yToggle += 5;
            RectF knobRect((REAL)togglePanelRectUnified.X + 8, (REAL)yToggle, (REAL)panelWidth - 16, (REAL)rowHeight + 10);
            
            wchar_t keyName1[64] = L"";
            if (toggleInputBeingSet) wcscpy_s(keyName1, L"\x2328 Press any key...");
            else if (togglePedalBeingSet) wcscpy_s(keyName1, L"\xD83C\xDFAE Press a pedal...");
            else {
                switch (g_knobToggleType) {
                case TOGGLE_KEYBOARD:
                    if (g_knobToggleKey != 0 && GetKeyNameTextW(MapVirtualKeyW(g_knobToggleKey, MAPVK_VK_TO_VSC) << 16, keyName1, 64) == 0) wcscpy_s(keyName1, L"Unknown Key");
                    else if (g_knobToggleKey == 0) wcscpy_s(keyName1, L"Not Set");
                    break;
                case TOGGLE_MOUSE_LEFT: wcscpy_s(keyName1, L"\xD83D\xDDB1 Mouse: Left"); break;
                case TOGGLE_MOUSE_RIGHT: wcscpy_s(keyName1, L"\xD83D\xDDB1 Mouse: Right"); break;
                case TOGGLE_MOUSE_MIDDLE: wcscpy_s(keyName1, L"\xD83D\xDDB1 Mouse: Middle"); break;
                case TOGGLE_MOUSE_BUTTON4: wcscpy_s(keyName1, L"\xD83D\xDDB1 Mouse: Button 4"); break;
                case TOGGLE_MOUSE_BUTTON5: wcscpy_s(keyName1, L"\xD83D\xDDB1 Mouse: Button 5"); break;
                case TOGGLE_PEDAL_CLUTCH: wcscpy_s(keyName1, L"\xD83C\xDFAE Pedal: Clutch"); break;
                case TOGGLE_PEDAL_BRAKE: wcscpy_s(keyName1, L"\xD83C\xDFAE Pedal: Brake"); break;
                case TOGGLE_PEDAL_ACCEL: wcscpy_s(keyName1, L"\xD83C\xDFAE Pedal: Accel"); break;
                }
            }
            DrawToggleCard(knobRect, L"Activate Knob", keyName1, (toggleInputBeingSet || togglePedalBeingSet), (hoveredToggleIndex == 0));
            
            g_toggleKeyRect = { (LONG)knobRect.X, (LONG)knobRect.Y, (LONG)(knobRect.X + knobRect.Width), (LONG)(knobRect.Y + knobRect.Height) };
            yToggle += (int)knobRect.Height + rowSpacing;

            // Assist Button
            RectF assistRect((REAL)togglePanelRectUnified.X + 8, (REAL)yToggle, (REAL)panelWidth - 16, (REAL)rowHeight + 10);
            wchar_t keyName2[64] = L"";
            if (assistButtonBeingSet) wcscpy_s(keyName2, L"\xD83C\xDFAE Press any button...");
            else if (g) {
                switch ((SDL_GameControllerButton)assistButton) {
                case SDL_CONTROLLER_BUTTON_A: wcscpy_s(keyName2, L"Button A"); break;
                case SDL_CONTROLLER_BUTTON_B: wcscpy_s(keyName2, L"Button B"); break;
                case SDL_CONTROLLER_BUTTON_X: wcscpy_s(keyName2, L"Button X"); break;
                case SDL_CONTROLLER_BUTTON_Y: wcscpy_s(keyName2, L"Button Y"); break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: wcscpy_s(keyName2, L"LB / L1"); break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: wcscpy_s(keyName2, L"RB / R1"); break;
                default: wcscpy_s(keyName2, L"Controller Button"); break;
                }
            } else wcscpy_s(keyName2, L"None");

            DrawToggleCard(assistRect, L"Knob Assist (Controller)", keyName2, assistButtonBeingSet, (hoveredToggleIndex == 1));
            g_assistButtonRect = { (LONG)assistRect.X, (LONG)assistRect.Y, (LONG)(assistRect.X + assistRect.Width), (LONG)(assistRect.Y + assistRect.Height) };
            yToggle += (int)assistRect.Height + rowSpacing;

            // Reverse Unlock
            RectF reverseRect((REAL)togglePanelRectUnified.X + 8, (REAL)yToggle, (REAL)panelWidth - 16, (REAL)rowHeight + 10);
            wchar_t keyName3[64] = L"";
            if (reverseUnlockBeingSet) wcscpy_s(keyName3, L"\x2328 Press any input...");
            else {
                switch (g_reverseUnlockType) {
                case TOGGLE_KEYBOARD:
                    if (g_reverseUnlockKey != 0 && GetKeyNameTextW(MapVirtualKeyW(g_reverseUnlockKey, MAPVK_VK_TO_VSC) << 16, keyName3, 64) == 0) wcscpy_s(keyName3, L"Unknown Key");
                    else if (g_reverseUnlockKey == 0) wcscpy_s(keyName3, L"Not Set");
                    break;
                case TOGGLE_MOUSE_LEFT: wcscpy_s(keyName3, L"\xD83D\xDDB1 Mouse: Left"); break;
                default: wcscpy_s(keyName3, L"Input Not Set"); break;
                }
            }
            DrawToggleCard(reverseRect, L"Reverse Unlock", keyName3, reverseUnlockBeingSet, (hoveredToggleIndex == 2));
            reverseUnlockKeyRect = { (LONG)reverseRect.X, (LONG)reverseRect.Y, (LONG)(reverseRect.X + reverseRect.Width), (LONG)(reverseRect.Y + reverseRect.Height) };
            yToggle += (int)reverseRect.Height + rowSpacing;

            rightPanelContentBottom = max(rightPanelContentBottom, yToggle);
            rightPanelContentBottom = max(rightPanelContentBottom, yToggle);
        }

        // --- Update max scroll for right panel ---
        // Add 50px extra padding at the bottom so the last items aren't stuck at the bottom edge
        int bottomPadding = 50;
        rightPanelScrollMax = max(0, (rightPanelContentBottom + bottomPadding - rightPanelScrollOffset) - (panelRect.Y + panelRect.Height - 10));

        graphics.ResetClip();
    }

