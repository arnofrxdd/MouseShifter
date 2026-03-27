#include "Config.h"
#include "AppGlobals.h"

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


void RefreshProfilesList();

void InitializeProfiles() {
    std::string exeFolder = GetExeFolder();
    std::string profilesDir = exeFolder + "\\profiles";
    std::string oldConfig = exeFolder + "\\config.ini";
    std::string defaultProfile = profilesDir + "\\Default Profile.ini";

    // Create profiles directory if it doesn't exist
    CreateDirectoryA(profilesDir.c_str(), NULL);

    // Check if old config exists and move it
    if (GetFileAttributesA(oldConfig.c_str()) != INVALID_FILE_ATTRIBUTES) {
        // Move old config to profiles folder
        if (MoveFileA(oldConfig.c_str(), defaultProfile.c_str())) {
            OutputDebugStringA("[PROFILES] Migrated config.ini to profiles/Default Profile.ini\n");
        }
    }

    // Scan for existing profiles
    RefreshProfilesList();

    // Update configFile to point to current profile
    if (!profileNames.empty()) {
        configFile = profilesDir + "\\" + profileNames[currentProfileIndex];
    }
}

void RefreshProfilesList() {
    profileNames.clear();
    std::string profilesDir = GetExeFolder() + "\\profiles";

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((profilesDir + "\\*.ini").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                profileNames.push_back(findFileData.cFileName);
            }
        } while (FindNextFileA(hFind, &findFileData));
        FindClose(hFind);
    }

    // If no profiles found, create a default one
    if (profileNames.empty()) {
        std::string defaultProfile = profilesDir + "\\Default Profile.ini";
        // Create empty file
        HANDLE hFile = CreateFileA(defaultProfile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            profileNames.push_back("Default Profile.ini");
        }
    }
}

void CreateNewProfile(HWND hwnd) {
    std::string profilesDir = GetExeFolder() + "\\profiles";
    std::string newProfilePath = profilesDir + "\\" + newProfileName + ".ini";

    // Create new profile file
    HANDLE hFile = CreateFileA(newProfilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);

        // Refresh list and switch to new profile
        RefreshProfilesList();

        // Find the new profile index
        for (size_t i = 0; i < profileNames.size(); ++i) {
            if (profileNames[i] == newProfileName + ".ini") {
                currentProfileIndex = i;
                configFile = profilesDir + "\\" + profileNames[currentProfileIndex];

                // Load default settings instead of saving current ones
                LoadConfig(); // This will load default values since the file is empty

                break;
            }
        }

        creatingNewProfile = false;
        newProfileName = "New Profile";

        // Recompute layout after profile switch
        ComputeLayout(hwnd);
        ComputeIntersections();
    }
}

void SwitchProfile(int index, HWND hwnd) {
    if (index >= 0 && index < (int)profileNames.size()) {
        // Save current profile before switching
        SaveConfig();

        currentProfileIndex = index;
        std::string profilesDir = GetExeFolder() + "\\profiles";
        configFile = profilesDir + "\\" + profileNames[currentProfileIndex];
        LoadConfig();

        // Recompute layout
        ComputeLayout(hwnd);
        ComputeIntersections();
    }
}
