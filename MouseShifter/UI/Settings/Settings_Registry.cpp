#include "Settings_Registry.h"
#include <vector>
#include <functional>

// Global definition of the settings registry
std::vector<SettingElement> g_settingsRegistry;
SettingElement* g_draggingElement = nullptr;

// We will populate this from Settings_VariablesAndTitles.cpp
// but the variable itself needs to be global.
