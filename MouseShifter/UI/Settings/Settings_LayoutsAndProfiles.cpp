        // --- Mouse Device Selector ---
        currentY += 30; 
        float br = 6.0f;
        StringFormat vertCenter; vertCenter.SetLineAlignment(StringAlignmentCenter);
        float chevSize = 8.0f;
        
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
        
        currentY += comboHeight + 50; 
        steeringDeviceComboRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + comboHeight };

        PointF steeringLabelPos((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top - 25 + settingsScrollOffset));
        graphics.DrawString(L"Steering Mouse Device:", -1, &rowFont, steeringLabelPos, &labelBrush);
        tooltips[39].bounds = steeringDeviceComboRect;
        tooltips[39].text = L"Select which mouse device controls steering (can be different from H-shifter mouse).";

        RectF steeringBoxRect((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top + settingsScrollOffset), (REAL)(steeringDeviceComboRect.right - steeringDeviceComboRect.left), (REAL)(steeringDeviceComboRect.bottom - steeringDeviceComboRect.top));
        bool isHoveredSteer = PtInRect(&steeringDeviceComboRect, itemAdjCursor);

        GraphicsPath steerPath;
        steerPath.AddArc(steeringBoxRect.X, steeringBoxRect.Y, br * 2, br * 2, 180, 90);
        steerPath.AddArc(steeringBoxRect.X + steeringBoxRect.Width - br * 2, steeringBoxRect.Y, br * 2, br * 2, 270, 90);
        steerPath.AddArc(steeringBoxRect.X + steeringBoxRect.Width - br * 2, steeringBoxRect.Y + steeringBoxRect.Height - br * 2, br * 2, br * 2, 0, 90);
        steerPath.AddArc(steeringBoxRect.X, steeringBoxRect.Y + steeringBoxRect.Height - br * 2, br * 2, br * 2, 90, 90);
        steerPath.CloseFigure();

        graphics.FillPath(isHoveredSteer ? &highlightBrush : &darkBrush, &steerPath);
        graphics.DrawPath(&accentPen, &steerPath);

        std::wstring selNameSteer = L"All Mice";
        if (g_selectedSteeringDevice) {
            for (auto& d : g_mouseDevices) if (d.hDevice == g_selectedSteeringDevice) selNameSteer = d.name;
        }
        RectF steerTextRect = steeringBoxRect; steerTextRect.X += 10; steerTextRect.Width -= 40;
        graphics.DrawString(selNameSteer.c_str(), -1, &rowFont, steerTextRect, &vertCenter, &valueBrush);

        float sChevX = steeringBoxRect.X + steeringBoxRect.Width - 15;
        float sChevY = steeringBoxRect.Y + steeringBoxRect.Height / 2;
        PointF sPts[3] = { {sChevX - chevSize / 2, sChevY - chevSize / 4}, {sChevX + chevSize / 2, sChevY - chevSize / 4}, {sChevX, sChevY + chevSize / 4} };
        graphics.FillPolygon(&accentBrush, sPts, 3);
        
        currentY += comboHeight;

