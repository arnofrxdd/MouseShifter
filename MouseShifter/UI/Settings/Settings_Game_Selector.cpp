        // --- Game Selection ---
        currentY += 40;
        graphics.DrawLine(&linePen, settingsPanelRect.left + 40, currentY + settingsScrollOffset, settingsPanelRect.right - 40, currentY + settingsScrollOffset);
        currentY += 20;
        
        RectF gameHeadingRect((REAL)settingsPanelRect.left, (REAL)(currentY + settingsScrollOffset), (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Game Target Selection", -1, &headingFont, gameHeadingRect, &centerFormat, &labelBrush);
        currentY += 40;

        refreshButtonRect = { settingsPanelRect.right - margin - 40, currentY, settingsPanelRect.right - margin, currentY + comboHeight };
        processComboRect = { settingsPanelRect.left + margin, currentY, settingsPanelRect.right - margin - 50, currentY + comboHeight };
        
        RectF processBoxRect((REAL)processComboRect.left, (REAL)(processComboRect.top + settingsScrollOffset), (REAL)(processComboRect.right - processComboRect.left), (REAL)(processComboRect.bottom - processComboRect.top));
        RectF refreshBoxRect((REAL)refreshButtonRect.left, (REAL)(refreshButtonRect.top + settingsScrollOffset), (REAL)(refreshButtonRect.right - refreshButtonRect.left), (REAL)(refreshButtonRect.bottom - refreshButtonRect.top));

        float gameBr = 6.0f;
        bool isHoveredProc = !anyDropdownOpen && PtInRect(&processComboRect, itemAdjCursor);
        bool isHoveredRef = !anyDropdownOpen && PtInRect(&refreshButtonRect, itemAdjCursor);

        float gBr = 6.0f;
        // Process Dropdown Box
        GraphicsPath procPath;
        procPath.AddArc(processBoxRect.X, processBoxRect.Y, gBr * 2, gBr * 2, 180, 90);
        procPath.AddArc(processBoxRect.X + processBoxRect.Width - gBr * 2, processBoxRect.Y, gBr * 2, gBr * 2, 270, 90);
        procPath.AddArc(processBoxRect.X + processBoxRect.Width - gBr * 2, processBoxRect.Y + processBoxRect.Height - gBr * 2, gBr * 2, gBr * 2, 0, 90);
        procPath.AddArc(processBoxRect.X, processBoxRect.Y + processBoxRect.Height - gBr * 2, gBr * 2, gBr * 2, 90, 90);
        procPath.CloseFigure();
        graphics.FillPath(isHoveredProc ? &highlightBrush : &darkBrush, &procPath);
        graphics.DrawPath(&accentPen, &procPath);

        // Refresh Button
        GraphicsPath refPath;
        refPath.AddArc(refreshBoxRect.X, refreshBoxRect.Y, gBr * 2, gBr * 2, 180, 90);
        refPath.AddArc(refreshBoxRect.X + refreshBoxRect.Width - gBr * 2, refreshBoxRect.Y, gBr * 2, gBr * 2, 270, 90);
        refPath.AddArc(refreshBoxRect.X + refreshBoxRect.Width - gBr * 2, refreshBoxRect.Y + refreshBoxRect.Height - gBr * 2, gBr * 2, gBr * 2, 0, 90);
        refPath.AddArc(refreshBoxRect.X, refreshBoxRect.Y + refreshBoxRect.Height - gBr * 2, gBr * 2, gBr * 2, 90, 90);
        refPath.CloseFigure();
        graphics.FillPath(isHoveredRef ? &accentBrush : &darkBrush, &refPath);
        graphics.DrawPath(&accentPen, &refPath);
        StringFormat centerF; centerF.SetAlignment(StringAlignmentCenter); centerF.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(L"\x21BA", -1, &rowFont, refreshBoxRect, &centerF, isHoveredRef ? &darkBrush : &valueBrush);

        std::wstring gameName = L"Click to Select Game";
        if (g_selectedProcessId != 0) {
            std::wstring windowTitle = L"";
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                std::wstring* result = reinterpret_cast<std::wstring*>(lParam);
                DWORD windowProcessId;
                GetWindowThreadProcessId(hwnd, &windowProcessId);
                if (windowProcessId == g_selectedProcessId && IsWindowVisible(hwnd)) {
                    wchar_t title[256];
                    if (GetWindowTextW(hwnd, title, 256) > 0) {
                        *result = title;
                        if (!result->empty() && *result != L"Default IME" && *result != L"MSCTFIME UI") return FALSE;
                    }
                }
                return TRUE;
            }, reinterpret_cast<LPARAM>(&windowTitle));

            std::wstring exeName = L"";
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe; pe.dwSize = sizeof(PROCESSENTRY32);
                if (Process32First(hSnapshot, &pe)) {
                    do { if (pe.th32ProcessID == g_selectedProcessId) { exeName = pe.szExeFile; break; } } while (Process32Next(hSnapshot, &pe));
                }
                CloseHandle(hSnapshot);
            }
            if (!windowTitle.empty() && windowTitle != L"Default IME" && windowTitle != L"MSCTFIME UI") gameName = windowTitle + L" (" + exeName + L")";
            else gameName = exeName;
        }

        StringFormat gameVertCenter; gameVertCenter.SetLineAlignment(StringAlignmentCenter); gameVertCenter.SetFormatFlags(StringFormatFlagsNoWrap); gameVertCenter.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF gameTextRect = processBoxRect; gameTextRect.X += 10; gameTextRect.Width -= 20;
        graphics.DrawString(gameName.c_str(), -1, &rowFont, gameTextRect, &gameVertCenter, &valueBrush);
        
        // --- Legacy Buttons (Mouse Free / XInput Free) ---
        currentY += comboHeight + 15;
        int buttonWidth = 125;
        int buttonHeight = 30;
        int buttonSpacing = 10;

        mouseBlockCheckboxRect = { settingsPanelRect.left + margin, currentY, settingsPanelRect.left + margin + buttonWidth, currentY + buttonHeight };
        xinputBlockCheckboxRect = { settingsPanelRect.left + margin + buttonWidth + buttonSpacing, currentY, settingsPanelRect.left + margin + buttonWidth + buttonSpacing + buttonWidth, currentY + buttonHeight };

        RectF mouseBtnRect((REAL)mouseBlockCheckboxRect.left, (REAL)(mouseBlockCheckboxRect.top + settingsScrollOffset), (REAL)buttonWidth, (REAL)buttonHeight);
        RectF xinputBtnRect((REAL)xinputBlockCheckboxRect.left, (REAL)(xinputBlockCheckboxRect.top + settingsScrollOffset), (REAL)buttonWidth, (REAL)buttonHeight);

        bool isHoveredM = !anyDropdownOpen && PtInRect(&mouseBlockCheckboxRect, itemAdjCursor);
        bool isHoveredX = !anyDropdownOpen && PtInRect(&xinputBlockCheckboxRect, itemAdjCursor);

        auto DrawPillButton = [&](RectF r, bool active, bool hovered, const wchar_t* text) {
            GraphicsPath p;
            float pr = r.Height / 2;
            p.AddArc(r.X, r.Y, pr * 2, pr * 2, 180, 90);
            p.AddArc(r.X + r.Width - pr * 2, r.Y, pr * 2, pr * 2, 270, 90);
            p.AddArc(r.X + r.Width - pr * 2, r.Y + r.Height - pr * 2, pr * 2, pr * 2, 0, 90);
            p.AddArc(r.X, r.Y + r.Height - pr * 2, pr * 2, pr * 2, 90, 90);
            p.CloseFigure();
            graphics.FillPath(active ? &accentBrush : (hovered ? &highlightBrush : &darkBrush), &p);
            graphics.DrawPath(&accentPen, &p);
            StringFormat cf; cf.SetAlignment(StringAlignmentCenter); cf.SetLineAlignment(StringAlignmentCenter);
            graphics.DrawString(text, -1, &rowFont, r, &cf, active ? &darkBrush : &labelBrush);
        };

        DrawPillButton(mouseBtnRect, g_mouseBlockEnabled, isHoveredM, g_mouseBlockEnabled ? L"Mouse Blocked" : L"Mouse Free");
        DrawPillButton(xinputBtnRect, g_xinputBlockEnabled, isHoveredX, g_xinputBlockEnabled ? L"XInput Blocked" : L"XInput Free");

        // Instruction Text
        currentY += buttonHeight + 8;
        PointF instructionPos((REAL)mouseBlockCheckboxRect.left, (REAL)(currentY + settingsScrollOffset));
        SolidBrush instructionBrush(Color(150, 150, 150));
        static Font smallFont(&fontFamily, 11, FontStyleRegular, UnitPixel);
        graphics.DrawString(L"Hold RMB to use mouse", -1, &smallFont, instructionPos, &instructionBrush);

        currentY += 25;