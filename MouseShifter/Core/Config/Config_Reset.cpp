#include "Core/Globals/AppGlobals.h"

// Define the factory defaults for reference
std::map<std::string, GearInput> GetDefaultGearInputMap() {
    return {
        {"1", {VJOY_BUTTON, 1}},
        {"2", {VJOY_BUTTON, 2}},
        {"3", {VJOY_BUTTON, 3}},
        {"4", {VJOY_BUTTON, 4}},
        {"5", {VJOY_BUTTON, 5}},
        {"6", {VJOY_BUTTON, 6}},
        {"7", {VJOY_BUTTON, 7}},
        {"8", {VJOY_BUTTON, 8}},
        {"R", {VJOY_BUTTON, 9}},
        {"N", {VJOY_BUTTON, 10}}
    };
}

void ResetGearToDefault(std::string gear) {
    auto defaults = GetDefaultGearInputMap();
    if (defaults.find(gear) != defaults.end()) {
        gearInputMap[gear] = defaults[gear];
        SaveConfig();
    }
}

std::vector<InputToVJoy> GetDefaultInputMap() {
    return {
        { MOUSE, 1, 11 }, // Left mouse button â†’ vJoy button 11
        { KEYBOARD, 191, 12 } // '/' key â†’ vJoy button 12
    };
}

void ResetKeybindsToDefault() {
    gearInputMap = GetDefaultGearInputMap();
    inputMap = GetDefaultInputMap(); // Also reset togglers
    SaveConfig();
}

void ResetSpecificInput(int index) {
    auto defaults = GetDefaultInputMap();
    if (index >= 0 && index < (int)inputMap.size() && index < (int)defaults.size()) {
        inputMap[index] = defaults[index];
        SaveConfig();
    }
}

void ResetInputMapToDefault() {
    inputMap = GetDefaultInputMap();
    SaveConfig();
}
