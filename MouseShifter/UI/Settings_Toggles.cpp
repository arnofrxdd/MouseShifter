        // --- Neutral Toggle ---
        int toggleY = startY + verticalSpacing * 7 + 25;
        neutralToggleRect = { settingsPanelRect.left + 20, toggleY, settingsPanelRect.right - 20, toggleY + toggleHeight };
        PointF toggleLabelPos((REAL)neutralToggleRect.left, (REAL)neutralToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Enable Neutral", -1, &rowFont, toggleLabelPos, &labelBrush);
        // --- Tooltip for Enable Neutral (EXCEPTIONAL - 35px below) ---
// --- Tooltip for Enable Neutral (EXCEPTIONAL - 35px below) ---
        tooltips[7].bounds = neutralToggleRect;
        tooltips[7].text = L"Enables neutral key binding. When knob is in neutral position, sends a neutral key press for games that require explicit neutral binding.";

        RectF checkboxRect(neutralToggleRect.left + 220, neutralToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush boxBrush(isNeutralEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&boxBrush, checkboxRect);
        Pen boxPen(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&boxPen, checkboxRect);
        // --- No Reverse Layout Toggle ---

        // --- Hide High Gears Toggle ---
        // --- Hide High Gears Toggle ---
        int hideHighGearsY = neutralToggleRect.bottom + 10; // stack below noReverse
        hideHighGearsToggleRect = { settingsPanelRect.left + 20, hideHighGearsY, settingsPanelRect.right - 20, hideHighGearsY + toggleHeight };

        PointF hideHighGearsLabelPos((REAL)hideHighGearsToggleRect.left, (REAL)hideHighGearsToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Hide High Gears", -1, &rowFont, hideHighGearsLabelPos, &labelBrush);
        tooltips[8].bounds = hideHighGearsToggleRect;
        tooltips[8].text = L"Hides high gear labeling";
        // Checkbox
        RectF hideHighGearsCheckboxRect(hideHighGearsToggleRect.left + 220, hideHighGearsToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush hideHighGearsBoxBrush(hideHighGears ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&hideHighGearsBoxBrush, hideHighGearsCheckboxRect);
        graphics.DrawRectangle(&boxPen, hideHighGearsCheckboxRect);

        // --- Precision Knob Movement Toggle (between Hide High Gears and Dynamic Transparency) ---
        int knobAccelY = hideHighGearsToggleRect.bottom + 10; // 15px gap
        knobAccelToggleRect = { settingsPanelRect.left + 20, knobAccelY, settingsPanelRect.right - 20, knobAccelY + toggleHeight };

        PointF knobLabelPos((REAL)knobAccelToggleRect.left, (REAL)knobAccelToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Precision Knob Movement", -1, &rowFont, knobLabelPos, &labelBrush);

        // Checkbox rectangle (inverted: checked = disabled, unchecked = enabled)
        RectF knobCheckboxRect(knobAccelToggleRect.left + 220, knobAccelToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush knobBoxBrush(!knobAccelerationEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&knobBoxBrush, knobCheckboxRect);
        Pen boxPen1(Color(0, 255, 136), 2);
        graphics.DrawRectangle(&boxPen1, knobCheckboxRect);
        tooltips[9].bounds = knobAccelToggleRect;
        tooltips[9].text = L"Enables precision mode for mouse knob movement. When enabled, knob movement is more precise and controlled.";
        // --- Realistic Knob Drawing Toggle ---
        int realisticKnobY = knobAccelToggleRect.bottom + 10; // 10px gap
        realisticKnobToggleRect = { settingsPanelRect.left + 20, realisticKnobY, settingsPanelRect.right - 20, realisticKnobY + toggleHeight };

        PointF realisticKnobLabelPos((REAL)realisticKnobToggleRect.left, (REAL)realisticKnobToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Fancy Knob Mode", -1, &rowFont, realisticKnobLabelPos, &labelBrush);

        // Checkbox
        RectF realisticKnobCheckboxRect(realisticKnobToggleRect.left + 220, realisticKnobToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush realisticKnobBoxBrush(realisticKnob ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&realisticKnobBoxBrush, realisticKnobCheckboxRect);
        graphics.DrawRectangle(&boxPen, realisticKnobCheckboxRect);

        tooltips[10].bounds = realisticKnobToggleRect;
        tooltips[10].text = L"Make your knob look extra fancy";

        // --- Clutch Lock Gear Toggle ---
        int clutchLockY = realisticKnobToggleRect.bottom + 10;
        clutchLockGearToggleRect = { settingsPanelRect.left + 20, clutchLockY, settingsPanelRect.right - 20, clutchLockY + toggleHeight };

        PointF clutchLockLabelPos((REAL)clutchLockGearToggleRect.left, (REAL)clutchLockGearToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Clutch Lock Gear", -1, &rowFont, clutchLockLabelPos, &labelBrush);

        RectF clutchLockCheckboxRect(clutchLockGearToggleRect.left + 220, clutchLockGearToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush clutchLockBoxBrush(clutchLockGear ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&clutchLockBoxBrush, clutchLockCheckboxRect);
        graphics.DrawRectangle(&boxPen, clutchLockCheckboxRect);

        // Tooltip for Clutch Lock
        tooltips[13].bounds = clutchLockGearToggleRect;
        tooltips[13].text = L"Prevents leaving/entering gears without clutch engagement.";

        // --- Reverse Gear Lock Toggle ---
        int reverseLockY = clutchLockGearToggleRect.bottom + 10;
        reverseLockToggleRect = { settingsPanelRect.left + 20, reverseLockY, settingsPanelRect.right - 20, reverseLockY + toggleHeight };

        PointF reverseLockLabelPos((REAL)reverseLockToggleRect.left, (REAL)reverseLockToggleRect.top + settingsScrollOffset);
        graphics.DrawString(L"Reverse Gear Lock", -1, &rowFont, reverseLockLabelPos, &labelBrush);

        RectF reverseLockCheckboxRect(reverseLockToggleRect.left + 220, reverseLockToggleRect.top + settingsScrollOffset, 20.0f, 20.0f);
        SolidBrush reverseLockBoxBrush(reverseLockEnabled ? Color(0, 255, 136) : Color(30, 30, 30));
        graphics.FillRectangle(&reverseLockBoxBrush, reverseLockCheckboxRect);
        graphics.DrawRectangle(&boxPen, reverseLockCheckboxRect);

        tooltips[20].bounds = reverseLockToggleRect;
        tooltips[20].text = L"Prevents accidental reverse engagement unless the unlock key is held.";

