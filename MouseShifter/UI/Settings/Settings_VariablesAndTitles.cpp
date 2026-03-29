// Registry is now defined globally in Settings_Registry.cpp

if (showSettingsPanel)
{
    using namespace Gdiplus;
    Graphics graphics(memDC);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    POINT cursor; GetCursorPos(&cursor); ScreenToClient(hwnd, &cursor);
    POINT itemAdjCursor = { cursor.x, cursor.y - settingsScrollOffset };
    bool anyDropdownOpen = profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen;

    // --- Style Tokens are inherited from ShifterUI.cpp ---

    int currentY = 120;
    int margin = 20;
    int verticalSpacing = 115; // More breathing room
    int toggleSpacing = 50;
    int nameControlSpacing = 35;
    int sliderHeight = 24;
    int toggleHeight = 32;
    int comboHeight = 30;

    char valueBuffer[64]; // For legacy fragments

    // --- Registry Initialization (Inline to avoid local function error) ---
    if (g_settingsRegistry.empty())
    {
        // --- 1. Core Physics Sliders ---
        g_settingsRegistry.push_back({ SettingType::SLIDER_INT, L"Knob Radius", L"Adjusts the size of the knob.", &knobRadius, (float)knobRadiusMin, (float)knobRadiusMax, 85.0f, L"%d", {}, {}, 0 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_INT, L"Gear Radius", L"Adjusts the size of gears.", &gearRadius, (float)gearRadiusMin, (float)gearRadiusMax, 30.0f, L"%d", {}, {}, 1 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Knob Sensitivity", L"Adjusts mouse sensitivity for knob movement.", &knobSensitivity, knobSensitivityMin, knobSensitivityMax, 0.45f, L"%.2f", {}, {}, 2 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Diagonal Assist Strength", L"Adjusts assistance for diagonal gear shifts.", &diagonalAssist, diagMin, diagMax, 1.70f, L"%.2f", {}, {}, 3 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Snap Sensitivity", L"Adjusts how easily gears snap into position.", &gearSnapInMultiplier, snapInMin, snapInMax, 1.70f, L"%.2f", {}, {}, 4 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Snap Speed", L"Adjusts how fast the knob snaps to gears.", &snapSpeed, snapSpeedMin, snapSpeedMax, 0.75f, L"%.2f", {}, {}, 5 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"H-Shifter Size", L"Adjusts the overall size of H-shifter layout.", &layoutScale, 1.0f, 3.0f, 1.75f, L"%.2f", {}, {}, 6 });

        // --- 2. Shifter Configuration Toggles ---
        g_settingsRegistry.push_back({ SettingType::HEADING, L"H-Shifter Settings", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"16-Gear Set", L"Enable 16-gear layout (5 rails) instead of 12-gear (4 rails).", &is16GearSet, 0, 0, 0, L"", {}, {}, 66 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Enable Neutral", L"Enables neutral key binding.", &isNeutralEnabled, 0, 0, 1.0f, L"", {}, {}, 7 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Hide High Gears", L"Hides high gear labeling.", &hideHighGears, 0, 0, 0, L"", {}, {}, 8 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Precision Knob Move", L"Enables precision mode for knob movement.", &knobAccelerationEnabled, 0, 0, 0, L"", {}, {}, 9 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Fancy Knob Mode", L"Make your knob look extra fancy.", &realisticKnob, 0, 0, 0, L"", {}, {}, 10 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Clutch Lock Gear", L"Prevents entering gears without clutch.", &clutchLockGear, 0, 0, 0, L"", {}, {}, 52 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Reverse Gear Lock", L"Prevents accidental reverse engagement.", &reverseLockEnabled, 0, 0, 1.0f, L"", {}, {}, 20 });

        // --- 3. Transparencies ---
        g_settingsRegistry.push_back({ SettingType::HEADING, L"Transparency & Performance", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_collapsedSections[L"Transparency & Performance"] = true;
        
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Smart Adaptive Transparency", L"Automatically adjusts transparency based on background brightness.", &dynamicTransparencyEnabled, 0, 0, 0, L"", {}, {}, 15 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_BYTE, L"Transparency", L"Sets the transparency level when active.", (void*)&maxAlpha, 0.0f, 255.0f, 150.0f, L"%d%%", {}, {}, 16 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_BYTE, L"Idle Transparency", L"Sets the minimum transparency when idle.", (void*)&minAlpha, 0.0f, 255.0f, 20.0f, L"%d%%", {}, {}, 17 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_INT, L"Fade Delay", L"Sets the delay before transparency fades to idle level.", (int*)&transparencyFadeDelay, 0.0f, 2000.0f, 300.0f, L"%d ms", {}, {}, 18 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Performance Mode", L"Optimizes for maximum game performance.", (bool*)((char*)&disableSmartRedraws), 0, 0, 0, L"", {}, {}, 19 });

        // --- 4. Game Control & Indicators ---
        g_settingsRegistry.push_back({ SettingType::HEADING, L"Block Mouse & Look:", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Auto Inject DLL", L"Automatically injects blocking DLLs.", &g_autoInjectEnabled, 0, 0, 0, L"", {}, {}, 49 });

        g_settingsRegistry.push_back({ SettingType::HEADING, L"Mouse Steering", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_collapsedSections[L"Mouse Steering"] = true;

        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Mouse Steering", L"Enables mouse steering for games.", &mouseSteeringEnabled, 0, 0, 0, L"", {}, {}, 51 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Steering Sensitivity", L"Adjusts steering response.", &steeringSensitivity, 0.1f, 5.0f, 1.0f, L"%.2f", {}, {}, 21 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Max Steering Degrees", L"Limits steering rotation range.", &maxSteeringDegrees, 90.0f, 900.0f, 900.0f, L"%.0f", {}, {}, 22 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Acc/Brake Sensitivity", L"Adjusts throttle/brake intensity.", &accBrakeSensitivity, 0.1f, 20.0f, 1.0f, L"%.2f", {}, {}, 23 });

        g_settingsRegistry.push_back({ SettingType::HEADING, L"Indicators & Control", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_collapsedSections[L"Indicators & Control"] = true;

        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Throttle/Brake Indicator", L"Shows vertical bar for T/B.", &showYBar, 0, 0, 0, L"", {}, {}, 24 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Brake Resistance", L"Adjusts Y-axis brake feel.", &brakeresistanceFactor, 0.0f, 50.0f, 15.0f, L"%.1f", {}, {}, 25 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Throttle Resistance", L"Adjusts Y-axis throttle feel.", &accelerationResistanceFactor, 0.0f, 50.0f, 5.0f, L"%.1f", {}, {}, 26 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"T/B Better Visibility", L"Dynamic colors for indicator.", &useYbarFixedTransparency, 0, 0, 0, L"", {}, {}, 27 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_INT, L"T/B Transparency Level", L"Sets transparency for bar.", (int*)&yBarAlpha, 0.0f, 255.0f, 180.0f, L"%d%%", {}, {}, 28 });

        g_settingsRegistry.push_back({ SettingType::HEADING, L"Clutch & Smoothing", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_collapsedSections[L"Clutch & Smoothing"] = true;

        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Steering T/B Smoothing", L"Reduces steering sensitivity while braking/accel.", &useAxisSmoothing, 0, 0, 0, L"", {}, {}, 29 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Smoothing Strength", L"Control axis reduction depth.", &axisSmoothingFactor, 0.0f, 1.0f, 1.0f, L"%.2f", {}, {}, 30 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Mouse Steering Indicator", L"Shows horizontal bar for steering.", &showXBar, 0, 0, 0, L"", {}, {}, 31 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Use Throttle/Brake (W/S)", L"Enables W/S for truck axes.", &useThrottleBrakeAxes, 0, 0, 0, L"", {}, {}, 32 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Use Scroll to Clutch", L"Enables scroll wheel clutch.", &useScrollClutch, 0, 0, 0, L"", {}, {}, 33 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Half Scroll Clutch", L"Uses upper half range.", &useHalfClutch, 0, 0, 0, L"", {}, {}, 34 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Clutch Indicator", L"Shows clutch level bar.", &showClutchIndicator, 0, 0, 1.0f, L"", {}, {}, 35 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Scroll Sensitivity", L"Adjusts scroll response.", &scrollClutchSens, 0.0f, 10.0f, 5.0f, L"%.2f", {}, {}, 36 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Scroll Smoothness", L"Adjusts clutch transitions.", &smoothScrollSpeed, 1.0f, 20.0f, 3.0f, L"%.1f", {}, {}, 37 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Invert Scroll to Clutch", L"Reverses scroll wheel.", &invertScrollClutchAxis, 0, 0, 0, L"", {}, {}, 38 });

        g_settingsRegistry.push_back({ SettingType::HEADING, L"Controller Configuration", L"", nullptr, 0, 0, 0, L"", {}, {}, -1 });
        g_collapsedSections[L"Controller Configuration"] = true;

        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Enable Controller", L"Uses gamepad logic.", &useXInput, 0, 0, 0, L"", {}, {}, 40 });
        g_settingsRegistry.push_back({ SettingType::SLIDER_FLOAT, L"Controller Sensitivity", L"Adjusts stick response.", &controllerSensMultiplier, 0.0f, 3.0f, 1.0f, L"%.2f", {}, {}, 42 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Use Right Stick for Knob", L"Right stick moves knob.", &useRightStick, 0, 0, 1.0f, L"", {}, {}, 43 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Use Right Stick to Look", L"Right stick for camera.", &disableRealKnobMovement, 0, 0, 0, L"", {}, {}, 44 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Binding Mode", L"Enables full axis output.", &bindingModeForRAxis, 0, 0, 0, L"", {}, {}, 45 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Hold Assist to Look", L"Hybrid assist mode.", &invertAssistAxes, 0, 0, 0, L"", {}, {}, 46 });
        g_settingsRegistry.push_back({ SettingType::TOGGLE, L"Use LT/L2 as Clutch", L"Trigger as clutch input.", &useLTAsClutch, 0, 0, 0, L"", {}, {}, 47 });
    }

    // --- Legacy Helper: DrawSlider (for other fragments) ---
    auto DrawSlider = [&](RECT rect, float t, const char* label, const char* valueText)
    {
        RectF sliderRectF((REAL)rect.left, (REAL)rect.top + settingsScrollOffset, (REAL)(rect.right - rect.left), (REAL)(rect.bottom - rect.top));
        graphics.FillRectangle(&bgBrush, sliderRectF);
        float knobX = rect.left + t * (rect.right - rect.left);
        RectF knobRect(knobX - 5, (REAL)(rect.top + settingsScrollOffset), 10.0f, (REAL)(rect.bottom - rect.top));
        graphics.FillRectangle(&accentBrush, knobRect);
        PointF labelPos((REAL)rect.left, (REAL)(rect.top - nameControlSpacing + settingsScrollOffset));
        graphics.DrawString(std::wstring(label, label + strlen(label)).c_str(), -1, &rowFont, labelPos, &labelBrush);
        StringFormat valFormat; valFormat.SetAlignment(StringAlignmentCenter);
        PointF valuePos((REAL)(rect.left + (rect.right - rect.left) / 2), (REAL)(rect.bottom + 2 + settingsScrollOffset));
        graphics.DrawString(std::wstring(valueText, valueText + strlen(valueText)).c_str(), -1, &rowFont, valuePos, &valFormat, &valueBrush);
    };

    // --- Clip drawing to panel ---
    graphics.SetClip(Rect(
        settingsPanelRect.left,
        settingsPanelRect.top,
        settingsPanelRect.right - settingsPanelRect.left,
        settingsPanelRect.bottom - settingsPanelRect.top));

    // --- Smooth scroll ---
    settingsScrollOffsetF += (settingsScrollTarget - settingsScrollOffsetF) * settingsScrollSpeed;
    settingsScrollOffset = (int)settingsScrollOffsetF;

    // --- Panel Title ---
    RectF titleRectF((REAL)settingsPanelRect.left, (REAL)(settingsPanelRect.top + 10 + settingsScrollOffset), (REAL)(settingsPanelRect.right - settingsPanelRect.left), 30.0f);
    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);
    centerFormat.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(L"Settings", -1, &titleFont, titleRectF, &centerFormat, &titleBrush);

    // --- Dynamic Layout Loop (Part 1: Physics & Shifter) ---
    currentY = settingsPanelRect.top + 60; // Initial Y offset
    registryIndex = 0; // Use the index from the router
    for (; registryIndex < g_settingsRegistry.size(); ++registryIndex)
    {
        auto& element = g_settingsRegistry[registryIndex];
        // Stop before Transparency section
        if (element.type == SettingType::HEADING && element.label == L"Transparency & Performance") break;

#include "UI/Settings/Settings_Registry_LoopBody.cpp"
    }

