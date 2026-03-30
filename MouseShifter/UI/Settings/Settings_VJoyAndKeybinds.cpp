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

        // Dropdowns moved to Top Navigation Bar

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
        
        settingsScrollMax = max(0, lastControlBottom - (settingsPanelRect.bottom - settingsPanelRect.top));

        // Draw all tooltips from registry
        for (const auto& tooltip : tooltips) {
             DrawTooltip(graphics, tooltip, settingsScrollOffset);
        }
