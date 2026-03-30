#ifndef SHIFTER_TOP_DROPDOWNS_CPP
#define SHIFTER_TOP_DROPDOWNS_CPP

// --- Dropdown Fragment for ShifterUI.cpp ---
{
    using namespace Gdiplus;
    
    // Fix: Also reset transform for dropdowns
    graphics.ResetTransform();

    // Shared parameters for menus
    int listItemHeight = 32;
    int itemGap = 2;
    int listPadding = 8;
    StringFormat dropdownF; dropdownF.SetLineAlignment(StringAlignmentCenter);
    POINT dMouse; GetCursorPos(&dMouse); ScreenToClient(hwnd, &dMouse);
    
    // 1. Gear Label Dropdown
    if (gearLayoutDropdownOpen && !gearLayoutNames.empty()) {
        float menuW = 180.0f;
        float menuH = (float)((listItemHeight + itemGap) * gearLayoutNames.size() + listPadding);
        float menuX = topGearLabelBtnRect.left + (topGearLabelBtnRect.right - topGearLabelBtnRect.left - menuW) / 2.0f;
        // Drop-up from the TOP of the first row
        int listY = topGearLabelBtnRect.top - (int)menuH - 5; 
        
        RectF menuRect(menuX, (float)listY, menuW, menuH);
        
        GraphicsPath mPath; float mr = 10.0f;
        mPath.AddArc(menuRect.X, menuRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(menuRect.X, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(245, 12, 12, 12));
        graphics.FillPath(&mBg, &mPath);
        Pen mPen(Color(100, 0, 255, 170), 1.2f);
        graphics.DrawPath(&mPen, &mPath);
        
        for (size_t i = 0; i < gearLayoutNames.size(); ++i) {
            RectF itemR(menuRect.X + 4, menuRect.Y + 4 + i * (listItemHeight + itemGap), menuRect.Width - 8, (REAL)listItemHeight);
            RECT itemRWin = {(int)itemR.X, (int)itemR.Y, (int)(itemR.X + itemR.Width), (int)(itemR.Y + itemR.Height)};
            
            bool hovered = PtInRect(&itemRWin, dMouse);
            bool selected = (i == (size_t)currentGearLayout);
            
            if (hovered || selected) {
                GraphicsPath ip; float ir = 5.0f;
                ip.AddArc(itemR.X, itemR.Y, ir * 2, ir * 2, 180, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y, ir * 2, ir * 2, 270, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                ip.AddArc(itemR.X, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                ip.CloseFigure();
                graphics.FillPath(selected ? &accentBrush : &highlightBrush, &ip);
            }
            
            graphics.DrawString(gearLayoutNames[i].c_str(), -1, &rowFont, RectF(itemR.X + 10, itemR.Y, itemR.Width - 20, itemR.Height), &dropdownF, selected ? &darkBrush : &valueBrush);
        }
    }

    // 2. Profile Dropdown
    if (profileDropdownOpen && !profileNames.empty()) {
        float menuW = 200.0f;
        float menuH = (float)((listItemHeight + itemGap) * profileNames.size() + listPadding);
        float menuX = topProfileBtnRect.left + (topProfileBtnRect.right - topProfileBtnRect.left - menuW) / 2.0f;
        // Drop-up from the TOP of the first row
        int listY = topProfileBtnRect.top - (int)menuH - 5;
        
        RectF menuRect(menuX, (float)listY, menuW, menuH);
        
        GraphicsPath mPath; float mr = 10.0f;
        mPath.AddArc(menuRect.X, menuRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(menuRect.X, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(245, 12, 12, 12));
        graphics.FillPath(&mBg, &mPath);
        Pen mPen(Color(100, 0, 255, 170), 1.2f);
        graphics.DrawPath(&mPen, &mPath);
        
        for (size_t i = 0; i < profileNames.size(); ++i) {
            RectF itemR(menuRect.X + 4, menuRect.Y + 4 + i * (listItemHeight + itemGap), menuRect.Width - 8, (REAL)listItemHeight);
            RECT itemRWin = {(int)itemR.X, (int)itemR.Y, (int)(itemR.X + itemR.Width), (int)(itemR.Y + itemR.Height)};
            
            bool hovered = PtInRect(&itemRWin, dMouse);
            bool selected = (i == (size_t)currentProfileIndex);
            
            if (hovered || selected) {
                GraphicsPath ip; float ir = 5.0f;
                ip.AddArc(itemR.X, itemR.Y, ir * 2, ir * 2, 180, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y, ir * 2, ir * 2, 270, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                ip.AddArc(itemR.X, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                ip.CloseFigure();
                graphics.FillPath(selected ? &accentBrush : &highlightBrush, &ip);
            }
            
            std::string p = profileNames[i];
            if (p.size() > 4 && p.substr(p.size() - 4) == ".ini") p = p.substr(0, p.size() - 4);
            std::wstring pW(p.begin(), p.end());
            
            graphics.DrawString(pW.c_str(), -1, &rowFont, RectF(itemR.X + 10, itemR.Y, itemR.Width - 20, itemR.Height), &dropdownF, selected ? &darkBrush : &valueBrush);
        }
    }
    
    // 3. Layout Dropdown
    if (hShifterLayoutDropdownOpen && !hShifterLayouts.empty()) {
        int listY = topLayoutBtnRect.bottom + 5;
        float menuW = 240.0f;
        float menuH = (float)((listItemHeight + itemGap) * hShifterLayouts.size() + listPadding);
        float menuX = topLayoutBtnRect.left + (topLayoutBtnRect.right - topLayoutBtnRect.left - menuW) / 2.0f;
        
        RectF menuRect(menuX, (float)listY, menuW, menuH);
        
        GraphicsPath mPath; float mr = 10.0f;
        mPath.AddArc(menuRect.X, menuRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(menuRect.X, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(245, 12, 12, 12));
        graphics.FillPath(&mBg, &mPath);
        Pen mPen(Color(100, 0, 255, 170), 1.2f);
        graphics.DrawPath(&mPen, &mPath);
        
        for (size_t i = 0; i < hShifterLayouts.size(); ++i) {
            RectF itemR(menuRect.X + 4, menuRect.Y + 4 + i * (listItemHeight + itemGap), menuRect.Width - 8, (REAL)listItemHeight);
            RECT itemRWin = {(int)itemR.X, (int)itemR.Y, (int)(itemR.X + itemR.Width), (int)(itemR.Y + itemR.Height)};
            
            bool hovered = PtInRect(&itemRWin, dMouse);
            bool selected = (hShifterLayouts[i].id == currentHShifterLayout);
            
            if (hovered || selected) {
                GraphicsPath ip; float ir = 5.0f;
                ip.AddArc(itemR.X, itemR.Y, ir * 2, ir * 2, 180, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y, ir * 2, ir * 2, 270, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                ip.AddArc(itemR.X, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                ip.CloseFigure();
                graphics.FillPath(selected ? &accentBrush : &highlightBrush, &ip);
            }
            
            graphics.DrawString(hShifterLayouts[i].name.c_str(), -1, &rowFont, RectF(itemR.X + 10, itemR.Y, itemR.Width - 20, itemR.Height), &dropdownF, selected ? &darkBrush : &valueBrush);
        }
    }

    // 4. Mouse Device Dropdown
    if (mouseDeviceDropdownOpen) {
        RECT rS = GetScrolledRect(deviceComboRect);
        float menuW = (float)(rS.right - rS.left);
        float menuH = (float)((listItemHeight + itemGap) * (g_mouseDevices.size() + 1) + listPadding);
        float menuX = (float)rS.left;
        float menuY = (float)rS.bottom + 2.0f;
        
        RectF menuRect(menuX, menuY, menuW, menuH);
        GraphicsPath mPath; float mr = 8.0f;
        mPath.AddArc(menuRect.X, menuRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(menuRect.X, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(250, 15, 15, 15));
        graphics.FillPath(&mBg, &mPath);
        graphics.DrawPath(&accentPen, &mPath);
        
        for (size_t i = 0; i < g_mouseDevices.size() + 1; ++i) {
            RectF itemR(menuRect.X + 4, menuRect.Y + 4 + i * (listItemHeight + itemGap), menuRect.Width - 8, (REAL)listItemHeight);
            RECT itemRWin = {(int)itemR.X, (int)itemR.Y, (int)(itemR.X + itemR.Width), (int)(itemR.Y + itemR.Height)};
            bool hovered = PtInRect(&itemRWin, dMouse);
            
            HANDLE hDev = (i == 0) ? NULL : g_mouseDevices[i-1].hDevice;
            bool selected = (g_selectedDevice == hDev);
            
            if (hovered || selected) {
                GraphicsPath ip; float ir = 4.0f;
                ip.AddArc(itemR.X, itemR.Y, ir * 2, ir * 2, 180, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y, ir * 2, ir * 2, 270, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                ip.AddArc(itemR.X, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                ip.CloseFigure();
                graphics.FillPath(selected ? &accentBrush : &highlightBrush, &ip);
            }
            std::wstring name = (i == 0) ? L"All Mice" : g_mouseDevices[i-1].name;
            graphics.DrawString(name.c_str(), -1, &rowFont, RectF(itemR.X + 10, itemR.Y, itemR.Width - 20, itemR.Height), &dropdownF, selected ? &darkBrush : &valueBrush);
        }
    }

    // 5. Steering Mouse Dropdown
    if (steeringDeviceDropdownOpen) {
        RECT rS = GetScrolledRect(steeringDeviceComboRect);
        float menuW = (float)(rS.right - rS.left);
        float menuH = (float)((listItemHeight + itemGap) * (g_mouseDevices.size() + 1) + listPadding);
        float menuX = (float)rS.left;
        float menuY = (float)rS.bottom + 2.0f;
        
        RectF menuRect(menuX, menuY, menuW, menuH);
        GraphicsPath mPath; float mr = 8.0f;
        mPath.AddArc(menuRect.X, menuRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(menuRect.X + menuRect.Width - mr * 2, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(menuRect.X, menuRect.Y + menuRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(250, 15, 15, 15));
        graphics.FillPath(&mBg, &mPath);
        graphics.DrawPath(&accentPen, &mPath);
        
        for (size_t i = 0; i < g_mouseDevices.size() + 1; ++i) {
            RectF itemR(menuRect.X + 4, menuRect.Y + 4 + i * (listItemHeight + itemGap), menuRect.Width - 8, (REAL)listItemHeight);
            RECT itemRWin = {(int)itemR.X, (int)itemR.Y, (int)(itemR.X + itemR.Width), (int)(itemR.Y + itemR.Height)};
            bool hovered = PtInRect(&itemRWin, dMouse);
            
            HANDLE hDev = (i == 0) ? NULL : g_mouseDevices[i-1].hDevice;
            bool selected = (g_selectedSteeringDevice == hDev);
            
            if (hovered || selected) {
                GraphicsPath ip; float ir = 4.0f;
                ip.AddArc(itemR.X, itemR.Y, ir * 2, ir * 2, 180, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y, ir * 2, ir * 2, 270, 90);
                ip.AddArc(itemR.X + itemR.Width - ir * 2, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                ip.AddArc(itemR.X, itemR.Y + itemR.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                ip.CloseFigure();
                graphics.FillPath(selected ? &accentBrush : &highlightBrush, &ip);
            }
            std::wstring name = (i == 0) ? L"All Mice" : g_mouseDevices[i-1].name;
            graphics.DrawString(name.c_str(), -1, &rowFont, RectF(itemR.X + 10, itemR.Y, itemR.Width - 20, itemR.Height), &dropdownF, selected ? &darkBrush : &valueBrush);
        }
    }

    // 6. Process Picker Modal
    if (processPickerModalOpen) {
        // Full screen dimming
        graphics.FillRectangle(&highlightBrush, RectF(0, 0, (REAL)width, (REAL)height));
        
        float modalW = 500.0f;
        float modalH = 400.0f;
        float modalX = (width - modalW) / 2.0f;
        float modalY = (height - modalH) / 2.0f;
        
        RectF modalRect(modalX, modalY, modalW, modalH);
        processPickerBoxRect = {(int)modalRect.X, (int)modalRect.Y, (int)(modalRect.X + modalRect.Width), (int)(modalRect.Y + modalRect.Height)};
        
        GraphicsPath mPath; float mr = 15.0f;
        mPath.AddArc(modalRect.X, modalRect.Y, mr * 2, mr * 2, 180, 90);
        mPath.AddArc(modalRect.X + modalRect.Width - mr * 2, modalRect.Y, mr * 2, mr * 2, 270, 90);
        mPath.AddArc(modalRect.X + modalRect.Width - mr * 2, modalRect.Y + modalRect.Height - mr * 2, mr * 2, mr * 2, 0, 90);
        mPath.AddArc(modalRect.X, modalRect.Y + modalRect.Height - mr * 2, mr * 2, mr * 2, 90, 90);
        mPath.CloseFigure();
        
        SolidBrush mBg(Color(255, 10, 10, 10));
        graphics.FillPath(&mBg, &mPath);
        graphics.DrawPath(&accentPen, &mPath);
        
        // Modal Title
        RectF titleRect(modalRect.X, modalRect.Y + 20, modalRect.Width, 30.0f);
        StringFormat titleF; titleF.SetAlignment(StringAlignmentCenter);
        graphics.DrawString(L"Select Game Application", -1, &headingFont, titleRect, &titleF, &titleBrush);

        // Refresh Button in Modal
        RectF refreshBtn(modalRect.X + modalRect.Width - 45, modalRect.Y + 15, 30, 30);
        RECT refreshBtnR = {(int)refreshBtn.X, (int)refreshBtn.Y, (int)(refreshBtn.X + refreshBtn.Width), (int)(refreshBtn.Y + refreshBtn.Height)};
        bool refHover = PtInRect(&refreshBtnR, dMouse);
        if (refHover) graphics.FillEllipse(&highlightBrush, refreshBtn);
        // Draw ↻ symbol
        graphics.DrawString(L"\x21BB", -1, &headingFont, refreshBtn, &titleF, refHover ? &accentBrush : &valueBrush);
        
        // Modal Action Rect for Refresh (Hack: reuse a global rect for hit testing if needed)
        g_modalActionRect = refreshBtnR; 
        
        // Scrollable List Area
        RectF listRect(modalRect.X + 20, modalRect.Y + 60, modalRect.Width - 40, modalRect.Height - 100);
        graphics.SetClip(listRect);
        
        processPickerScrollOffset += (processPickerScrollTarget - processPickerScrollOffset) * 0.2f;
        
        int rowH = 35;
        hoveredProcessIndex = -1;
        for (size_t i = 0; i < g_processList.size(); ++i) {
            float rowY = listRect.Y + i * rowH + processPickerScrollOffset;
            RectF rowRect(listRect.X, rowY, listRect.Width, (REAL)rowH);
            
            if (rowY + rowH < listRect.Y || rowY > listRect.Y + listRect.Height) continue;
            
            RECT rowRectWin = {(int)rowRect.X, (int)rowRect.Y, (int)(rowRect.X + rowRect.Width), (int)(rowRect.Y + rowRect.Height)};
            bool hovered = PtInRect(&rowRectWin, dMouse);
            if (hovered) hoveredProcessIndex = (int)i;
            bool selected = (g_processList[i] == g_selectedProcessId);
            
            if (hovered || selected) {
                graphics.FillRectangle(selected ? &accentBrush : &highlightBrush, rowRect);
            }
            
            // Get process name
            std::wstring procName = L"Unknown Process";
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, g_processList[i]);
            if (hProcess) {
                wchar_t buf[MAX_PATH];
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameW(hProcess, 0, buf, &size)) {
                    std::wstring fullPath(buf);
                    size_t pos = fullPath.find_last_of(L"\\/");
                    procName = (pos != std::wstring::npos) ? fullPath.substr(pos + 1) : fullPath;
                }
                CloseHandle(hProcess);
            }
            // Add PID for clarity
            procName += L" (PID: " + std::to_wstring(g_processList[i]) + L")";
            
            RectF textRect = rowRect; textRect.X += 10;
            graphics.DrawString(procName.c_str(), -1, &rowFont, textRect, &dropdownF, selected ? &darkBrush : &valueBrush);
        }
        graphics.ResetClip();
        
        // Cancel Hint
        StringFormat hintF; hintF.SetAlignment(StringAlignmentCenter);
        RectF hintRect(modalRect.X, modalRect.Y + modalRect.Height - 35, modalRect.Width, 20.0f);
        graphics.DrawString(L"Click outside to close", -1, &rowFont, hintRect, &hintF, &valueBrush);
    }
}

#endif
