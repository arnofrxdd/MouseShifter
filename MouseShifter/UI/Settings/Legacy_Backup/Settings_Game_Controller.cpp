        // --- Controller Heading ---
        currentY += 80;
        graphics.DrawLine(&linePen, settingsPanelRect.left + 40, currentY + settingsScrollOffset, settingsPanelRect.right - 40, currentY + settingsScrollOffset);
        currentY += 20;

        RectF controllerHeadingRect((REAL)settingsPanelRect.left, (REAL)(currentY + settingsScrollOffset), (REAL)(settingsPanelRect.right - settingsPanelRect.left), 25.0f);
        graphics.DrawString(L"Controller Selection", -1, &headingFont, controllerHeadingRect, &centerFormat, &labelBrush);
        currentY += 40;

        // --- Gamepad Device Selector ---
        gamepadComboRect = { settingsPanelRect.left + 20, currentY, settingsPanelRect.right - 20, currentY + comboHeight };
        PointF gamepadLabelPos((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset - nameControlSpacing));
        graphics.DrawString(L"Select Controller:", -1, &rowFont, gamepadLabelPos, &labelBrush);
        tooltips[41].bounds = gamepadComboRect;
        tooltips[41].text = L"Select which gamepad/controller to use for H-shifter input.";

        RectF gamepadBoxRect((REAL)gamepadComboRect.left, (REAL)(gamepadComboRect.top + settingsScrollOffset), (REAL)(gamepadComboRect.right - gamepadComboRect.left), (REAL)(gamepadComboRect.bottom - gamepadComboRect.top));
        graphics.FillRectangle(&darkBrush, gamepadBoxRect);
        graphics.DrawRectangle(&accentPen, gamepadBoxRect);

        std::wstring selGamepadName = L"None";
        if (g_selectedGamepadIndex >= 0 && g_selectedGamepadIndex < (int)g_gamepads.size()) {
            const char* name = SDL_GameControllerName(g_gamepads[g_selectedGamepadIndex].controller);
            selGamepadName = ToWString(name);
        }
        PointF selTextPos1((REAL)gamepadComboRect.left + 4, (REAL)gamepadComboRect.top + 4 + settingsScrollOffset);
        graphics.DrawString(selGamepadName.c_str(), -1, &rowFont, selTextPos1, &valueBrush);
        
        currentY += comboHeight + 20;
