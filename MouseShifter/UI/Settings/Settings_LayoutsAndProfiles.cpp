        // --- H-Shifter Layout ---
        // Start right after reverseLockToggleRect
        int hShifterLayoutY = reverseLockToggleRect.bottom + 40;
        hShifterLayoutButtonRect = { settingsPanelRect.left + 20, hShifterLayoutY, settingsPanelRect.right - 20, hShifterLayoutY + toggleHeight };

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
        SolidBrush layoutBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&layoutBoxBrush, hShifterLayoutBoxRect);
        Pen layoutBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&layoutBoxPen, hShifterLayoutBoxRect);

        std::wstring hShifterLayoutName = hShifterLayouts[currentHShifterLayout - 1].name;
        PointF hShifterLayoutValuePos(hShifterLayoutBoxRect.X + 4, hShifterLayoutBoxRect.Y + 4);
        graphics.DrawString(hShifterLayoutName.c_str(), -1, &rowFont, hShifterLayoutValuePos, &valueBrush);

        PointF hShifterArrowPos(hShifterLayoutBoxRect.X + (hShifterLayoutBoxRect.Width - 20), hShifterLayoutBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, hShifterArrowPos, &valueBrush);

        // --- Gear Layout Cycle Button ---
        const int spacingAfterHShifter = 50;
        int layoutButtonY = hShifterLayoutY + toggleHeight + spacingAfterHShifter;
        gearLayoutButtonRect = { settingsPanelRect.left + 20, layoutButtonY, settingsPanelRect.right - 20, layoutButtonY + toggleHeight };

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
        graphics.FillRectangle(&layoutBoxBrush, layoutBoxRect);
        graphics.DrawRectangle(&layoutBoxPen, layoutBoxRect);

        std::wstring layoutName = gearLayoutNames[currentGearLayout];
        PointF layoutValuePos(layoutBoxRect.X + 4, layoutBoxRect.Y + 4);
        graphics.DrawString(layoutName.c_str(), -1, &rowFont, layoutValuePos, &valueBrush);

        PointF arrowPos(layoutBoxRect.X + (layoutBoxRect.Width - 20), layoutBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, arrowPos, &valueBrush);
        // --- Profile Selector ---
        const int spacingAfterGearLayout = 50; // Updated spacing as requested
        int profileSelectorY = layoutButtonY + toggleHeight + spacingAfterGearLayout;
        profileButtonRect = { settingsPanelRect.left + 20, profileSelectorY, settingsPanelRect.right - 20, profileSelectorY + toggleHeight };

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
        SolidBrush profileBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&profileBoxBrush, profileBoxRect);
        Pen profileBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&profileBoxPen, profileBoxRect);

        // Display current profile name or new profile name being typed
        std::string currentProfileDisplay;
        if (creatingNewProfile) {
            currentProfileDisplay = newProfileName; // Show typing in dropdown box
        }
        else if (!profileNames.empty()) {
            currentProfileDisplay = profileNames[currentProfileIndex];
            // Remove .ini extension for display
            if (currentProfileDisplay.size() > 4 && currentProfileDisplay.substr(currentProfileDisplay.size() - 4) == ".ini") {
                currentProfileDisplay = currentProfileDisplay.substr(0, currentProfileDisplay.size() - 4);
            }
        }
        else {
            currentProfileDisplay = "No Profiles";
        }

        std::wstring profileDisplayW(currentProfileDisplay.begin(), currentProfileDisplay.end());
        PointF profileValuePos(profileBoxRect.X + 4, profileBoxRect.Y + 4);

        if (creatingNewProfile) {
            // Draw the text input field with selection in the DROPDOWN BOX
            std::wstring profileNameW(newProfileName.begin(), newProfileName.end());

            // Use StringFormat for consistent text rendering
            StringFormat format;
            format.SetAlignment(StringAlignmentNear);
            format.SetLineAlignment(StringAlignmentNear);
            format.SetFormatFlags(StringFormatFlagsNoClip | StringFormatFlagsNoWrap);

            // Measure the entire string for height
            RectF fullTextBounds;
            graphics.MeasureString(profileNameW.c_str(), -1, &rowFont, PointF(0, 0), &format, &fullTextBounds);

            // Draw selection background if text is selected
            if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                // Measure text before selection using CharacterRanges for precise measurement
                CharacterRange ranges[2];
                ranges[0] = CharacterRange(0, profileTextSelectionStart);
                ranges[1] = CharacterRange(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);

                StringFormat measureFormat;
                measureFormat.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces);
                measureFormat.SetMeasurableCharacterRanges(2, ranges);

                RectF measureRect(0, 0, 10000, 10000); // Large enough rectangle for measurement
                Region regions[2];
                graphics.MeasureCharacterRanges(profileNameW.c_str(), -1, &rowFont, measureRect, &measureFormat, 2, regions);

                // Get bounds for selection region
                RectF selectionBounds;
                regions[1].GetBounds(&selectionBounds, &graphics);

                // Draw selection highlight at precise position
                RectF selectionRect(
                    profileBoxRect.X + 4 + selectionBounds.X,
                    profileBoxRect.Y + 4,
                    selectionBounds.Width,
                    fullTextBounds.Height
                );
                SolidBrush selectionBrush(Color(0, 120, 215));
                graphics.FillRectangle(&selectionBrush, selectionRect);
            }

            // Draw the text in DROPDOWN BOX
            SolidBrush textBrush(Color(255, 255, 255));
            graphics.DrawString(profileNameW.c_str(), -1, &rowFont, profileValuePos, &format, &textBrush);

            // Draw blinking cursor in DROPDOWN BOX if no selection
            if (!profileTextSelected || profileTextSelectionStart == profileTextSelectionEnd) {
                static DWORD lastBlink = GetTickCount();
                static bool cursorVisible = true;

                DWORD currentTime = GetTickCount();
                if (currentTime - lastBlink > 500) {
                    cursorVisible = !cursorVisible;
                    lastBlink = currentTime;
                }

                if (cursorVisible) {
                    // Calculate cursor position using precise CharacterRanges measurement
                    if (profileTextSelectionStart >= 0) {
                        CharacterRange range(0, profileTextSelectionStart);
                        StringFormat cursorFormat;
                        cursorFormat.SetFormatFlags(StringFormatFlagsMeasureTrailingSpaces);
                        cursorFormat.SetMeasurableCharacterRanges(1, &range);

                        RectF measureRect(0, 0, 10000, 10000);
                        Region regions[1];
                        graphics.MeasureCharacterRanges(profileNameW.c_str(), -1, &rowFont, measureRect, &cursorFormat, 1, regions);

                        RectF bounds;
                        if (regions[0].GetBounds(&bounds, &graphics) == Ok) {
                            REAL cursorX = profileBoxRect.X + 4 + bounds.GetRight();
                            REAL cursorY = profileBoxRect.Y + 4;
                            REAL cursorHeight = fullTextBounds.Height > 0 ? fullTextBounds.Height : rowFont.GetHeight(&graphics);

                            RectF cursorRect(cursorX, cursorY, 2, cursorHeight);
                            SolidBrush cursorBrush(Color(255, 255, 255));
                            graphics.FillRectangle(&cursorBrush, cursorRect);
                        }
                    }
                    else {
                        // Fallback for cursor at start (no text)
                        REAL cursorHeight = fullTextBounds.Height > 0 ? fullTextBounds.Height : rowFont.GetHeight(&graphics);
                        RectF cursorRect(profileBoxRect.X + 4, profileBoxRect.Y + 4, 2, cursorHeight);
                        SolidBrush cursorBrush(Color(255, 255, 255));
                        graphics.FillRectangle(&cursorBrush, cursorRect);
                    }
                }
            }
        }
        else {
            // Normal profile display in dropdown box
            graphics.DrawString(profileDisplayW.c_str(), -1, &rowFont, profileValuePos, &valueBrush);
        }

        PointF profileArrowPos(profileBoxRect.X + (profileBoxRect.Width - 20), profileBoxRect.Y + 4);
        graphics.DrawString(L"▼", -1, &rowFont, profileArrowPos, &valueBrush);



        // --- Create New Profile Button (separate button below dropdown) ---
        int createProfileButtonY = profileSelectorY + toggleHeight + 10; // 10px spacing after profile selector
        createProfileButtonRect = { settingsPanelRect.left + 20, createProfileButtonY, settingsPanelRect.right - 20, createProfileButtonY + toggleHeight };

        RectF createProfileBoxRect(
            (REAL)createProfileButtonRect.left,
            (REAL)(createProfileButtonRect.top + settingsScrollOffset),
            (REAL)(createProfileButtonRect.right - createProfileButtonRect.left),
            (REAL)createProfileButtonRect.bottom - createProfileButtonRect.top
        );

        // Different color for create button to make it stand out
        SolidBrush createProfileBoxBrush(Color(60, 60, 60));
        graphics.FillRectangle(&createProfileBoxBrush, createProfileBoxRect);
        Pen createProfileBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&createProfileBoxPen, createProfileBoxRect);

        // ALWAYS show instruction text in create button, never show the typing
        std::wstring createButtonText = creatingNewProfile ? L"Type profile name and press Enter" : L"Create New Profile";

        PointF createButtonTextPos(createProfileBoxRect.X + 4, createProfileBoxRect.Y + 4);
        graphics.DrawString(createButtonText.c_str(), -1, &rowFont, createButtonTextPos, &valueBrush);

        // Update the deviceComboY position to be after profile selector

        // --- Mouse Device Selector ---
// --- Mouse Device Selector ---
        SolidBrush deviceBoxBrush(Color(30, 30, 30));
        Pen deviceBoxPen(Color(0, 255, 136), 2);

        // Update the deviceComboY position to be after create profile button
        const int spacingAfterProfile = 50;
        int deviceComboY = createProfileButtonY + toggleHeight + spacingAfterProfile;
        deviceComboRect = { settingsPanelRect.left + 20, deviceComboY, settingsPanelRect.right - 20, deviceComboY + comboHeight };

        PointF deviceLabelPos((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"H-Shifter Mouse Device:", -1, &rowFont, deviceLabelPos, &labelBrush);
        tooltips[14].bounds = deviceComboRect;
        tooltips[14].text = L"Select which mouse device controls the H-shifter knob movement.";
        RectF deviceBoxRect((REAL)deviceComboRect.left, (REAL)(deviceComboRect.top + settingsScrollOffset),
            (REAL)(deviceComboRect.right - deviceComboRect.left), (REAL)(deviceComboRect.bottom - deviceComboRect.top));
        graphics.FillRectangle(&deviceBoxBrush, deviceBoxRect);
        graphics.DrawRectangle(&deviceBoxPen, deviceBoxRect);


        std::wstring selName = L"All Mice";
        if (g_selectedDevice)
        {
            for (auto& d : g_mouseDevices)
                if (d.hDevice == g_selectedDevice)
                    selName = d.name;
        }
        PointF selTextPos((REAL)deviceComboRect.left + 4, (REAL)deviceComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selName.c_str(), -1, &rowFont, selTextPos, &valueBrush);

