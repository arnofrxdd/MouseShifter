void LoadConfig()
{
    char buf[64];

    // Booleans
    GetPrivateProfileStringA(configSection, "NeutralEnabled", "0", buf, sizeof(buf), configFile.c_str());
    isNeutralEnabled = (strcmp(buf, "1") == 0);
    GetPrivateProfileStringA(
        configSection,
        "KnobAccelerationEnabled",
        "0", // Default: acceleration disabled (Precision Movement enabled)
        buf,
        sizeof(buf),
        configFile.c_str());
    knobAccelerationEnabled = (strcmp(buf, "1") == 0);
    // Booleans
    GetPrivateProfileStringA(configSection, "ShowYBar", "0", buf, sizeof(buf), configFile.c_str());
    showYBar = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "ShowXBar", "0", buf, sizeof(buf), configFile.c_str());
    showXBar = (strcmp(buf, "1") == 0);
    // Dynamic Transparency
    GetPrivateProfileStringA(
        configSection,
        "DynamicTransparency",
        "0", // default disabled
        buf,
        sizeof(buf),
        configFile.c_str());
    dynamicTransparencyEnabled = (strcmp(buf, "1") == 0);

    // Floats/ints
    GetPrivateProfileStringA(configSection, "LayoutScale", "1.75", buf, sizeof(buf), configFile.c_str());
    layoutScale = (float)atof(buf);

    GetPrivateProfileStringA(configSection, "KnobRadius", "85", buf, sizeof(buf), configFile.c_str());
    knobRadius = atoi(buf);

    GetPrivateProfileStringA(configSection, "KnobRadiusMin", "20", buf, sizeof(buf), configFile.c_str());
    knobRadiusMin = atoi(buf);

    GetPrivateProfileStringA(configSection, "KnobSensitivity", "0.45", buf, sizeof(buf), configFile.c_str());
    knobSensitivity = (float)atof(buf);
    // Load multiplier from config
    GetPrivateProfileStringA(configSection, "ControllerSensMultiplier", "1.0", buf, sizeof(buf), configFile.c_str());
    controllerSensMultiplier = (float)atof(buf);
    controllerSensMultiplier = max(0.1f, min(2.0f, controllerSensMultiplier)); // clamp

    // Update slider position to match loaded multiplier
    controllerSensSliderValue = (controllerSensMultiplier - 0.1f) / 2.0f; // inverse of mapping: 0.1-2.1 → 0-1

    GetPrivateProfileStringA(configSection, "SnapSpeed", "0.75", buf, sizeof(buf), configFile.c_str());
    snapSpeed = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "DiagonalAssist", "1.70", buf, sizeof(buf), configFile.c_str());
    diagonalAssist = (float)atof(buf);
    diagonalAssist = max(diagMin, min(diagMax, diagonalAssist)); // clamp to [0.5, 4.0]

    // Recompute dependent values
    enterVerticalThreshold = int(baseEnterVerticalThreshold * diagonalAssist);
    for (auto& inter : intersections)
    {
        inter.radius = int(baseIntersectionRadius * diagonalAssist);
    }

    GetPrivateProfileStringA(configSection, "GearSnapInMultiplier", "1.70", buf, sizeof(buf), configFile.c_str());
    gearSnapInMultiplier = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "GearRadius", "30", buf, sizeof(buf), configFile.c_str());
    gearRadius = atoi(buf);
    gearSnapInThreshold = int(gearRadius * gearSnapInMultiplier);
    GetPrivateProfileStringA(configSection, "MinAlpha", "20", buf, sizeof(buf), configFile.c_str());
    minAlpha = (BYTE)atoi(buf);
    minAlpha = max(0, min(255, minAlpha)); // clamp to valid range
    GetPrivateProfileStringA(configSection, "MaxAlpha", "150", buf, sizeof(buf), configFile.c_str());
    maxAlpha = (BYTE)atoi(buf);
    maxAlpha = max(minAlpha, min(255, maxAlpha)); // clamp to valid range
    GetPrivateProfileStringA(configSection, "TransparencyFadeDelay", "300", buf, sizeof(buf), configFile.c_str());
    transparencyFadeDelay = (DWORD)atoi(buf);
    GetPrivateProfileStringA(configSection, "SteeringSensitivity", "1.0", buf, sizeof(buf), configFile.c_str());
    steeringSensitivity = (float)atof(buf);

    GetPrivateProfileStringA(configSection, "AccBrakeSensitivity", "1.0", buf, sizeof(buf), configFile.c_str());
    accBrakeSensitivity = (float)atof(buf);
    GetPrivateProfileStringA(configSection, "RealisticKnob", "0", buf, sizeof(buf), configFile.c_str());
    realisticKnob = (strcmp(buf, "1") == 0);
    // Inside LoadConfig()
    GetPrivateProfileStringA(configSection, "KnobToggleKey", "16", buf, sizeof(buf), configFile.c_str()); // 16 = VK_SHIFT
    g_knobToggleKey = atoi(buf);
    GetPrivateProfileStringA(configSection, "KnobToggleType", "0", buf, sizeof(buf), configFile.c_str());
    g_knobToggleType = (ToggleType)atoi(buf);

    GetPrivateProfileStringA(configSection, "ReverseLockEnabled", "1", buf, sizeof(buf), configFile.c_str());
    reverseLockEnabled = (strcmp(buf, "1") == 0);
    GetPrivateProfileStringA(configSection, "ReverseUnlockKey", "16", buf, sizeof(buf), configFile.c_str()); // 16 = VK_SHIFT
    g_reverseUnlockKey = atoi(buf);
    GetPrivateProfileStringA(configSection, "ReverseUnlockType", "0", buf, sizeof(buf), configFile.c_str());
    g_reverseUnlockType = (ToggleType)atoi(buf);

    // Scroll Clutch Sensitivity
    GetPrivateProfileStringA(configSection, "ScrollClutchSens", "5.0", buf, sizeof(buf), configFile.c_str());
    scrollClutchSens = (float)atof(buf);
    scrollClutchSens = max(0.0f, min(10.0f, scrollClutchSens)); // clamp 0–10
    // Smooth Scroll Speed (range 1.0-20.0)
    GetPrivateProfileStringA(configSection, "SmoothScrollSpeed", "3.0", buf, sizeof(buf), configFile.c_str());
    smoothScrollSpeed = (float)atof(buf);
    smoothScrollSpeed = max(1.0f, min(20.0f, smoothScrollSpeed)); // clamp to valid range

    // Brake Resistance Factor (range 0-50)
    GetPrivateProfileStringA(configSection, "BrakeResistanceFactor", "15.0", buf, sizeof(buf), configFile.c_str());
    brakeresistanceFactor = (float)atof(buf);
    brakeresistanceFactor = max(0.0f, min(50.0f, brakeresistanceFactor)); // clamp to valid range

    // Acceleration Resistance Factor (range 0-50)
    GetPrivateProfileStringA(configSection, "AccelerationResistanceFactor", "5.0", buf, sizeof(buf), configFile.c_str());
    accelerationResistanceFactor = (float)atof(buf);
    accelerationResistanceFactor = max(0.0f, min(50.0f, accelerationResistanceFactor)); // clamp to valid range
    // Invert Scroll → Clutch toggle
    GetPrivateProfileStringA(configSection, "InvertScrollClutch", "0", buf, sizeof(buf), configFile.c_str());
    invertScrollClutchAxis = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "MaxSteeringDegrees", "900", buf, sizeof(buf), configFile.c_str());
    maxSteeringDegrees = (float)atof(buf);
    maxSteeringDegrees = max(90.0f, min(900.0f, maxSteeringDegrees)); // clamp 90–900
    // Add these with the other boolean loads:
    GetPrivateProfileStringA(configSection, "UseAxisSmoothing", "0", buf, sizeof(buf), configFile.c_str());
    useAxisSmoothing = (strcmp(buf, "1") == 0);

    GetPrivateProfileStringA(configSection, "UseYbarFixedTransparency", "0", buf, sizeof(buf), configFile.c_str());
    useYbarFixedTransparency = (strcmp(buf, "1") == 0);

    // Add these with the other float/int loads:
    GetPrivateProfileStringA(configSection, "AxisSmoothingFactor", "1.0", buf, sizeof(buf), configFile.c_str());
    axisSmoothingFactor = (float)atof(buf);
    axisSmoothingFactor = max(0.0f, min(2.0f, axisSmoothingFactor)); // clamp if needed

    GetPrivateProfileStringA(configSection, "YBarAlpha", "180", buf, sizeof(buf), configFile.c_str()); // Default to 180 (about 70%)
    yBarAlpha = atoi(buf);
    yBarAlpha = max(0, min(255, yBarAlpha)); // clamp to 0-255 range
    // Show Clutch Indicator toggle
    GetPrivateProfileStringA(configSection, "ShowClutchIndicator", "1", buf, sizeof(buf), configFile.c_str());
    showClutchIndicator = (strcmp(buf, "1") == 0);
    // Load
    GetPrivateProfileStringA(
        configSection,
        "AssistButton",
        "10", // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER = 10
        buf,
        sizeof(buf),
        configFile.c_str());

    assistButton = static_cast<SDL_GameControllerButton>(atoi(buf));



    // --- Load keybindings ---
    for (auto& kv : gearInputMap)
    {
        std::string typeKey = kv.first + "_Type";
        std::string codeKey = kv.first + "_Code";

        // type
        // type
        const char* defaultTypeStr = nullptr;
        switch (kv.second.type)
        {
        case KEYBOARD:
            defaultTypeStr = "KEYBOARD";
            break;
        case MOUSE:
            defaultTypeStr = "MOUSE";
            break;
        case VJOY_BUTTON:
            defaultTypeStr = "VJOY";
            break;
        default:
            defaultTypeStr = "KEYBOARD";
            break;
        }

        DWORD charsRead = GetPrivateProfileStringA(
            configSection, typeKey.c_str(), defaultTypeStr, buf, sizeof(buf), configFile.c_str());

        // Parse type string
        if (strcmp(buf, "KEYBOARD") == 0)
            kv.second.type = KEYBOARD;
        else if (strcmp(buf, "MOUSE") == 0)
            kv.second.type = MOUSE;
        else if (strcmp(buf, "VJOY") == 0)
            kv.second.type = VJOY_BUTTON;
        else
            kv.second.type = KEYBOARD; // fallback

        // code
        char defaultCode[16];
        sprintf_s(defaultCode, "%u", kv.second.code); // use current value as default
        charsRead = GetPrivateProfileStringA(
            configSection, codeKey.c_str(), defaultCode, buf, sizeof(buf), configFile.c_str());
        kv.second.code = (WORD)atoi(buf);

        std::cout << kv.first << " loaded: type=" << (kv.second.type == KEYBOARD ? "KEYBOARD" : "MOUSE")
            << " code=" << kv.second.code
            << " (charsRead=" << charsRead << ")" << std::endl;
    }

    // Clear inputMap before loading
    inputMap.clear();

    // Read the count of entries
    GetPrivateProfileStringA(configSection, "InputMapCount", "0", buf, sizeof(buf), configFile.c_str());
    size_t count = (size_t)atoi(buf);

    for (size_t i = 0; i < count; ++i)
    {
        std::string index = std::to_string(i);

        std::string typeKey = "InputMap_" + index + "_Type";
        std::string codeKey = "InputMap_" + index + "_Code";
        std::string vjoyKey = "InputMap_" + index + "_VJoyButton";

        InputToVJoy entry;
        entry.type = KEYBOARD;
        entry.code = 0;
        entry.vjoyButton = 0;

        GetPrivateProfileStringA(configSection, typeKey.c_str(), "KEYBOARD", buf, sizeof(buf), configFile.c_str());
        if (strcmp(buf, "KEYBOARD") == 0)
            entry.type = KEYBOARD;
        else if (strcmp(buf, "MOUSE") == 0)
            entry.type = MOUSE;
        else if (strcmp(buf, "VJOY") == 0)
            entry.type = VJOY_BUTTON;

        GetPrivateProfileStringA(configSection, codeKey.c_str(), "0", buf, sizeof(buf), configFile.c_str());
        entry.code = (WORD)atoi(buf);

        GetPrivateProfileStringA(configSection, vjoyKey.c_str(), "0", buf, sizeof(buf), configFile.c_str());
        entry.vjoyButton = (UINT)atoi(buf);

        inputMap.push_back(entry);
    }

    // If inputMap is empty, or missing entries, fill in fallback defaults
    if (inputMap.empty())
    {
        inputMap.push_back({ MOUSE, 1, 11 });
        inputMap.push_back({ KEYBOARD, 191, 12 });
    }
}