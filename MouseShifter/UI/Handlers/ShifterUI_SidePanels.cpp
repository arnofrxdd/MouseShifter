    if (showKeybindPanel || showInputPanel || showTogglePanel)
    {
        using namespace Gdiplus;

        // --- Clip drawing to panel area ---
        graphics.SetClip(panelRect);

        FontFamily fontFamily(L"Segoe UI");
        Font titleFont(&fontFamily, 22, FontStyleBold, UnitPixel);
        Font subtitleFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
        Font rowFont(&fontFamily, 16, FontStyleRegular, UnitPixel);

        SolidBrush titleBrush(Color(0, 255, 136));
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
            // Title
            RectF titleRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, 30);
            graphics.DrawString(L"Key Bindings", -1, &titleFont, titleRectF, &centerAlign, &titleBrush);

            y += 32;

            // Subtitle
            RectF subtitleRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, 20);
            graphics.DrawString(L"Shift+LMB to pick vJoy buttons", -1, &subtitleFont, subtitleRectF, &centerAlign, &subtitleBrush);

            y += 30;

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
                if (input.type == KEYBOARD)
                {
                    char buffer[64];
                    if (GetKeyNameTextA(MapVirtualKey(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyNameStr = buffer;
                }
                else if (input.type == MOUSE)
                {
                    switch (input.code)
                    {
                    case 1:
                        keyNameStr = "LMB";
                        break;
                    case 2:
                        keyNameStr = "RMB";
                        break;
                    case 3:
                        keyNameStr = "MMB";
                        break;
                    default:
                        keyNameStr = "Mouse " + std::to_string(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON)
                {
                    keyNameStr = "vJoy Btn " + std::to_string(input.code);
                }

                Color textColor = (keybindBeingSet == gear) ? Color(255, 220, 0) : Color(220, 220, 220);
                SolidBrush textBrushRow(textColor);

                // Check if this gear has an active glow animation
                float glowAlpha = 0.0f;
                if (keybindAnimations.find(gear) != keybindAnimations.end()) {
                    glowAlpha = keybindAnimations[gear].glowAlpha;
                }

                // Base row color
                Color rowColor = (keybindBeingSet == gear) ? Color(40, 40, 0) : Color(36, 36, 36);

                // Apply glow effect if active
                if (glowAlpha > 0.0f) {
                    // Create green glow colors (same as title brush: Color(0, 255, 136))
                    Color innerGlow(0, 255, 136, (int)(glowAlpha * 180));  // Green inner glow
                    Color outerGlow(0, 200, 100, (int)(glowAlpha * 80));   // Slightly darker green outer glow

                    // Draw outer subtle glow
                    SolidBrush outerGlowBrush(outerGlow);
                    RectF outerGlowRect((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, (REAL)rowHeight);
                    graphics.FillRectangle(&outerGlowBrush, outerGlowRect);

                    // Draw inner strong glow
                    SolidBrush innerGlowBrush(innerGlow);
                    RectF innerGlowRect((REAL)panelRect.X + 1, (REAL)y + 1, (REAL)panelWidth - 2, (REAL)rowHeight - 2);
                    graphics.FillRectangle(&innerGlowBrush, innerGlowRect);

                    // Make the row slightly brighter during glow with green tint
                    rowColor = Color(
                        min(255, rowColor.GetR() + (int)(20 * glowAlpha)),  // Less red
                        min(255, rowColor.GetG() + (int)(40 * glowAlpha)),  // More green
                        min(255, rowColor.GetB() + (int)(10 * glowAlpha))   // Slight blue
                    );
                }

                SolidBrush rowBrush(rowColor);

                // Remove border by using the same rect without +2/-4 adjustments
                RectF rowRectF((REAL)panelRect.X, (REAL)y, (REAL)panelWidth, (REAL)rowHeight);
                graphics.FillRectangle(&rowBrush, rowRectF);

                RectF gearRectF((REAL)panelRect.X + 10, (REAL)(y + rowPadding / 2), 50.0f, (REAL)(rowHeight - rowPadding));
                StringFormat leftAlign;
                leftAlign.SetAlignment(StringAlignmentNear);
                leftAlign.SetLineAlignment(StringAlignmentCenter);
                std::wstring gearW(gear.begin(), gear.end());
                graphics.DrawString(gearW.c_str(), -1, &rowFont, gearRectF, &leftAlign, &textBrushRow);

                std::string displayKey = keyNameStr;
                if (input.type == MOUSE)
                    displayKey += " (Mouse)";
                else if (input.type == KEYBOARD)
                    displayKey += " (Keyboard)";
                else if (input.type == VJOY_BUTTON)
                    displayKey += " (vJoy)";

                std::wstring keyW(displayKey.begin(), displayKey.end());
                RectF keyRectF((REAL)panelRect.X + 80, (REAL)(y + rowPadding / 2), (REAL)panelWidth - 90, (REAL)(rowHeight - rowPadding));
                graphics.DrawString(keyW.c_str(), -1, &rowFont, keyRectF, &leftAlign, &textBrushRow);

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
            graphics.DrawString(L"Keys & Mouse → vJoy (Shifter Togglers)", -1, &subtitleFont, subtitleRect, &leftAlign, &subtitleBrush);

            yInput += 28;

            StringFormat rowTextFormat;
            rowTextFormat.SetAlignment(StringAlignmentNear);
            rowTextFormat.SetLineAlignment(StringAlignmentCenter);

            for (size_t i = 0; i < inputMap.size(); ++i)
            {
                InputToVJoy& input = inputMap[i];
                std::wstring keyName;

                // Determine key name
                if (input.type == KEYBOARD)
                {
                    wchar_t buffer[64];
                    if (GetKeyNameTextW(MapVirtualKeyW(input.code, MAPVK_VK_TO_VSC) << 16, buffer, 64) > 0)
                        keyName = buffer;
                }
                else if (input.type == MOUSE)
                {
                    switch (input.code)
                    {
                    case 1:
                        keyName = L"LMB";
                        break;
                    case 2:
                        keyName = L"RMB";
                        break;
                    case 3:
                        keyName = L"MMB";
                        break;
                    default:
                        keyName = L"Mouse " + std::to_wstring(input.code);
                    }
                }
                else if (input.type == VJOY_BUTTON)
                {
                    keyName = L"vJoy Btn " + std::to_wstring(input.code);
                }

                // Row background
                Color rowColor = (inputBeingSet == std::to_string(i)) ? Color(0, 255, 136) : Color(36, 36, 36);
                SolidBrush rowBrush(rowColor);
                RectF rowRect((REAL)inputPanelRectUnified.X + leftMargin, (REAL)yInput, (REAL)keyColumnWidth, (REAL)rowHeight);
                graphics.FillRectangle(&rowBrush, rowRect);

                // Combined text centered
                std::wstring combinedText = keyName + L" → vJoy " + std::to_wstring(input.vjoyButton);
                StringFormat centerFormat;
                centerFormat.SetAlignment(StringAlignmentCenter);
                centerFormat.SetLineAlignment(StringAlignmentCenter);

                graphics.DrawString(combinedText.c_str(), -1, &rowFont, rowRect, &centerFormat, &textBrush);

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

            // Knob Toggle
            RectF knobSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Activate Knob Key", -1, &subtitleFont, knobSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush1((toggleInputBeingSet || togglePedalBeingSet) ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect1F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush1, keyRect1F);

            g_toggleKeyRect.left = (LONG)keyRect1F.X;
            g_toggleKeyRect.top = (LONG)keyRect1F.Y;
            g_toggleKeyRect.right = (LONG)(keyRect1F.X + keyRect1F.Width);
            g_toggleKeyRect.bottom = (LONG)(keyRect1F.Y + keyRect1F.Height);

            wchar_t keyName1[64] = L"";
            if (toggleInputBeingSet)
            {
                wcscpy_s(keyName1, L"Press any key...");
            }
            else if (togglePedalBeingSet)
            {
                wcscpy_s(keyName1, L"Press a pedal...");
            }
            else
            {
                switch (g_knobToggleType)
                {
                case TOGGLE_KEYBOARD:
                    if (g_knobToggleKey != 0 &&
                        GetKeyNameTextW(MapVirtualKeyW(g_knobToggleKey, MAPVK_VK_TO_VSC) << 16,
                            keyName1, 64) == 0)
                    {
                        wcscpy_s(keyName1, L"Unknown Key");
                    }
                    else if (g_knobToggleKey == 0)
                    {
                        wcscpy_s(keyName1, L"Not Set");
                    }
                    break;

                    // ✅ Add mouse buttons
                case TOGGLE_MOUSE_LEFT:
                    wcscpy_s(keyName1, L"Mouse: Left Button");
                    break;
                case TOGGLE_MOUSE_RIGHT:
                    wcscpy_s(keyName1, L"Mouse: Right Button");
                    break;
                case TOGGLE_MOUSE_MIDDLE:
                    wcscpy_s(keyName1, L"Mouse: Middle Button");
                    break;
                case TOGGLE_MOUSE_BUTTON4:
                    wcscpy_s(keyName1, L"Mouse: Button 4");
                    break;
                case TOGGLE_MOUSE_BUTTON5:
                    wcscpy_s(keyName1, L"Mouse: Button 5");
                    break;

                    // Existing pedals
                case TOGGLE_PEDAL_CLUTCH:
                    wcscpy_s(keyName1, L"Pedal: Clutch");
                    break;
                case TOGGLE_PEDAL_BRAKE:
                    wcscpy_s(keyName1, L"Pedal: Brake");
                    break;
                case TOGGLE_PEDAL_ACCEL:
                    wcscpy_s(keyName1, L"Pedal: Accelerator");
                    break;
                }
            }



            graphics.DrawString(keyName1, -1, &rowFont, keyRect1F, &centerAlign, &textBrush);

            yToggle += rowHeight + rowSpacing;

            // Assist Button
            RectF assistSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Knob Assist Button (Controller)", -1, &subtitleFont, assistSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush2(assistButtonBeingSet ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect2F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush2, keyRect2F);

            g_assistButtonRect.left = (LONG)keyRect2F.X;
            g_assistButtonRect.top = (LONG)keyRect2F.Y;
            g_assistButtonRect.right = (LONG)(keyRect2F.X + keyRect2F.Width);
            g_assistButtonRect.bottom = (LONG)(keyRect2F.Y + keyRect2F.Height);

            wchar_t keyName2[64] = L"";

            if (assistButtonBeingSet) {
                wcscpy_s(keyName2, L"Press any button...");
            }
            else if (g) {
                switch ((SDL_GameControllerButton)assistButton) {
                case SDL_CONTROLLER_BUTTON_A:        wcscpy_s(keyName2, L"A Button"); break;
                case SDL_CONTROLLER_BUTTON_B:        wcscpy_s(keyName2, L"B Button"); break;
                case SDL_CONTROLLER_BUTTON_X:        wcscpy_s(keyName2, L"X Button"); break;
                case SDL_CONTROLLER_BUTTON_Y:        wcscpy_s(keyName2, L"Y Button"); break;
                case SDL_CONTROLLER_BUTTON_BACK:     wcscpy_s(keyName2, L"Back"); break;
                case SDL_CONTROLLER_BUTTON_GUIDE:    wcscpy_s(keyName2, L"Guide"); break;
                case SDL_CONTROLLER_BUTTON_START:    wcscpy_s(keyName2, L"Start"); break;
                case SDL_CONTROLLER_BUTTON_LEFTSTICK: wcscpy_s(keyName2, L"L3"); break;
                case SDL_CONTROLLER_BUTTON_RIGHTSTICK: wcscpy_s(keyName2, L"R3"); break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  wcscpy_s(keyName2, L"LB / L1"); break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: wcscpy_s(keyName2, L"RB / R1"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:    wcscpy_s(keyName2, L"DPad Up"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  wcscpy_s(keyName2, L"DPad Down"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  wcscpy_s(keyName2, L"DPad Left"); break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: wcscpy_s(keyName2, L"DPad Right"); break;
                default:                               wcscpy_s(keyName2, L"Unknown Button"); break;
                }
            }
            else {
                wcscpy_s(keyName2, L"None");
            }

            graphics.DrawString(keyName2, -1, &rowFont, keyRect2F, &centerAlign, &textBrush);

            yToggle += rowHeight + rowSpacing;

            // Reverse Unlock Button
            RectF reverseSubtitleRect((REAL)togglePanelRectUnified.X, (REAL)yToggle, (REAL)rectWidth, 20);
            graphics.DrawString(L"Reverse Unlock Button", -1, &subtitleFont, reverseSubtitleRect, &leftAlign, &subtitleBrush);

            yToggle += 25;

            SolidBrush rowBrush3(reverseUnlockBeingSet ? Color(0, 255, 136) : Color(36, 36, 36));
            RectF keyRect3F((REAL)togglePanelRectUnified.X + leftMargin, (REAL)yToggle, (REAL)keyColumnWidth, (REAL)rowHeight);
            graphics.FillRectangle(&rowBrush3, keyRect3F);

            reverseUnlockKeyRect.left = (LONG)keyRect3F.X;
            reverseUnlockKeyRect.top = (LONG)keyRect3F.Y;
            reverseUnlockKeyRect.right = (LONG)(keyRect3F.X + keyRect3F.Width);
            reverseUnlockKeyRect.bottom = (LONG)(keyRect3F.Y + keyRect3F.Height);

            wchar_t keyName3[64] = L"";

            if (reverseUnlockBeingSet) {
                wcscpy_s(keyName3, L"Press any key/button...");
            }
            else {
                switch (g_reverseUnlockType)
                {
                case TOGGLE_KEYBOARD:
                    if (g_reverseUnlockKey != 0 &&
                        GetKeyNameTextW(MapVirtualKeyW(g_reverseUnlockKey, MAPVK_VK_TO_VSC) << 16,
                            keyName3, 64) == 0)
                    {
                        wcscpy_s(keyName3, L"Unknown Key");
                    }
                    else if (g_reverseUnlockKey == 0)
                    {
                        wcscpy_s(keyName3, L"Not Set");
                    }
                    break;
                case TOGGLE_MOUSE_LEFT: wcscpy_s(keyName3, L"Mouse: Left Button"); break;
                case TOGGLE_MOUSE_RIGHT: wcscpy_s(keyName3, L"Mouse: Right Button"); break;
                case TOGGLE_MOUSE_MIDDLE: wcscpy_s(keyName3, L"Mouse: Middle Button"); break;
                case TOGGLE_MOUSE_BUTTON4: wcscpy_s(keyName3, L"Mouse: Button 4"); break;
                case TOGGLE_MOUSE_BUTTON5: wcscpy_s(keyName3, L"Mouse: Button 5"); break;
                case TOGGLE_PEDAL_CLUTCH: wcscpy_s(keyName3, L"Pedal: Clutch"); break;
                case TOGGLE_PEDAL_BRAKE: wcscpy_s(keyName3, L"Pedal: Brake"); break;
                case TOGGLE_PEDAL_ACCEL: wcscpy_s(keyName3, L"Pedal: Accelerator"); break;
                }
            }

            graphics.DrawString(keyName3, -1, &rowFont, keyRect3F, &centerAlign, &textBrush);
            yToggle += rowHeight + rowSpacing;
            rightPanelContentBottom = max(rightPanelContentBottom, yToggle);
        }

        // --- Update max scroll for right panel ---
        // Add 50px extra padding at the bottom so the last items aren't stuck at the bottom edge
        int bottomPadding = 50;
        rightPanelScrollMax = max(0, (rightPanelContentBottom + bottomPadding - rightPanelScrollOffset) - (panelRect.Y + panelRect.Height - 10));

        graphics.ResetClip();
    }

