        // --- Use LT as Clutch Toggle ---
        int useLTAsClutchY = invertAssistToggleRect.bottom + 20; // space after previous toggle
        useLTAsClutchToggleRect = { settingsPanelRect.left + 20, useLTAsClutchY, settingsPanelRect.right - 20, useLTAsClutchY + toggleHeight };
        PointF useLTAsClutchLabelPos((REAL)useLTAsClutchToggleRect.left, (REAL)useLTAsClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use LT/L2 as Clutch", -1, &rowFont, useLTAsClutchLabelPos, &labelBrush);
        // Use LT/L2 as Clutch
        tooltips[47].bounds = useLTAsClutchToggleRect;
        tooltips[47].text = L"Uses LT/L2 trigger as clutch input instead of scroll wheel.";
        // Checkbox
        RectF useLTAsClutchCheckboxRect(useLTAsClutchToggleRect.left + 180, useLTAsClutchToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush useLTAsClutchBoxBrush(useLTAsClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&useLTAsClutchBoxBrush, useLTAsClutchCheckboxRect);
        graphics.DrawRectangle(&boxPen, useLTAsClutchCheckboxRect);
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = gearLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * gearLayouts.size();
            RectF listRect((REAL)gearLayoutButtonRect.left, (REAL)listY, (REAL)gearLayoutButtonRect.right - gearLayoutButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&layoutBoxPen, listRect);

            for (size_t i = 0; i < gearLayouts.size(); ++i)
            {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight (traditional green)
                if ((int)i == hoveredGearLayoutIndex)
                {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Traditional green
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected item highlight
                if (i == currentGearLayout)
                {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::wstring itemName = gearLayoutNames[i];
                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(itemName.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size();
            RectF listRect((REAL)hShifterLayoutButtonRect.left, (REAL)listY, (REAL)hShifterLayoutButtonRect.right - hShifterLayoutButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&layoutBoxPen, listRect);

            for (size_t i = 0; i < hShifterLayouts.size(); ++i)
            {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight (traditional green)
                if ((int)i == hoveredHShifterLayoutIndex)
                {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Traditional green
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected item highlight
                if ((i + 1) == currentHShifterLayout)
                {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::wstring itemName = hShifterLayouts[i].name;
                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(itemName.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }
        if (profileDropdownOpen) {
            int listItemHeight = 25;
            int itemGap = 5;
            int listY = profileButtonRect.top + settingsScrollOffset + 25;
            int totalHeight = (listItemHeight + itemGap) * profileNames.size();
            RectF listRect((REAL)profileButtonRect.left, (REAL)listY, (REAL)profileButtonRect.right - profileButtonRect.left, (REAL)totalHeight);

            SolidBrush listBrush(Color(30, 30, 30));
            graphics.FillRectangle(&listBrush, listRect);
            graphics.DrawRectangle(&profileBoxPen, listRect);

            for (size_t i = 0; i < profileNames.size(); ++i) {
                int itemY = listY + i * (listItemHeight + itemGap);
                RectF itemRect((REAL)listRect.X, (REAL)itemY, (REAL)listRect.Width, (REAL)listItemHeight);

                // Hover highlight
                if ((int)i == hoveredProfileIndex) {
                    SolidBrush hoverBrush(Color(40, 100, 255, 100)); // Same green as other dropdowns
                    graphics.FillRectangle(&hoverBrush, itemRect);
                }

                // Selected profile highlight
                if ((int)i == currentProfileIndex) {
                    SolidBrush highlightBrush(Color(80, 100, 255, 100)); // Same color, more opaque
                    graphics.FillRectangle(&highlightBrush, itemRect);
                }

                std::string profileName = profileNames[i];
                if (profileName.size() > 4 && profileName.substr(profileName.size() - 4) == ".ini") {
                    profileName = profileName.substr(0, profileName.size() - 4);
                }
                std::wstring profileNameW(profileName.begin(), profileName.end());

                PointF itemPos(itemRect.X + 8, itemRect.Y + 4);
                graphics.DrawString(profileNameW.c_str(), -1, &rowFont, itemPos, &valueBrush);
            }
        }
        // --- Update max scroll ---
// --- Update max scroll ---
        if (g_showTooltip) {
            std::wstring tooltipText = L"Prevents in-game mouse look & right stick while using H-shifter.\n\n• Blocks mouse input to selected game\n• Blocks controller right stick (XInput)\n• Hold RMB to temporarily use mouse\n• Knob Assist Button affects R stick blocking\n• Disable Knob to restore normal controls\n\n Experimental: PlayStation controllers may have issues (DS4Windows)";            Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
            SolidBrush tooltipTextBrush(Color(255, 255, 255));

            // Set up format WITH word wrapping
            StringFormat tooltipFormat;
            tooltipFormat.SetAlignment(StringAlignmentNear);
            tooltipFormat.SetLineAlignment(StringAlignmentNear);
            // Remove StringFormatFlagsNoWrap to enable word wrapping

            // Measure text with wrapping
            RectF textBounds;
            graphics.MeasureString(tooltipText.c_str(), -1, &tooltipFont, RectF(0, 0, 250, 500), &tooltipFormat, &textBounds);

            // Create tooltip rectangle
            RectF tooltipTextRect(
                (REAL)processComboRect.left,
                (REAL)(processComboRect.top - 40 + settingsScrollOffset),
                (REAL)270,
                textBounds.Height + 20
            );

            // Draw background and border
            SolidBrush tooltipBgBrush(Color(30, 30, 30));
            graphics.FillRectangle(&tooltipBgBrush, tooltipTextRect);
            graphics.DrawRectangle(&layoutBoxPen, tooltipTextRect);

            // Draw text with word wrapping
            graphics.DrawString(
                tooltipText.c_str(),
                -1,
                &tooltipFont,
                RectF(tooltipTextRect.X + 8, tooltipTextRect.Y + 8, tooltipTextRect.Width - 16, tooltipTextRect.Height - 16),
                &tooltipFormat,
                &tooltipTextBrush
            );
        }
        // Find the bottom-most control: transparency slider is the last one
        int lastControlBottom = useLTAsClutchToggleRect.bottom; // now last toggle
// Update scroll max to account for dropdowns
        if (gearLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int layoutListBottom = (int)(gearLayoutButtonRect.bottom + listItemHeight * gearLayouts.size());
            lastControlBottom = max(lastControlBottom, layoutListBottom + 20);
        }

        if (hShifterLayoutDropdownOpen)
        {
            int listItemHeight = 25;
            int layoutListBottom = (int)(hShifterLayoutButtonRect.bottom + listItemHeight * hShifterLayouts.size());
            lastControlBottom = max(lastControlBottom, layoutListBottom + 20);
        }
        if (profileDropdownOpen) {
            int listItemHeight = 25;
            int profileListBottom = (int)(profileButtonRect.bottom + listItemHeight * profileNames.size());
            lastControlBottom = max(lastControlBottom, profileListBottom + 20);
        }
        settingsScrollMax = max(0, lastControlBottom - (settingsPanelRect.bottom - settingsPanelRect.top));

        for (const auto& tooltip : tooltips) {
            // Special case for sliders only (indices 0-6, 14-16, 19-21, 27, 32) - draw 25px below
            if ((&tooltip >= &tooltips[0] && &tooltip <= &tooltips[6]) ||    // First 7 sliders
                (&tooltip >= &tooltips[16] && &tooltip <= &tooltips[18]) ||  // Transparency sliders
                (&tooltip >= &tooltips[21] && &tooltip <= &tooltips[23]) ||
                (&tooltip >= &tooltips[25] && &tooltip <= &tooltips[26]) ||
                (&tooltip >= &tooltips[28] && &tooltip <= &tooltips[29]) ||// Steering sensitivity sliders
                (&tooltip >= &tooltips[30] && &tooltip <= &tooltips[30]) ||
                (&tooltip >= &tooltips[37] && &tooltip <= &tooltips[36]) ||
                // Scroll sensitivity slider
                &tooltip == &tooltips[42]) {                                 // Controller sensitivity slider
                DrawTooltip(graphics, tooltip, settingsScrollOffset, 25);
            }
            // Special handling for inverted scroll toggle to position it above
            else if (&tooltip == &tooltips[47]) {
                DrawTooltip(graphics, tooltip, settingsScrollOffset, -85); // Position above instead of below
            }
            else {
                // All other tooltips (toggles, buttons, dropdowns) use default offset (25px)
                DrawTooltip(graphics, tooltip, settingsScrollOffset);
            }
        }
        // --- Reset clip ---
