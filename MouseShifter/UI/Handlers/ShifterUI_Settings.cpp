        size_t registryIndex = 0;
        std::wstring renderingSection = L"";
#include "UI/Settings/Settings_VariablesAndTitles.cpp" // Part 1: Physics & Toggles

#include "UI/Settings/Settings_LayoutsAndProfiles.cpp" // Manual: Combos & Profiles

// Part 2 Start: Transparency Section + Game Heading
#include "UI/Settings/Settings_Registry_Continue.cpp"  

#include "UI/Settings/Settings_Game_Selector.cpp"      // Manual: Process Picker

#include "UI/Settings/Settings_Registry_Final.cpp"     // Part 3: Blocking Toggles + Steering + Indicators + Controller

#include "UI/Settings/Settings_VJoyAndKeybinds.cpp"    // Manual: vJoy Picker

// --- Reset clip ---
graphics.ResetClip();
}