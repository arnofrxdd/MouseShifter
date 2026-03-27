        // --- Transparencies ---
        // Dynamic Transparency Toggle
        int dynTransY = deviceComboY + comboHeight + 50;
        dynamicTransparencyToggleRect = { settingsPanelRect.left + 20, dynTransY, settingsPanelRect.right - 20, dynTransY + toggleHeight };

        PointF dynTransLabelPos((REAL)dynamicTransparencyToggleRect.left, (REAL)dynamicTransparencyToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Smart Adaptive Transparency", -1, &rowFont, dynTransLabelPos, &labelBrush);
        tooltips[15].bounds = dynamicTransparencyToggleRect;
        tooltips[15].text = L"Automatically adjusts transparency based on background brightness when in overlay mode.";
        RectF dynTransCheckboxRect(dynamicTransparencyToggleRect.left + 220, dynamicTransparencyToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush dynTransBoxBrush(dynamicTransparencyEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&dynTransBoxBrush, dynTransCheckboxRect);
        graphics.DrawRectangle(&boxPen, dynTransCheckboxRect);

        // Max Alpha Slider
        int transSliderY = dynTransY + toggleHeight + 35;
        transparencySliderRect = { settingsPanelRect.left + 20, transSliderY, settingsPanelRect.right - 20, transSliderY + sliderHeight };
        float tAlpha = float(maxAlpha) / 255.0f;
        int alphaPercent = int(tAlpha * 100.0f);
        sprintf_s(valueBuffer, "%d%%", alphaPercent);
        DrawSlider(transparencySliderRect, tAlpha, "Transparency", valueBuffer);
        tooltips[16].bounds = transparencySliderRect;
        tooltips[16].text = L"Sets the transparency level when the overlay is active.";

        // Idle Transparency Slider
        int idleSliderY = transparencySliderRect.bottom + 55;
        minTransparencySliderRect = { settingsPanelRect.left + 20, idleSliderY, settingsPanelRect.right - 20, idleSliderY + sliderHeight };
        float tIdle = float(minAlpha) / 255.0f;
        int idlePercent = int(tIdle * 100.0f);
        sprintf_s(valueBuffer, "%d%%", idlePercent);
        DrawSlider(minTransparencySliderRect, tIdle, "Idle Transparency", valueBuffer);
        tooltips[17].bounds = minTransparencySliderRect;
        tooltips[17].text = L"Sets the minimum transparency when idle (requires Smart Adaptive Transparency to be enabled).";
        // Fade Delay Slider
// Fade Delay Slider
        int delaySliderY = minTransparencySliderRect.bottom + 55;
        transparencyFadeDelaySliderRect = { settingsPanelRect.left + 20, delaySliderY, settingsPanelRect.right - 20, delaySliderY + sliderHeight };
        float tDelay = float(transparencyFadeDelay) / 2000.0f;
        tDelay = max(0.0f, min(1.0f, tDelay));
        char delayValueBuffer[32];
        sprintf_s(delayValueBuffer, "%lu ms", transparencyFadeDelay);
        DrawSlider(transparencyFadeDelaySliderRect, tDelay, "Fade Delay", delayValueBuffer);
        tooltips[18].bounds = transparencyFadeDelaySliderRect;
        tooltips[18].text = L"Sets the delay before transparency fades to idle level after stopping H-shifter use.";
        // --- Performance Optimization Toggle ---
        int optimizationY = transparencyFadeDelaySliderRect.bottom + 40;
        optimizationToggleRect = { settingsPanelRect.left + 20, optimizationY, settingsPanelRect.right - 20, optimizationY + toggleHeight };

        PointF optimizationLabelPos((REAL)optimizationToggleRect.left, (REAL)optimizationToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Performance Mode", -1, &rowFont, optimizationLabelPos, &labelBrush);
        Font descFont(&fontFamily, 11, FontStyleRegular, UnitPixel); // Slightly bigger font
        tooltips[19].bounds = optimizationToggleRect;
        tooltips[19].text = L"Optimizes for maximum game performance. Highly Recommended.";

        RectF optimizationCheckboxRect(optimizationToggleRect.left + 220, optimizationToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush optimizationBoxBrush(!disableSmartRedraws ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&optimizationBoxBrush, optimizationCheckboxRect);
        graphics.DrawRectangle(&boxPen, optimizationCheckboxRect);


        // --- Subtle Line Below Device Selector / Transparencies ---
        int bottomLineY = optimizationToggleRect.bottom + 15; // Adjusted spacing
        Pen bottomLinePen(Color(80, 80, 80), 1.0f);
        graphics.DrawLine(&bottomLinePen,
            settingsPanelRect.left + 40, bottomLineY + settingsScrollOffset,
            settingsPanelRect.right - 40, bottomLineY + settingsScrollOffset);

