    // --- Dynamic Layout Loop (Part 2: Transparency Section) ---
    for (; registryIndex < g_settingsRegistry.size(); ++registryIndex)
    {
        auto& element = g_settingsRegistry[registryIndex];
        // Stop before Game Selection
        if (element.type == SettingType::HEADING && element.label == L"Block Mouse & Look:") break;

#include "UI/Settings/Settings_Registry_LoopBody.cpp"
    }
