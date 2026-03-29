        // --- Dropdowns (must be drawn last to overlap other controls) ---
        int listItemHeight = 30; // Slightly taller for premium feel
        int itemGap = 2;
        float menuRadius = 8.0f;

        auto DrawModernMenu = [&](RectF listRect, size_t itemCount, size_t currentIndex, int hoveredIndex, std::function<std::wstring(size_t)> getNameFunc) {
            GraphicsPath menuPath;
            menuPath.AddArc(listRect.X, listRect.Y, menuRadius * 2, menuRadius * 2, 180, 90);
            menuPath.AddArc(listRect.X + listRect.Width - menuRadius * 2, listRect.Y, menuRadius * 2, menuRadius * 2, 270, 90);
            menuPath.AddArc(listRect.X + listRect.Width - menuRadius * 2, listRect.Y + listRect.Height - menuRadius * 2, menuRadius * 2, menuRadius * 2, 0, 90);
            menuPath.AddArc(listRect.X, listRect.Y + listRect.Height - menuRadius * 2, menuRadius * 2, menuRadius * 2, 90, 90);
            menuPath.CloseFigure();

            SolidBrush menuBg(Color(240, 25, 25, 25)); // High opacity dark
            graphics.FillPath(&menuBg, &menuPath);
            graphics.DrawPath(&accentPen, &menuPath);

            for (size_t i = 0; i < itemCount; ++i) {
                RectF itemRect(listRect.X + 4, listRect.Y + 4 + i * (listItemHeight + itemGap), listRect.Width - 8, (REAL)listItemHeight);
                bool hovered = ((int)i == hoveredIndex);
                bool selected = (i == currentIndex);

                if (hovered || selected) {
                    GraphicsPath itemPath;
                    float ir = 4.0f;
                    itemPath.AddArc(itemRect.X, itemRect.Y, ir * 2, ir * 2, 180, 90);
                    itemPath.AddArc(itemRect.X + itemRect.Width - ir * 2, itemRect.Y, ir * 2, ir * 2, 270, 90);
                    itemPath.AddArc(itemRect.X + itemRect.Width - ir * 2, itemRect.Y + itemRect.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                    itemPath.AddArc(itemRect.X, itemRect.Y + itemRect.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                    itemPath.CloseFigure();
                    graphics.FillPath(selected ? &accentBrush : &highlightBrush, &itemPath);
                }

                std::wstring itemName = getNameFunc(i);
                StringFormat itemF; itemF.SetLineAlignment(StringAlignmentCenter);
                RectF textRect = itemRect; textRect.X += 10;
                graphics.DrawString(itemName.c_str(), -1, &rowFont, textRect, &itemF, selected ? &darkBrush : &valueBrush);
            }
        };

        if (gearLayoutDropdownOpen) {
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size() + 8;
            RectF listRect((REAL)gearLayoutButtonRect.left, (REAL)listY, (REAL)gearLayoutButtonRect.right - gearLayoutButtonRect.left, (REAL)totalHeight);
            DrawModernMenu(listRect, gearLayouts.size(), currentGearLayout, hoveredGearLayoutIndex, [&](size_t i) { return gearLayoutNames[i]; });
        }

        if (hShifterLayoutDropdownOpen) {
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size() + 8;
            RectF listRect((REAL)hShifterLayoutButtonRect.left, (REAL)listY, (REAL)hShifterLayoutButtonRect.right - hShifterLayoutButtonRect.left, (REAL)totalHeight);
            DrawModernMenu(listRect, hShifterLayouts.size(), currentHShifterLayout - 1, hoveredHShifterLayoutIndex, [&](size_t i) { return hShifterLayouts[i].name; });
        }

        if (profileDropdownOpen) {
            int listY = profileButtonRect.top + settingsScrollOffset + 35;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size() + 8;
            RectF listRect((REAL)profileButtonRect.left, (REAL)listY, (REAL)profileButtonRect.right - profileButtonRect.left, (REAL)totalHeight);
            
            // Draw Background and border
            GraphicsPath menuPath;
            float menuRadius = 8.0f;
            menuPath.AddArc(listRect.X, listRect.Y, menuRadius * 2, menuRadius * 2, 180, 90);
            menuPath.AddArc(listRect.X + listRect.Width - menuRadius * 2, listRect.Y, menuRadius * 2, menuRadius * 2, 270, 90);
            menuPath.AddArc(listRect.X + listRect.Width - menuRadius * 2, listRect.Y + listRect.Height - menuRadius * 2, menuRadius * 2, menuRadius * 2, 0, 90);
            menuPath.AddArc(listRect.X, listRect.Y + listRect.Height - menuRadius * 2, menuRadius * 2, menuRadius * 2, 90, 90);
            menuPath.CloseFigure();
            SolidBrush menuBg(Color(245, 20, 20, 20));
            graphics.FillPath(&menuBg, &menuPath);
            graphics.DrawPath(&accentPen, &menuPath);

            for (size_t i = 0; i < profileNames.size(); ++i) {
                RectF itemRect(listRect.X + 4, listRect.Y + 4 + i * (listItemHeight + itemGap), listRect.Width - 8, (REAL)listItemHeight);
                bool hovered = ((int)i == hoveredProfileIndex);
                bool selected = (i == (size_t)currentProfileIndex);

                if (hovered || selected) {
                    GraphicsPath itemPath;
                    float ir = 4.0f;
                    itemPath.AddArc(itemRect.X, itemRect.Y, ir * 2, ir * 2, 180, 90);
                    itemPath.AddArc(itemRect.X + itemRect.Width - ir * 2, itemRect.Y, ir * 2, ir * 2, 270, 90);
                    itemPath.AddArc(itemRect.X + itemRect.Width - ir * 2, itemRect.Y + itemRect.Height - ir * 2, ir * 2, ir * 2, 0, 90);
                    itemPath.AddArc(itemRect.X, itemRect.Y + itemRect.Height - ir * 2, ir * 2, ir * 2, 90, 90);
                    itemPath.CloseFigure();
                    graphics.FillPath(selected ? &accentBrush : &highlightBrush, &itemPath);
                }

                std::string p = profileNames[i];
                if (p.size() > 4 && p.substr(p.size() - 4) == ".ini") p = p.substr(0, p.size() - 4);
                std::wstring itemName(p.begin(), p.end());
                
                StringFormat itemF; itemF.SetLineAlignment(StringAlignmentCenter);
                RectF textRect = itemRect; textRect.X += 10; textRect.Width -= 60; // leave room for icons
                graphics.DrawString(itemName.c_str(), -1, &rowFont, textRect, &itemF, selected ? &darkBrush : &valueBrush);

                // --- Action Icons (Clone & Delete) ---
                float iconSize = 18.0f;
                float iconY = itemRect.Y + (itemRect.Height - iconSize) / 2;
                
                // Clone Index (Duplicate)
                RectF cloneIconRect(itemRect.X + itemRect.Width - 50, iconY, iconSize, iconSize);
                bool cloneHover = (hoveredCloneIndex == (int)i);
                graphics.DrawString(L"\x2302", -1, &rowFont, cloneIconRect, &itemF, cloneHover ? &accentBrush : &labelBrush); // Box icon as clone placeholder or ❐ (\x2750) 
                
                // Delete Icon
                RectF deleteIconRect(itemRect.X + itemRect.Width - 25, iconY, iconSize, iconSize);
                bool deleteHover = (hoveredDeleteIndex == (int)i);
                static SolidBrush redBrush(Color(255, 80, 80));
                graphics.DrawString(L"\x2715", -1, &rowFont, deleteIconRect, &itemF, deleteHover ? &redBrush : &labelBrush); // X symbol
            }
        }

        // --- Tooltips (Must be drawn after dropdowns) ---
        if (g_showTooltip) {
            std::wstring tooltipText = L"Prevents in-game mouse look & right stick while using H-shifter.\n\n• Blocks mouse input to selected game\n• Blocks controller right stick (XInput)\n• Hold RMB to temporarily use mouse\n• Knob Assist Button affects R stick blocking\n• Disable Knob to restore normal controls\n\n Experimental: PlayStation controllers may have issues (DS4Windows)";
            Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
            SolidBrush tooltipTextBrush(Color(255, 255, 255));
            StringFormat tooltipFormat;
            tooltipFormat.SetAlignment(StringAlignmentNear);
            tooltipFormat.SetLineAlignment(StringAlignmentNear);

            RectF textBounds;
            graphics.MeasureString(tooltipText.c_str(), -1, &tooltipFont, RectF(0, 0, 250, 500), &tooltipFormat, &textBounds);
            RectF tooltipTextRect((REAL)processComboRect.left, (REAL)(processComboRect.top - 40 + settingsScrollOffset), (REAL)270, textBounds.Height + 20);

            SolidBrush tooltipBgBrush(Color(30, 30, 30));
            graphics.FillRectangle(&tooltipBgBrush, tooltipTextRect);
            graphics.DrawRectangle(&accentPen, tooltipTextRect);
            graphics.DrawString(tooltipText.c_str(), -1, &tooltipFont, RectF(tooltipTextRect.X + 8, tooltipTextRect.Y + 8, tooltipTextRect.Width - 16, tooltipTextRect.Height - 16), &tooltipFormat, &tooltipTextBrush);
        }

        // --- Final Scroll Range Calculation ---
        int lastControlBottom = currentY + 50; 
        if (gearLayoutDropdownOpen) lastControlBottom = max(lastControlBottom, (int)(gearLayoutButtonRect.bottom + 30 * gearLayouts.size()));
        if (hShifterLayoutDropdownOpen) lastControlBottom = max(lastControlBottom, (int)(hShifterLayoutButtonRect.bottom + 30 * hShifterLayouts.size()));
        if (profileDropdownOpen) lastControlBottom = max(lastControlBottom, (int)(profileButtonRect.bottom + 30 * profileNames.size()));
        
        settingsScrollMax = max(0, lastControlBottom - (settingsPanelRect.bottom - settingsPanelRect.top));

        // Draw all tooltips from registry
        for (const auto& tooltip : tooltips) {
             DrawTooltip(graphics, tooltip, settingsScrollOffset);
        }
