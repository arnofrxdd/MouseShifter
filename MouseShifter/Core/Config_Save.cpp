void SaveConfig()
{
    char buf[64];

    // Booleans
    WritePrivateProfileStringA(configSection, "NeutralEnabled", isNeutralEnabled ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "DynamicTransparency", dynamicTransparencyEnabled ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "ShowYBar", showYBar ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "ShowXBar", showXBar ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "KnobAccelerationEnabled", knobAccelerationEnabled ? "1" : "0", configFile.c_str());

    // Floats/ints
    sprintf_s(buf, "%.2f", layoutScale);
    WritePrivateProfileStringA(configSection, "LayoutScale", buf, configFile.c_str());

    sprintf_s(buf, "%d", knobRadius);
    WritePrivateProfileStringA(configSection, "KnobRadius", buf, configFile.c_str());

    sprintf_s(buf, "%d", knobRadiusMin);
    WritePrivateProfileStringA(configSection, "KnobRadiusMin", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", knobSensitivity);
    WritePrivateProfileStringA(configSection, "KnobSensitivity", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", snapSpeed);
    WritePrivateProfileStringA(configSection, "SnapSpeed", buf, configFile.c_str());
    // Diagonal & snap-in multipliers
    sprintf_s(buf, "%.2f", diagonalAssist);
    WritePrivateProfileStringA(configSection, "DiagonalAssist", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", gearSnapInMultiplier);
    WritePrivateProfileStringA(configSection, "GearSnapInMultiplier", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", controllerSensMultiplier);
    WritePrivateProfileStringA(configSection, "ControllerSensMultiplier", buf, configFile.c_str());

    sprintf_s(buf, "%d", gearRadius);
    WritePrivateProfileStringA(configSection, "GearRadius", buf, configFile.c_str());
    sprintf_s(buf, "%u", maxAlpha);
    WritePrivateProfileStringA(configSection, "MaxAlpha", buf, configFile.c_str());
    sprintf_s(buf, "%u", minAlpha);
    WritePrivateProfileStringA(configSection, "MinAlpha", buf, configFile.c_str());
    sprintf_s(buf, "%u", transparencyFadeDelay);
    WritePrivateProfileStringA(configSection, "TransparencyFadeDelay", buf, configFile.c_str());
    sprintf_s(buf, "%d", g_knobToggleKey);
    WritePrivateProfileStringA(configSection, "KnobToggleKey", buf, configFile.c_str());
    sprintf_s(buf, "%d", (int)g_knobToggleType);
    WritePrivateProfileStringA(configSection, "KnobToggleType", buf, configFile.c_str());

    WritePrivateProfileStringA(configSection, "ReverseLockEnabled", reverseLockEnabled ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%d", g_reverseUnlockKey);
    WritePrivateProfileStringA(configSection, "ReverseUnlockKey", buf, configFile.c_str());
    sprintf_s(buf, "%d", (int)g_reverseUnlockType);
    WritePrivateProfileStringA(configSection, "ReverseUnlockType", buf, configFile.c_str());

    sprintf_s(buf, "%.2f", steeringSensitivity);
    WritePrivateProfileStringA(configSection, "SteeringSensitivity", buf, configFile.c_str());
    WritePrivateProfileStringA(configSection, "RealisticKnob", realisticKnob ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%.2f", accBrakeSensitivity);
    WritePrivateProfileStringA(configSection, "AccBrakeSensitivity", buf, configFile.c_str());
    // XInput assist button
    // Save
    sprintf_s(buf, "%u", assistButton);
    WritePrivateProfileStringA(configSection, "AssistButton", buf, configFile.c_str());
    // Scroll Clutch Sensitivity
    sprintf_s(buf, "%.2f", scrollClutchSens);
    WritePrivateProfileStringA(configSection, "ScrollClutchSens", buf, configFile.c_str());

    // Invert Scroll → Clutch toggle
    WritePrivateProfileStringA(configSection, "InvertScrollClutch", invertScrollClutchAxis ? "1" : "0", configFile.c_str());
    // Smooth Scroll Speed
    sprintf_s(buf, "%.1f", smoothScrollSpeed);
    WritePrivateProfileStringA(configSection, "SmoothScrollSpeed", buf, configFile.c_str());

    // Brake Resistance Factor
    sprintf_s(buf, "%.1f", brakeresistanceFactor);
    WritePrivateProfileStringA(configSection, "BrakeResistanceFactor", buf, configFile.c_str());

    // Acceleration Resistance Factor  
    sprintf_s(buf, "%.1f", accelerationResistanceFactor);
    WritePrivateProfileStringA(configSection, "AccelerationResistanceFactor", buf, configFile.c_str());
    // Show Clutch Indicator toggle
    WritePrivateProfileStringA(configSection, "ShowClutchIndicator", showClutchIndicator ? "1" : "0", configFile.c_str());
    sprintf_s(buf, "%.0f", maxSteeringDegrees);
    WritePrivateProfileStringA(configSection, "MaxSteeringDegrees", buf, configFile.c_str());
    // Add these with the other boolean saves:
    WritePrivateProfileStringA(configSection, "UseAxisSmoothing", useAxisSmoothing ? "1" : "0", configFile.c_str());
    WritePrivateProfileStringA(configSection, "UseYbarFixedTransparency", useYbarFixedTransparency ? "1" : "0", configFile.c_str());

    // Add these with the other float/int saves:
    sprintf_s(buf, "%.2f", axisSmoothingFactor);
    WritePrivateProfileStringA(configSection, "AxisSmoothingFactor", buf, configFile.c_str());

    sprintf_s(buf, "%d", yBarAlpha); // This will save 0-255
    WritePrivateProfileStringA(configSection, "YBarAlpha", buf, configFile.c_str());
    // Keybindings (save both type and code)
    for (auto& kv : gearInputMap)
    {
        std::string typeKey = kv.first + "_Type";
        std::string codeKey = kv.first + "_Code";

        const char* typeStr = nullptr;
        switch (kv.second.type)
        {
        case KEYBOARD:
            typeStr = "KEYBOARD";
            break;
        case MOUSE:
            typeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            typeStr = "VJOY";
            break;
        }
        WritePrivateProfileStringA(configSection, typeKey.c_str(), typeStr, configFile.c_str());

        sprintf_s(buf, "%u", kv.second.code);
        WritePrivateProfileStringA(configSection, codeKey.c_str(), buf, configFile.c_str());
    }
    sprintf_s(buf, "%zu", inputMap.size());
    WritePrivateProfileStringA(configSection, "InputMapCount", buf, configFile.c_str());

    // Then, store each entry
    for (size_t i = 0; i < inputMap.size(); ++i)
    {
        std::string index = std::to_string(i);

        std::string typeKey = "InputMap_" + index + "_Type";
        std::string codeKey = "InputMap_" + index + "_Code";
        std::string vjoyKey = "InputMap_" + index + "_VJoyButton";

        const char* typeStr = nullptr;
        switch (inputMap[i].type)
        {
        case KEYBOARD:
            typeStr = "KEYBOARD";
            break;
        case MOUSE:
            typeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            typeStr = "VJOY";
            break;
        default:
            typeStr = "KEYBOARD";
            break;
        }
        WritePrivateProfileStringA(configSection, typeKey.c_str(), typeStr, configFile.c_str());

        sprintf_s(buf, "%u", inputMap[i].code);
        WritePrivateProfileStringA(configSection, codeKey.c_str(), buf, configFile.c_str());

        sprintf_s(buf, "%u", inputMap[i].vjoyButton);
        WritePrivateProfileStringA(configSection, vjoyKey.c_str(), buf, configFile.c_str());
    }
}