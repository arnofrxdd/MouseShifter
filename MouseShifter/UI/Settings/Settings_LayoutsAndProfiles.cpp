        // --- H-Shifter Layout ---
        currentY += 40;
        hShifterLayoutButtonRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + toggleHeight };

        PointF hShifterLayoutLabelPos((REAL)hShifterLayoutButtonRect.left, (REAL)(hShifterLayoutButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"H-Shifter Layout:", -1, &rowFont, hShifterLayoutLabelPos, &labelBrush);
        tooltips[11].bounds = hShifterLayoutButtonRect;
        tooltips[11].text = L"Select the physical layout and number of gear positions for your H-shifter.";
        
        RectF hShifterLayoutBoxRect(
            (REAL)hShifterLayoutButtonRect.left,
            (REAL)(hShifterLayoutButtonRect.top + settingsScrollOffset),
            (REAL)(hShifterLayoutButtonRect.right - hShifterLayoutButtonRect.left),
            (REAL)hShifterLayoutButtonRect.bottom - hShifterLayoutButtonRect.top
        );

        bool isHoveredLayout = !anyDropdownOpen && PtInRect(&hShifterLayoutButtonRect, itemAdjCursor);

        // Rounded Box
        GraphicsPath boxPath;
        float br = 6.0f;
        boxPath.AddArc(hShifterLayoutBoxRect.X, hShifterLayoutBoxRect.Y, br * 2, br * 2, 180, 90);
        boxPath.AddArc(hShifterLayoutBoxRect.X + hShifterLayoutBoxRect.Width - br * 2, hShifterLayoutBoxRect.Y, br * 2, br * 2, 270, 90);
        boxPath.AddArc(hShifterLayoutBoxRect.X + hShifterLayoutBoxRect.Width - br * 2, hShifterLayoutBoxRect.Y + hShifterLayoutBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        boxPath.AddArc(hShifterLayoutBoxRect.X, hShifterLayoutBoxRect.Y + hShifterLayoutBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        boxPath.CloseFigure();

        graphics.FillPath(isHoveredLayout ? &highlightBrush : &darkBrush, &boxPath);
        graphics.DrawPath(&accentPen, &boxPath);

        std::wstring hShifterLayoutName = hShifterLayouts[currentHShifterLayout - 1].name;
        StringFormat vertCenter; vertCenter.SetLineAlignment(StringAlignmentCenter);
        RectF textRect = hShifterLayoutBoxRect; textRect.X += 10; textRect.Width -= 40;
        graphics.DrawString(hShifterLayoutName.c_str(), -1, &rowFont, textRect, &vertCenter, &valueBrush);

        // Custom Chevron
        float chevSize = 8.0f;
        float chevX = hShifterLayoutBoxRect.X + hShifterLayoutBoxRect.Width - 15;
        float chevY = hShifterLayoutBoxRect.Y + hShifterLayoutBoxRect.Height / 2;
        PointF chevPts[3] = { {chevX - chevSize / 2, chevY - chevSize / 4}, {chevX + chevSize / 2, chevY - chevSize / 4}, {chevX, chevY + chevSize / 4} };
        graphics.FillPolygon(&accentBrush, chevPts, 3);

        // --- Gear Layout Cycle Button ---
        currentY += toggleHeight + 50;
        gearLayoutButtonRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + toggleHeight };

        PointF layoutLabelPos((REAL)gearLayoutButtonRect.left, (REAL)(gearLayoutButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"Gear Label Layout:", -1, &rowFont, layoutLabelPos, &labelBrush);
        tooltips[12].bounds = gearLayoutButtonRect;
        tooltips[12].text = L"Select the visual theme and styling for gear number labels and text display.";
        
        RectF layoutBoxRect(
            (REAL)gearLayoutButtonRect.left,
            (REAL)(gearLayoutButtonRect.top + settingsScrollOffset),
            (REAL)(gearLayoutButtonRect.right - gearLayoutButtonRect.left),
            (REAL)gearLayoutButtonRect.bottom - gearLayoutButtonRect.top
        );

        bool isHoveredGear = PtInRect(&gearLayoutButtonRect, itemAdjCursor);
        GraphicsPath gearPath;
        gearPath.AddArc(layoutBoxRect.X, layoutBoxRect.Y, br * 2, br * 2, 180, 90);
        gearPath.AddArc(layoutBoxRect.X + layoutBoxRect.Width - br * 2, layoutBoxRect.Y, br * 2, br * 2, 270, 90);
        gearPath.AddArc(layoutBoxRect.X + layoutBoxRect.Width - br * 2, layoutBoxRect.Y + layoutBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        gearPath.AddArc(layoutBoxRect.X, layoutBoxRect.Y + layoutBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        gearPath.CloseFigure();

        graphics.FillPath(isHoveredGear ? &highlightBrush : &darkBrush, &gearPath);
        graphics.DrawPath(&accentPen, &gearPath);

        std::wstring layoutName = gearLayoutNames[currentGearLayout];
        RectF gearTextRect = layoutBoxRect; gearTextRect.X += 10; gearTextRect.Width -= 40;
        graphics.DrawString(layoutName.c_str(), -1, &rowFont, gearTextRect, &vertCenter, &valueBrush);

        float gChevX = layoutBoxRect.X + layoutBoxRect.Width - 15;
        float gChevY = layoutBoxRect.Y + layoutBoxRect.Height / 2;
        PointF gChevPts[3] = { {gChevX - chevSize / 2, gChevY - chevSize / 4}, {gChevX + chevSize / 2, gChevY - chevSize / 4}, {gChevX, gChevY + chevSize / 4} };
        graphics.FillPolygon(&accentBrush, gChevPts, 3);

        // --- Profile Selector ---
        currentY += toggleHeight + 50;
        profileButtonRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + toggleHeight };

        PointF profileLabelPos((REAL)profileButtonRect.left, (REAL)(profileButtonRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"Profile:", -1, &rowFont, profileLabelPos, &labelBrush);
        tooltips[13].bounds = profileButtonRect;
        tooltips[13].text = L"Select or create configuration profiles for different games or setups.";

        RectF profileBoxRect(
            (REAL)profileButtonRect.left,
            (REAL)(profileButtonRect.top + settingsScrollOffset),
            (REAL)(profileButtonRect.right - profileButtonRect.left),
            (REAL)profileButtonRect.bottom - profileButtonRect.top
        );

        bool isHoveredProf = !anyDropdownOpen && PtInRect(&profileButtonRect, itemAdjCursor);
        GraphicsPath profPath;
        profPath.AddArc(profileBoxRect.X, profileBoxRect.Y, br * 2, br * 2, 180, 90);
        profPath.AddArc(profileBoxRect.X + profileBoxRect.Width - br * 2, profileBoxRect.Y, br * 2, br * 2, 270, 90);
        profPath.AddArc(profileBoxRect.X + profileBoxRect.Width - br * 2, profileBoxRect.Y + profileBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        profPath.AddArc(profileBoxRect.X, profileBoxRect.Y + profileBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        profPath.CloseFigure();

        graphics.FillPath(isHoveredProf ? &highlightBrush : &darkBrush, &profPath);
        graphics.DrawPath(&accentPen, &profPath);

        // Display current profile name
        std::string currentProfileDisplay;
        if (!profileNames.empty()) {
            currentProfileDisplay = profileNames[currentProfileIndex];
            if (currentProfileDisplay.size() > 4 && currentProfileDisplay.substr(currentProfileDisplay.size() - 4) == ".ini")
                currentProfileDisplay = currentProfileDisplay.substr(0, currentProfileDisplay.size() - 4);
        }
        else currentProfileDisplay = "No Profiles";

        std::wstring profileDisplayW(currentProfileDisplay.begin(), currentProfileDisplay.end());
        RectF profileTextRect = profileBoxRect; profileTextRect.X += 10; profileTextRect.Width -= 40;
        graphics.DrawString(profileDisplayW.c_str(), -1, &rowFont, profileTextRect, &vertCenter, &valueBrush);

        float pChevX = profileBoxRect.X + profileBoxRect.Width - 15;
        float pChevY = profileBoxRect.Y + profileBoxRect.Height / 2;
        PointF pChevPts[3] = { {pChevX - chevSize / 2, pChevY - chevSize / 4}, {pChevX + chevSize / 2, pChevY - chevSize / 4}, {pChevX, pChevY + chevSize / 4} };
        graphics.FillPolygon(&accentBrush, pChevPts, 3);

        // --- Create New Profile Button ---
        currentY += toggleHeight + 10;
        createProfileButtonRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + toggleHeight };

        RectF createProfileBoxRect((REAL)createProfileButtonRect.left, (REAL)(createProfileButtonRect.top + settingsScrollOffset), (REAL)(createProfileButtonRect.right - createProfileButtonRect.left), (REAL)createProfileButtonRect.bottom - createProfileButtonRect.top);
        bool isHoveredCreate = !anyDropdownOpen && PtInRect(&createProfileButtonRect, itemAdjCursor);
        
        GraphicsPath createPath;
        createPath.AddArc(createProfileBoxRect.X, createProfileBoxRect.Y, br * 2, br * 2, 180, 90);
        createPath.AddArc(createProfileBoxRect.X + createProfileBoxRect.Width - br * 2, createProfileBoxRect.Y, br * 2, br * 2, 270, 90);
        createPath.AddArc(createProfileBoxRect.X + createProfileBoxRect.Width - br * 2, createProfileBoxRect.Y + createProfileBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        createPath.AddArc(createProfileBoxRect.X, createProfileBoxRect.Y + createProfileBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        createPath.CloseFigure();

        SolidBrush createIdleBrush(Color(60, 60, 60));
        graphics.FillPath(isHoveredCreate ? &accentBrush : &createIdleBrush, &createPath);
        graphics.DrawPath(&accentPen, &createPath);

        graphics.DrawString(L"Create New Profile", -1, &rowFont, createProfileBoxRect, &vertCenter, isHoveredCreate ? &darkBrush : &labelBrush);

        // --- Mouse Device Selector ---
        currentY += toggleHeight + 50;
        deviceComboRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + comboHeight };

        PointF deviceLabelPos((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"H-Shifter Mouse Device:", -1, &rowFont, deviceLabelPos, &labelBrush);
        tooltips[14].bounds = deviceComboRect;
        tooltips[14].text = L"Select which mouse device controls the H-shifter knob movement.";
        
        RectF deviceBoxRect((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top + settingsScrollOffset), (REAL)(deviceComboRect.right - deviceComboRect.left), (REAL)(deviceComboRect.bottom - deviceComboRect.top));
        bool isHoveredDev = PtInRect(&deviceComboRect, itemAdjCursor);
        
        GraphicsPath devPath;
        devPath.AddArc(deviceBoxRect.X, deviceBoxRect.Y, br * 2, br * 2, 180, 90);
        devPath.AddArc(deviceBoxRect.X + deviceBoxRect.Width - br * 2, deviceBoxRect.Y, br * 2, br * 2, 270, 90);
        devPath.AddArc(deviceBoxRect.X + deviceBoxRect.Width - br * 2, deviceBoxRect.Y + deviceBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        devPath.AddArc(deviceBoxRect.X, deviceBoxRect.Y + deviceBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        devPath.CloseFigure();

        graphics.FillPath(isHoveredDev ? &highlightBrush : &darkBrush, &devPath);
        graphics.DrawPath(&accentPen, &devPath);

        std::wstring selName = L"All Mice";
        if (g_selectedDevice) {
            for (auto& d : g_mouseDevices) if (d.hDevice == g_selectedDevice) selName = d.name;
        }
        RectF devTextRect = deviceBoxRect; devTextRect.X += 10; devTextRect.Width -= 40;
        graphics.DrawString(selName.c_str(), -1, &rowFont, devTextRect, &vertCenter, &valueBrush);

        float dChevX = deviceBoxRect.X + deviceBoxRect.Width - 15;
        float dChevY = deviceBoxRect.Y + deviceBoxRect.Height / 2;
        PointF dPts[3] = { {dChevX - chevSize / 2, dChevY - chevSize / 4}, {dChevX + chevSize / 2, dChevY - chevSize / 4}, {dChevX, dChevY + chevSize / 4} };
        graphics.FillPolygon(&accentBrush, dPts, 3);
        
        currentY += comboHeight;

