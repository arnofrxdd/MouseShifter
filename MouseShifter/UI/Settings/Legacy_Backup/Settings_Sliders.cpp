        // --- Draw sliders ---
        char valueBuffer[16];

        // Knob Radius
// Knob Radius
        float tKnob = float(knobRadius - knobRadiusMin) / float(knobRadiusMax - knobRadiusMin);
        sprintf_s(valueBuffer, "%d", knobRadius);
        knobSliderRect = { settingsPanelRect.left + 20, startY, settingsPanelRect.right - 20, startY + sliderHeight };
        DrawSlider(knobSliderRect, tKnob, "Knob Radius", valueBuffer);

        // --- Tooltip for Knob Radius ---
        tooltips[0].bounds = knobSliderRect;
        tooltips[0].text = L"Adjusts the size of the knob.";
        // Gear Radius
        float tGear = float(gearRadius - gearRadiusMin) / float(gearRadiusMax - gearRadiusMin);
        sprintf_s(valueBuffer, "%d", gearRadius);
        gearSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing, settingsPanelRect.right - 20, startY + verticalSpacing + sliderHeight };
        DrawSlider(gearSliderRect, tGear, "Gear Radius", valueBuffer);

        tooltips[1].bounds = gearSliderRect;
        tooltips[1].text = L"Adjusts the size of gears.";
        // Sensitivity
        float tSens = (knobSensitivity - knobSensitivityMin) / (knobSensitivityMax - knobSensitivityMin);
        sprintf_s(valueBuffer, "%.2f", knobSensitivity);
        sensSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 2, settingsPanelRect.right - 20, startY + verticalSpacing * 2 + sliderHeight };
        DrawSlider(sensSliderRect, tSens, "Knob Sensitivity", valueBuffer);
        tooltips[2].bounds = sensSliderRect;
        tooltips[2].text = L"Adjusts mouse sensitivity for knob movement.";
        // Diagonal Assist
        float tDiag = (diagonalAssist - diagMin) / (diagMax - diagMin);
        sprintf_s(valueBuffer, "%.2f", diagonalAssist);
        diagSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 3, settingsPanelRect.right - 20, startY + verticalSpacing * 3 + sliderHeight };
        DrawSlider(diagSliderRect, tDiag, "Diagonal Assist Strength", valueBuffer);
        tooltips[3].bounds = diagSliderRect;
        tooltips[3].text = L"Adjusts assistance for diagonal gear shifts. Higher values may not help with smaller H-shifter layout.";

        // Snap-In Threshold
        float tSnap = (gearSnapInMultiplier - snapInMin) / (snapInMax - snapInMin);
        sprintf_s(valueBuffer, "%.2f", gearSnapInMultiplier);
        snapInSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 4, settingsPanelRect.right - 20, startY + verticalSpacing * 4 + sliderHeight };
        DrawSlider(snapInSliderRect, tSnap, "Snap Sensitivity", valueBuffer);
        tooltips[4].bounds = snapInSliderRect;
        tooltips[4].text = L"Adjusts how easily gears snap into position.";

        // Snap Speed
        float tSnapSpeed = (snapSpeed - snapSpeedMin) / (snapSpeedMax - snapSpeedMin);
        sprintf_s(valueBuffer, "%.2f", snapSpeed);
        snapSpeedSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 5, settingsPanelRect.right - 20, startY + verticalSpacing * 5 + sliderHeight };
        DrawSlider(snapSpeedSliderRect, tSnapSpeed, "Snap Speed", valueBuffer);
        tooltips[5].bounds = snapSpeedSliderRect;
        tooltips[5].text = L"Adjusts how fast the knob snaps to gears. Lower for smooth transitions, higher for instant teleport.";
        // Layout Scale
        float tLayout = (layoutScale - 1.0f) / (3.0f - 1.0f);
        sprintf_s(valueBuffer, "%.2f", layoutScale);
        layoutScaleSliderRect = { settingsPanelRect.left + 20, startY + verticalSpacing * 6, settingsPanelRect.right - 20, startY + verticalSpacing * 6 + sliderHeight };
        DrawSlider(layoutScaleSliderRect, tLayout, "H-Shifter Size", valueBuffer);
        tooltips[6].bounds = layoutScaleSliderRect;
        tooltips[6].text = L"Adjusts the overall size of H-shifter layout.";
        // --- Subtle Line Above H-Shifter Settings ---
        int headingY = startY + verticalSpacing * 7 - 15; // Position above Neutral Toggle
        int lineY = headingY - 10; // 10 pixels above the heading
        Pen linePen(Color(80, 80, 80), 1.0f); // Very subtle gray line
        graphics.DrawLine(&linePen,
            settingsPanelRect.left + 40, lineY + settingsScrollOffset,
            settingsPanelRect.right - 40, lineY + settingsScrollOffset);

        // --- H-Shifter Settings Heading ---
        PointF headingPos((REAL)(settingsPanelRect.left), (REAL)(headingY + settingsScrollOffset));
        SolidBrush headingBrush(Color(180, 180, 180)); // Subtle gray-white color
        Font headingFont(&fontFamily, 16, FontStyleRegular, UnitPixel); // Regular weight, normal size
        StringFormat headingFormat;
        headingFormat.SetAlignment(StringAlignmentCenter);
        headingFormat.SetLineAlignment(StringAlignmentCenter);
        RectF headingRect((REAL)settingsPanelRect.left, (REAL)(headingY + settingsScrollOffset),
            (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"H-Shifter Settings", -1, &headingFont, headingRect, &headingFormat, &headingBrush);
