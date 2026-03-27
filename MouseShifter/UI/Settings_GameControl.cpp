        // --- Game Control Panel ---
        // Mouse Block / Process Selector
        const int gameControlSpacing = 40;
        int gameControlY = bottomLineY + gameControlSpacing;
        int processComboY = gameControlY + 25;

        processComboRect = { settingsPanelRect.left + 20, processComboY, settingsPanelRect.right - 20, processComboY + comboHeight };
        PointF processLabelPos((REAL)processComboRect.left, (REAL)(processComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"Block Mouse & Look:", -1, &rowFont, processLabelPos, &labelBrush);


        // Store label bounds for hover detection (instead of the "i" icon)
        RECT labelBoundsRect = {
            (int)processLabelPos.X,
            (int)processLabelPos.Y,
            (int)(processLabelPos.X + 200), // Approximate width of "Game Mouse Blocker:"
            (int)(processLabelPos.Y + 20)   // Approximate height
        };
        g_tooltipBounds = labelBoundsRect; // Use the same variable for hover detection


        // Display tooltip text if hovering


        // Continue with existing code...
        RectF processBoxRect((REAL)processComboRect.left, (REAL)(processComboRect.top + settingsScrollOffset),
            (REAL)(processComboRect.right - processComboRect.left), (REAL)(processComboRect.bottom - processComboRect.top));
        SolidBrush processBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&processBoxBrush, processBoxRect);
        Pen processBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&processBoxPen, processBoxRect);

        // Display selected game name
        std::wstring gameName = L"Click to Select Game";
        std::wstring exeName = L"";
        if (g_selectedProcessId != 0) {
            // Get window title for display
            std::wstring windowTitle = L"";
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                std::wstring* result = reinterpret_cast<std::wstring*>(lParam);
                DWORD windowProcessId;
                GetWindowThreadProcessId(hwnd, &windowProcessId);

                if (windowProcessId == g_selectedProcessId && IsWindowVisible(hwnd)) {
                    wchar_t title[256];
                    if (GetWindowTextW(hwnd, title, 256) > 0) {
                        *result = title;
                        // Filter out system windows
                        if (!result->empty() &&
                            *result != L"Default IME" &&
                            *result != L"MSCTFIME UI") {
                            return FALSE; // Found good title, stop
                        }
                    }
                }
                return TRUE; // Continue
                }, reinterpret_cast<LPARAM>(&windowTitle));

            // Get executable name
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot != INVALID_HANDLE_VALUE) {
                PROCESSENTRY32 pe;
                pe.dwSize = sizeof(PROCESSENTRY32);
                if (Process32First(hSnapshot, &pe)) {
                    do {
                        if (pe.th32ProcessID == g_selectedProcessId) {
                            exeName = pe.szExeFile;
                            break;
                        }
                    } while (Process32Next(hSnapshot, &pe));
                }
                CloseHandle(hSnapshot);
            }

            // Build display name
            if (!windowTitle.empty() && windowTitle != L"Default IME" && windowTitle != L"MSCTFIME UI") {
                gameName = windowTitle + L" (" + exeName + L")";
            }
            else {
                gameName = exeName;
            }
        }

        // Draw the game name text
        RectF textRect((REAL)processComboRect.left + 4, (REAL)processComboRect.top + 4 + settingsScrollOffset,
            (REAL)(processComboRect.right - processComboRect.left - 8), (REAL)(processComboRect.bottom - processComboRect.top - 8));

        StringFormat format;
        format.SetFormatFlags(StringFormatFlagsNoWrap);
        format.SetTrimming(StringTrimmingEllipsisCharacter);
        graphics.DrawString(gameName.c_str(), -1, &rowFont, textRect, &format, &valueBrush);

        // Control buttons
// Control buttons - Two checkboxes instead of one button
        int buttonWidth = 125;
        int buttonHeight = 30;
        int buttonSpacing = 10;
        int buttonsY = processComboY + comboHeight + 15;

        // Mouse Block checkbox
        mouseBlockCheckboxRect = {
            settingsPanelRect.left + 20,
            buttonsY,
            settingsPanelRect.left + 20 + buttonWidth,
            buttonsY + buttonHeight
        };
        RectF mouseBlockRect((REAL)mouseBlockCheckboxRect.left, (REAL)(mouseBlockCheckboxRect.top + settingsScrollOffset),
            (REAL)(mouseBlockCheckboxRect.right - mouseBlockCheckboxRect.left), (REAL)(mouseBlockCheckboxRect.bottom - mouseBlockCheckboxRect.top));

        // XInput Block checkbox
        xinputBlockCheckboxRect = {
            settingsPanelRect.left + 20 + buttonWidth + buttonSpacing,
            buttonsY,
            settingsPanelRect.left + 20 + buttonWidth + buttonSpacing + buttonWidth,
            buttonsY + buttonHeight
        };
        RectF xinputBlockRect((REAL)xinputBlockCheckboxRect.left, (REAL)(xinputBlockCheckboxRect.top + settingsScrollOffset),
            (REAL)(xinputBlockCheckboxRect.right - xinputBlockCheckboxRect.left), (REAL)(xinputBlockCheckboxRect.bottom - xinputBlockCheckboxRect.top));

        // Draw Mouse Block checkbox
        SolidBrush mouseBlockBrush(g_mouseBlockEnabled ? Color(0, 255, 136) : Color(60, 60, 60));
        graphics.FillRectangle(&mouseBlockBrush, mouseBlockRect);
        graphics.DrawRectangle(&processBoxPen, mouseBlockRect);

        // Draw XInput Block checkbox  
        SolidBrush xinputBlockBrush(g_xinputBlockEnabled ? Color(0, 255, 136) : Color(60, 60, 60));
        graphics.FillRectangle(&xinputBlockBrush, xinputBlockRect);
        graphics.DrawRectangle(&processBoxPen, xinputBlockRect);

        // Centered text for both checkboxes
        std::wstring mouseBlockText = g_mouseBlockEnabled ? L"Mouse Blocked" : L"Mouse Free";
        std::wstring xinputBlockText = g_xinputBlockEnabled ? L"XInput Blocked" : L"XInput Free";

        RectF mouseBlockTextRect = mouseBlockRect;
        RectF xinputBlockTextRect = xinputBlockRect;

        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);

        graphics.DrawString(mouseBlockText.c_str(), -1, &rowFont, mouseBlockTextRect, &centerFormat, &valueBrush);
        graphics.DrawString(xinputBlockText.c_str(), -1, &rowFont, xinputBlockTextRect, &centerFormat, &valueBrush);

        // Subtle instruction text
        PointF instructionPos((REAL)mouseBlockCheckboxRect.left, (REAL)(mouseBlockCheckboxRect.bottom + 5 + settingsScrollOffset));
        SolidBrush instructionBrush(Color(150, 150, 150));
        Font smallFont(&fontFamily, 10, FontStyleRegular, UnitPixel);
        graphics.DrawString(L"Hold RMB to use mouse", -1, &smallFont, instructionPos, &instructionBrush);
        int mouseSteeringY = buttonsY + 130; // 30px gap after injection buttons

        // --- Subtle Line and Heading Above Mouse Steering ---
        int mouseHeadingY = mouseSteeringY - 45; // Position above the toggle
        Pen mouseLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&mouseLinePen,
            settingsPanelRect.left + 40, mouseHeadingY + settingsScrollOffset,
            settingsPanelRect.right - 40, mouseHeadingY + settingsScrollOffset);

        PointF mouseHeadingPos((REAL)(settingsPanelRect.left), (REAL)(mouseHeadingY + 15 + settingsScrollOffset));
        SolidBrush mouseHeadingBrush(Color(180, 180, 180)); // Subtle gray-white color
        StringFormat mouseHeadingFormat;
        mouseHeadingFormat.SetAlignment(StringAlignmentCenter);
        mouseHeadingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF mouseHeadingRect((REAL)settingsPanelRect.left, (REAL)(mouseHeadingY + 5 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Mouse Steering", -1, &rowFont, mouseHeadingRect, &mouseHeadingFormat, &mouseHeadingBrush);

        // --- Mouse Steering Toggle ---
        mouseSteeringToggleRect = { settingsPanelRect.left + 20, mouseSteeringY, settingsPanelRect.right - 20, mouseSteeringY + toggleHeight * 2 };
        PointF mouseLabel1((REAL)mouseSteeringToggleRect.left, (REAL)mouseSteeringToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Mouse Steering", -1, &rowFont, mouseLabel1, &labelBrush);
        tooltips[20].bounds = {
            mouseSteeringToggleRect.left,
            mouseSteeringToggleRect.top,
            mouseSteeringToggleRect.right,
            mouseSteeringToggleRect.top + toggleHeight  // Only first line height
        };
        tooltips[20].text = L"Enables mouse steering for games. DO NOT USE in game's default mouse steering, always use this. Also works on games that don't have mouse steering support.";
        RectF mouseCheckboxRect(mouseLabel1.X + 140, mouseLabel1.Y, 20.0f, 20.0f);
        SolidBrush mouseBoxBrush(mouseSteeringEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&mouseBoxBrush, mouseCheckboxRect);
        graphics.DrawRectangle(&boxPen, mouseCheckboxRect);


        // --- Steering Sensitivity Slider ---
        int steeringSensY = mouseSteeringToggleRect.bottom + 15; // small gap below toggle
        steeringSensSliderRect = {
            settingsPanelRect.left + 20,
            steeringSensY,
            settingsPanelRect.right - 20,
            steeringSensY + sliderHeight
        };
        char steeringBuffer[8];
        sprintf_s(steeringBuffer, "%.2f", steeringSensitivity);
        float steeringNorm = (steeringSensitivity - 0.1f) / (5.0f - 0.1f);
        DrawSlider(steeringSensSliderRect, steeringNorm, "Steering Sensitivity", steeringBuffer);
        tooltips[21].bounds = steeringSensSliderRect;
        tooltips[21].text = L"Adjusts the sensitivity of mouse steering input.";
        // --- Max Steering Degrees Slider ---
        int steeringDegreesY = steeringSensSliderRect.bottom + 65; // gap below steering sensitivity
        steeringDegreesSliderRect = {
            settingsPanelRect.left + 20,
            steeringDegreesY,
            settingsPanelRect.right - 20,
            steeringDegreesY + sliderHeight
        };

        // Clamp maxSteeringDegrees to 90–900
        maxSteeringDegrees = max(90.0f, min(900.0f, maxSteeringDegrees));

        char steeringDegreesBuffer[8];
        sprintf_s(steeringDegreesBuffer, "%.0f", maxSteeringDegrees); // show integer degrees

        // Normalize for slider: 90 → 0, 900 → 1
        float steeringDegreesNorm = (maxSteeringDegrees - 90.0f) / (900.0f - 90.0f);

        DrawSlider(steeringDegreesSliderRect, steeringDegreesNorm, "Max Steering Degrees", steeringDegreesBuffer);
        tooltips[22].bounds = steeringDegreesSliderRect;
        tooltips[22].text = L"Limits the maximum steering rotation to prevent over-rotation. Higher values allow more steering range.";
        // --- Acc/Brake Sensitivity Slider ---
// --- Acc/Brake Sensitivity Slider ---
        int accBrakeSensY = steeringDegreesSliderRect.bottom + 65; // gap below max steering degrees
        accBrakeSensSliderRect = {
            settingsPanelRect.left + 20,
            accBrakeSensY,
            settingsPanelRect.right - 20,
            accBrakeSensY + sliderHeight
        };
        char accBrakeBuffer[8];
        sprintf_s(accBrakeBuffer, "%.2f", accBrakeSensitivity);
        float accBrakeNorm = (accBrakeSensitivity - 0.1f) / 19.9f; // normalize 0.1-20.0 range
        DrawSlider(accBrakeSensSliderRect, accBrakeNorm, "Acc/Brake Sensitivity", accBrakeBuffer);
        tooltips[23].bounds = accBrakeSensSliderRect;
        tooltips[23].text = L"Adjusts the sensitivity of throttle and brake input from vertical mouse movement.";

        // --- Throttle/Brake Indicator ---
        int showYBarY = accBrakeSensSliderRect.bottom + 30;
        showYBarToggleRect = { settingsPanelRect.left + 20, showYBarY, settingsPanelRect.right - 20, showYBarY + toggleHeight };

        PointF showYBarLabelPos((REAL)showYBarToggleRect.left, (REAL)showYBarToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Throttle/Brake Indicator", -1, &rowFont, showYBarLabelPos, &labelBrush);
        tooltips[24].bounds = showYBarToggleRect;
        tooltips[24].text = L"Shows a vertical bar next to the H-shifter displaying throttle and brake input levels.";

        RectF showYBarCheckboxRect(showYBarToggleRect.left + 190, showYBarToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush showYBarBoxBrush(showYBar ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showYBarBoxBrush, showYBarCheckboxRect);
        graphics.DrawRectangle(&boxPen, showYBarCheckboxRect);
        // --- Brake Resistance Factor Slider ---
        int brakeResistanceY = showYBarY + sliderHeight + 55;
        brakeresistanceSlider = {
            settingsPanelRect.left + 20,
            brakeResistanceY,
            settingsPanelRect.right - 20,
            brakeResistanceY + sliderHeight
        };

        char brakeResistanceBuffer[8];
        sprintf_s(brakeResistanceBuffer, "%.1f", brakeresistanceFactor);
        float brakeResistanceNorm = brakeresistanceFactor / 50.0f; // normalize 0-50 range
        DrawSlider(brakeresistanceSlider, brakeResistanceNorm, "Brake Resistance", brakeResistanceBuffer);
        tooltips[25].bounds = brakeresistanceSlider;
        tooltips[25].text = L"Adjusts resistance when pressing brakes (higher = more resistance at full brake).";

        // --- Acceleration Resistance Factor Slider ---
        int accelerationResistanceY = brakeResistanceY + sliderHeight + 55;
        accelerationresistanceSlider = {
            settingsPanelRect.left + 20,
            accelerationResistanceY,
            settingsPanelRect.right - 20,
            accelerationResistanceY + sliderHeight
        };

        char accelerationResistanceBuffer[8];
        sprintf_s(accelerationResistanceBuffer, "%.1f", accelerationResistanceFactor);
        float accelerationResistanceNorm = accelerationResistanceFactor / 50.0f; // normalize 0-50 range
        DrawSlider(accelerationresistanceSlider, accelerationResistanceNorm, "Throttle Resistance", accelerationResistanceBuffer);
        tooltips[26].bounds = accelerationresistanceSlider;
        tooltips[26].text = L"Adjusts resistance when pressing throttle (higher = more resistance at full throttle).";
        // --- Y-bar Fixed Transparency Toggle ---
        int yBarTransparencyY = accelerationresistanceSlider.bottom + 30;
        yBarFixedTransToggle = { settingsPanelRect.left + 20, yBarTransparencyY, settingsPanelRect.right - 20, yBarTransparencyY + toggleHeight };

        PointF yBarTransLabelPos((REAL)yBarFixedTransToggle.left, (REAL)yBarFixedTransToggle.top + settingsScrollOffset);
        graphics.DrawString(L"T/B Better Visibility", -1, &rowFont, yBarTransLabelPos, &labelBrush);
        tooltips[27].bounds = yBarFixedTransToggle;
        tooltips[27].text = L"Enhances throttle/brake indicator with dynamic colors and separate transparency control for better visibility in all conditions.";
        RectF yBarTransCheckboxRect(yBarFixedTransToggle.left + 190, yBarFixedTransToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush yBarTransBoxBrush(useYbarFixedTransparency ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&yBarTransBoxBrush, yBarTransCheckboxRect);
        graphics.DrawRectangle(&boxPen, yBarTransCheckboxRect);

        // --- Y-bar Alpha Slider ---
// --- Y-bar Alpha Slider ---
        int yBarAlphaY = yBarFixedTransToggle.bottom + 35;
        yBarAlphaSlider = { settingsPanelRect.left + 20, yBarAlphaY, settingsPanelRect.right - 20, yBarAlphaY + sliderHeight };

        char yBarAlphaBuffer[8];
        sprintf_s(yBarAlphaBuffer, "%d%%", (int)((yBarAlpha / 255.0f) * 100.0f)); // Convert to percentage
        DrawSlider(yBarAlphaSlider, (float)yBarAlpha / 255.0f, "T/B Transparency Level", yBarAlphaBuffer); // Changed label too
        tooltips[28].bounds = yBarAlphaSlider;
        tooltips[28].text = L"Sets the transparency level for the throttle/brake indicator when fixed transparency is enabled (0% = fully transparent, 100% = fully opaque).";

        // --- Axis Interdependent Smoothing ---
        int axisSmoothingY = yBarAlphaSlider.bottom + 35;  // Changed from showYBarToggleRect.bottom + 30
        useAxisSmoothingToggle = { settingsPanelRect.left + 20, axisSmoothingY, settingsPanelRect.right - 20, axisSmoothingY + toggleHeight };

        PointF axisSmoothingLabelPos((REAL)useAxisSmoothingToggle.left, (REAL)useAxisSmoothingToggle.top + settingsScrollOffset);
        graphics.DrawString(L"Steering T/B Smoothing", -1, &rowFont, axisSmoothingLabelPos, &labelBrush);
        tooltips[29].bounds = useAxisSmoothingToggle;
        tooltips[29].text = L"When enabled, reduces steering sensitivity during heavy acceleration/braking and vice versa for more stable control.";

        RectF axisSmoothingCheckboxRect(useAxisSmoothingToggle.left + 190, useAxisSmoothingToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush axisSmoothingBoxBrush(useAxisSmoothing ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&axisSmoothingBoxBrush, axisSmoothingCheckboxRect);
        graphics.DrawRectangle(&boxPen, axisSmoothingCheckboxRect);

        // --- Axis Smoothing Factor Slider ---
        int axisSmoothingFactorY = useAxisSmoothingToggle.bottom + 35;
        axisSmoothingFactorSlider = { settingsPanelRect.left + 20, axisSmoothingFactorY, settingsPanelRect.right - 20, axisSmoothingFactorY + sliderHeight };

        char axisSmoothingBuffer[8];
        sprintf_s(axisSmoothingBuffer, "%.2f", axisSmoothingFactor);
        DrawSlider(axisSmoothingFactorSlider, axisSmoothingFactor, "Smoothing Strength", axisSmoothingBuffer);
        tooltips[30].bounds = axisSmoothingFactorSlider;
        tooltips[30].text = L"Controls how much one axis's sensitivity is reduced when the other axis is active. Higher values = more stability.";

        // --- Mouse Steering Indicator ---
        int showXBarY = axisSmoothingFactorSlider.bottom + 35;
        showXBarToggleRect = { settingsPanelRect.left + 20, showXBarY, settingsPanelRect.right - 20, showXBarY + toggleHeight };

        PointF showXBarLabelPos((REAL)showXBarToggleRect.left, (REAL)showXBarToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Mouse Steering Indicator", -1, &rowFont, showXBarLabelPos, &labelBrush);
        tooltips[31].bounds = showXBarToggleRect;
        tooltips[31].text = L"Shows a horizontal bar below the H-shifter displaying steering input position.";

        RectF showXBarCheckboxRect(showXBarToggleRect.left + 190, showXBarToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush showXBarBoxBrush(showXBar ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showXBarBoxBrush, showXBarCheckboxRect);
        graphics.DrawRectangle(&boxPen, showXBarCheckboxRect);

        // --- Use Throttle/Brake Toggle ---
        int throttleBrakeY = showXBarToggleRect.bottom + 10;  // spacing after Mouse Steering Indicator
        useThrottleBrakeToggleRect = { settingsPanelRect.left + 20, throttleBrakeY, settingsPanelRect.right - 20, throttleBrakeY + toggleHeight };

        PointF throttleBrakeLabelPos((REAL)useThrottleBrakeToggleRect.left, (REAL)useThrottleBrakeToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Throttle/Brake (W/S)", -1, &rowFont, throttleBrakeLabelPos, &labelBrush);
        tooltips[32].bounds = useThrottleBrakeToggleRect;  // Update tooltip index as needed
        tooltips[32].text = L"Enables using W/S keys for throttle and brake control on vJoy Z/RZ axes.";

        RectF throttleBrakeCheckboxRect(throttleBrakeLabelPos.X + 190, throttleBrakeLabelPos.Y, 20.0f, 20.0f);
        SolidBrush throttleBrakeBoxBrush(useThrottleBrakeAxes ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&throttleBrakeBoxBrush, throttleBrakeCheckboxRect);
        graphics.DrawRectangle(&boxPen, throttleBrakeCheckboxRect);

        // --- Use Scroll → Rx Toggle ---
        int scrollClutchY = throttleBrakeY + toggleHeight + 10;  // spacing after throttle/brake toggle
        useScrollClutchToggleRect = { settingsPanelRect.left + 20, scrollClutchY, settingsPanelRect.right - 20, scrollClutchY + toggleHeight };

        PointF scrollLabelPos((REAL)useScrollClutchToggleRect.left, (REAL)useScrollClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Scroll to Clutch", -1, &rowFont, scrollLabelPos, &labelBrush);
        tooltips[33].bounds = useScrollClutchToggleRect;
        tooltips[33].text = L"Enables using mouse scroll wheel to control clutch input.";

        RectF scrollCheckboxRect(scrollLabelPos.X + 190, scrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush scrollBoxBrush(useScrollClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&scrollBoxBrush, scrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, scrollCheckboxRect);
        // --- Half Scroll Clutch Toggle ---
        int halfScrollClutchY = scrollClutchY + toggleHeight + 10; // spacing below scroll clutch toggle
        halfScrollClutchToggleRect = { settingsPanelRect.left + 20, halfScrollClutchY, settingsPanelRect.right - 20, halfScrollClutchY + toggleHeight };

        PointF halfScrollLabelPos((REAL)halfScrollClutchToggleRect.left, (REAL)halfScrollClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Half Scroll Clutch", -1, &rowFont, halfScrollLabelPos, &labelBrush);
        tooltips[34].bounds = halfScrollClutchToggleRect;
        tooltips[34].text = L"Uses only half the scroll range for full clutch control (0-100% clutch in half the axis range).";
        RectF halfScrollCheckboxRect(halfScrollLabelPos.X + 190, halfScrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush halfScrollBoxBrush(useHalfClutch ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&halfScrollBoxBrush, halfScrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, halfScrollCheckboxRect);

        // --- Show Clutch / Rx Indicator Toggle ---
        int showClutchY = halfScrollClutchY + toggleHeight + 10;
        showClutchToggleRect = { settingsPanelRect.left + 20, showClutchY, settingsPanelRect.right - 20, showClutchY + toggleHeight };

        PointF showClutchLabelPos((REAL)showClutchToggleRect.left, (REAL)showClutchToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Clutch Indicator", -1, &rowFont, showClutchLabelPos, &labelBrush);
        tooltips[35].bounds = showClutchToggleRect;
        tooltips[35].text = L"Shows clutch level indicator bar and changes H-shifter color when clutch is applied.";
        RectF showClutchCheckboxRect(showClutchLabelPos.X + 190, showClutchLabelPos.Y, 20.0f, 20.0f);
        SolidBrush showClutchBoxBrush(showClutchIndicator ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&showClutchBoxBrush, showClutchCheckboxRect);
        graphics.DrawRectangle(&boxPen, showClutchCheckboxRect);


        // --- Scroll Sensitivity Slider ---
        int scrollSensY = showClutchY + toggleHeight + 45; // 20px gap after showClutch toggle
        scrollSensSliderRect = {
            settingsPanelRect.left + 20,
            scrollSensY,
            settingsPanelRect.right - 20,
            scrollSensY + sliderHeight
        };

        char scrollSensBuffer[8];
        sprintf_s(scrollSensBuffer, "%.2f", scrollClutchSens);
        float scrollSensNorm = scrollClutchSens / 10.0f; // normalize 0–10 range
        DrawSlider(scrollSensSliderRect, scrollSensNorm, "Scroll Sensitivity", scrollSensBuffer);
        tooltips[36].bounds = scrollSensSliderRect;
        tooltips[36].text = L"Adjusts how sensitive the scroll wheel is for clutch control.";

        // --- Smooth Scroll Speed Slider ---
        int smoothScrollY = scrollSensY + toggleHeight + 45;
        smoothScrollSlider = {
            settingsPanelRect.left + 20,
            smoothScrollY,
            settingsPanelRect.right - 20,
            smoothScrollY + sliderHeight
        };

        char smoothScrollBuffer[8];
        sprintf_s(smoothScrollBuffer, "%.1f", smoothScrollSpeed);
        float smoothScrollNorm = (smoothScrollSpeed - 1.0f) / 19.0f; // normalize 1.0-20.0 range
        DrawSlider(smoothScrollSlider, smoothScrollNorm, "Scroll Smoothness", smoothScrollBuffer);
        tooltips[37].bounds = smoothScrollSlider;
        tooltips[37].text = L"Adjusts how smoothly the scroll clutch moves (higher = faster response, lower = smoother movement).";

        // --- Invert Scroll → Clutch Toggle ---
        int invertScrollY = smoothScrollY + sliderHeight + 30; // 20px gap after scroll sensitivity slider
        invertScrollToggleRect = { settingsPanelRect.left + 20, invertScrollY, settingsPanelRect.right - 20, invertScrollY + toggleHeight };

        PointF invertScrollLabelPos((REAL)invertScrollToggleRect.left, (REAL)invertScrollToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Invert Scroll to Clutch", -1, &rowFont, invertScrollLabelPos, &labelBrush);
        tooltips[38].bounds = invertScrollToggleRect;
        tooltips[38].text = L"Reverses the scroll direction for clutch control (scroll up/down reversed).";
        RectF invertScrollCheckboxRect(invertScrollLabelPos.X + 190, invertScrollLabelPos.Y, 20.0f, 20.0f);
        SolidBrush invertScrollBoxBrush(invertScrollClutchAxis ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&invertScrollBoxBrush, invertScrollCheckboxRect);
        graphics.DrawRectangle(&boxPen, invertScrollCheckboxRect);

        // --- Adjust steering combo position ---
        int steeringY = invertScrollY + toggleHeight + 40; // 40px gap after invert toggle
        steeringDeviceComboRect = { settingsPanelRect.left + 20, steeringY, settingsPanelRect.right - 20, steeringY + comboHeight };


        // --- Steering Device Selector ---
// --- Steering Device Selector ---


        PointF steeringLabelPos((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(L"Steering Mouse Device:", -1, &rowFont, steeringLabelPos, &labelBrush);
        tooltips[39].bounds = steeringDeviceComboRect;
        tooltips[39].text = L"Select which mouse device controls steering (can be different from H-shifter mouse).";

        RectF steeringBoxRect((REAL)steeringDeviceComboRect.left, (REAL)(steeringDeviceComboRect.top + settingsScrollOffset),
            (REAL)(steeringDeviceComboRect.right - steeringDeviceComboRect.left), (REAL)(steeringDeviceComboRect.bottom - steeringDeviceComboRect.top));
        graphics.FillRectangle(&deviceBoxBrush, steeringBoxRect);
        graphics.DrawRectangle(&deviceBoxPen, steeringBoxRect);

        std::wstring selName3 = L"All Mice";
        if (g_selectedSteeringDevice)
        {
            for (auto& d : g_mouseDevices)
                if (d.hDevice == g_selectedSteeringDevice)
                    selName3 = d.name;
        }
        PointF steeringTextPos((REAL)steeringDeviceComboRect.left + 4, (REAL)steeringDeviceComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selName3.c_str(), -1, &rowFont, steeringTextPos, &valueBrush);
        // --- Subtle Line Below Steering Device Selector ---
        int steeringLineY = steeringDeviceComboRect.bottom + 20; // 20 pixels below the steering device selector
        Pen steeringLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&steeringLinePen,
            settingsPanelRect.left + 40, steeringLineY + settingsScrollOffset,
            settingsPanelRect.right - 40, steeringLineY + settingsScrollOffset);
        // --- Use XInput Toggle ---
        int lastY = steeringDeviceComboRect.bottom + 90; // 30px gap

        // --- Use XInput / Controller Toggle ---


// --- Subtle Line and Heading Above Controller ---
        int controllerHeadingY = lastY - 45; // Position above the toggle
        Pen controllerLinePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&controllerLinePen,
            settingsPanelRect.left + 40, controllerHeadingY + settingsScrollOffset,
            settingsPanelRect.right - 40, controllerHeadingY + settingsScrollOffset);

        PointF controllerHeadingPos((REAL)(settingsPanelRect.left), (REAL)(controllerHeadingY + 8 + settingsScrollOffset));
        SolidBrush controllerHeadingBrush(Color(180, 180, 180)); // Subtle gray-white color
        StringFormat controllerHeadingFormat;
        controllerHeadingFormat.SetAlignment(StringAlignmentCenter);
        controllerHeadingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF controllerHeadingRect((REAL)settingsPanelRect.left, (REAL)(controllerHeadingY + 8 + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Controller", -1, &rowFont, controllerHeadingRect, &controllerHeadingFormat, &controllerHeadingBrush);

        // --- Enable Controller Toggle ---
        useXInputToggleRect = { settingsPanelRect.left + 20, lastY, settingsPanelRect.right - 20, lastY + toggleHeight };
        PointF useXInputLabelPos((REAL)useXInputToggleRect.left, (REAL)useXInputToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Enable Controller", -1, &rowFont, useXInputLabelPos, &labelBrush);
        // Enable Controller
        tooltips[40].bounds = useXInputToggleRect;
        tooltips[40].text = L"Uses controller instead of mouse for H-shifter input.";
        // Draw checkbox closer to label
        RectF useXInputCheckboxRect(useXInputToggleRect.left + 180, useXInputToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush useXInputBoxBrush(useXInput ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&useXInputBoxBrush, useXInputCheckboxRect);
        graphics.DrawRectangle(&boxPen, useXInputCheckboxRect);

        // --- Gamepad Device Selector ---
        const int spacingAfterToggle = 40; // gap after Enable Controller
        int gamepadComboY = useXInputToggleRect.bottom + spacingAfterToggle;

        gamepadComboRect = { settingsPanelRect.left + 20, gamepadComboY, settingsPanelRect.right - 20, gamepadComboY + comboHeight };
        PointF gamepadLabelPos((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset - nameControlSpacing));
        graphics.DrawString(L"Select Controller:", -1, &rowFont, gamepadLabelPos, &labelBrush);
        // Gamepad Device Selector
        tooltips[41].bounds = gamepadComboRect;
        tooltips[41].text = L"Select which gamepad/controller to use for H-shifter input.";


        // Draw selector box
        RectF gamepadBoxRect((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset),
            (REAL)(gamepadComboRect.right - gamepadComboRect.left), (REAL)(gamepadComboRect.bottom - gamepadComboRect.top));
        SolidBrush gamepadBoxBrush(Color(30, 30, 30));
        graphics.FillRectangle(&gamepadBoxBrush, gamepadBoxRect);
        Pen gamepadBoxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&gamepadBoxPen, gamepadBoxRect);

        // Draw selected gamepad name
        std::wstring selGamepadName = L"None";
        if (g_selectedGamepadIndex >= 0 && g_selectedGamepadIndex < g_gamepads.size()) {
            const char* name = SDL_GameControllerName(g_gamepads[g_selectedGamepadIndex].controller);
            selGamepadName = ToWString(name);
        }
        PointF selTextPos1((REAL)gamepadComboRect.left + 4, (REAL)gamepadComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selGamepadName.c_str(), -1, &rowFont, selTextPos1, &valueBrush);

        // --- Controller Sensitivity Slider ---
        // Position it just below Gamepad selector
        int controllerSensSliderY = gamepadComboRect.bottom + 55; // small gap
        controllerSensSliderRect = {
            settingsPanelRect.left + 20,
            controllerSensSliderY,
            settingsPanelRect.right - 20,
            controllerSensSliderY + sliderHeight
        };

        char sensValueBuffer[8];
        sprintf_s(sensValueBuffer, "%.2f", controllerSensMultiplier);
        DrawSlider(controllerSensSliderRect, controllerSensSliderValue, "Controller Sensitivity", sensValueBuffer);
        // Controller Sensitivity
        tooltips[42].bounds = controllerSensSliderRect;
        tooltips[42].text = L"Adjusts the sensitivity of controller stick inputs for H-shifter.";

        // --- Use Right Stick for Knob Toggle ---
        // (your existing code continues here)

        // --- Use Right Stick for Knob Toggle ---
        int rightStickToggleY = controllerSensSliderRect.bottom + 40; // space after slider
        useRightStickToggleRect = { settingsPanelRect.left + 20, rightStickToggleY, settingsPanelRect.right - 20, rightStickToggleY + toggleHeight };

        PointF rightStickLabelPos((REAL)useRightStickToggleRect.left, (REAL)useRightStickToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Right Stick for Knob", -1, &rowFont, rightStickLabelPos, &labelBrush);
        tooltips[43].bounds = useRightStickToggleRect;
        tooltips[43].text = L"Uses right stick instead of left stick for H-shifter knob movement.";
        // Checkbox
        RectF rightStickCheckboxRect(useRightStickToggleRect.left + 180, useRightStickToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush rightStickBoxBrush(useRightStick ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&rightStickBoxBrush, rightStickCheckboxRect);
        graphics.DrawRectangle(&boxPen, rightStickCheckboxRect);
        // --- Disable Real Knob Movement Toggle ---
        int disableKnobY = useRightStickToggleRect.bottom + 20; // space after Right Stick toggle
        disableRealKnobMovementToggleRect = { settingsPanelRect.left + 20, disableKnobY, settingsPanelRect.right - 20, disableKnobY + toggleHeight };
        PointF disableKnobLabelPos((REAL)disableRealKnobMovementToggleRect.left, (REAL)disableRealKnobMovementToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Use Right Stick to look", -1, &rowFont, disableKnobLabelPos, &labelBrush);
        // Use Right Stick to look
        tooltips[44].bounds = disableRealKnobMovementToggleRect;
        tooltips[44].text = L"Uses right stick for camera look instead of H-shifter movement. Holding assist button enables knob assist and disables camera look.";
        // Checkbox closer to label
        RectF disableKnobCheckboxRect(disableRealKnobMovementToggleRect.left + 180, disableRealKnobMovementToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush disableKnobBoxBrush(disableRealKnobMovement ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&disableKnobBoxBrush, disableKnobCheckboxRect);
        graphics.DrawRectangle(&boxPen, disableKnobCheckboxRect);

        // --- Binding Mode For R-Axis Toggle ---
        int bindingModeY = disableRealKnobMovementToggleRect.bottom + 20; // space after previous toggle
        bindingModeForAxisToggle = { settingsPanelRect.left + 20, bindingModeY, settingsPanelRect.right - 20, bindingModeY + toggleHeight };
        PointF bindingModeLabelPos((REAL)bindingModeForAxisToggle.left, (REAL)bindingModeForAxisToggle.top + settingsScrollOffset);
        graphics.DrawString(L"Binding Mode", -1, &rowFont, bindingModeLabelPos, &labelBrush);
        // Binding Mode for R-Axis
        tooltips[45].bounds = bindingModeForAxisToggle; // Add new tooltip index
        tooltips[45].text = L"When enabled, right stick and clutch will output full values with minimal input for easier controller binding. DISABLE IT WHEN IT'S NOT NEEDED.";
        // Checkbox
        RectF bindingModeCheckboxRect(bindingModeForAxisToggle.left + 180, bindingModeForAxisToggle.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush bindingModeBoxBrush(bindingModeForRAxis ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&bindingModeBoxBrush, bindingModeCheckboxRect);
        graphics.DrawRectangle(&boxPen, bindingModeCheckboxRect);

        // --- Invert Assist Axes Toggle ---
        int invertAssistY = bindingModeForAxisToggle.bottom + 20; // space after Binding Mode toggle
        invertAssistToggleRect = { settingsPanelRect.left + 20, invertAssistY, settingsPanelRect.right - 20, invertAssistY + toggleHeight };
        PointF invertAssistLabelPos((REAL)invertAssistToggleRect.left, (REAL)invertAssistToggleRect.top + settingsScrollOffset);
        // Main label
        graphics.DrawString(L"Hold Assist to Look", -1, &rowFont, invertAssistLabelPos, &labelBrush);
        // Hold Assist to Look
        tooltips[46].bounds = invertAssistToggleRect;
        tooltips[46].text = L"Holding assist button enables right stick for camera look (disables assist knob).";
        // --- Subtitle / hint ---
        // Checkbox
        RectF invertAssistCheckboxRect(invertAssistToggleRect.left + 180, invertAssistToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush invertAssistBoxBrush(invertAssistAxes ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&invertAssistBoxBrush, invertAssistCheckboxRect);
        graphics.DrawRectangle(&boxPen, invertAssistCheckboxRect);

