#pragma once
#include <string>
#include <vector>

enum class SettingType {
    SLIDER_INT,
    SLIDER_FLOAT,
    SLIDER_BYTE,
    TOGGLE,
    HEADING,
    SEPARATOR
};

struct SettingElement {
    SettingType type;
    std::wstring label;
    std::wstring tooltip;
    void* valuePtr;
    float minVal;
    float maxVal;
    float defaultValue; // For reset functionality
    std::wstring format; // e.g. L"%d" or L"%.2f"
    RECT rect;           // Interaction hitbox
    RECT resetRect;      // Hitbox for the reset button
    int tooltipId;       // Link to the legacy tooltip array if needed
};

// Global registry of all settings in the panel
extern std::vector<SettingElement> g_settingsRegistry;
extern SettingElement* g_draggingElement;
void InitializeSettingsRegistry();
