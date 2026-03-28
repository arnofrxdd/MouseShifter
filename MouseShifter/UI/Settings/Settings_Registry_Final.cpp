    // --- Dynamic Layout Loop (Part 3: Blocking, Steering, Indicators, Controller) ---
    for (; registryIndex < g_settingsRegistry.size(); ++registryIndex)
    {
        auto& element = g_settingsRegistry[registryIndex];
        // Skip the heading manual selector already drew
        if (element.type == SettingType::HEADING && (element.label == L"Game Target Selection" || element.label == L"Block Mouse & Look:")) continue;

#include "UI/Settings/Settings_Registry_LoopBody.cpp"
    }
