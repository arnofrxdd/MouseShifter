


// ETS2 AND ATS EDITOR
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h> // for FOLDERID_Documents

namespace fs = std::filesystem;

// Get Documents folder path
fs::path getDocumentsFolder()
{
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path)))
    {
        fs::path docPath(path);
        CoTaskMemFree(path);
        return docPath;
    }
    return ""; // fallback
}

// Universal game paths: ATS + ETS2
std::vector<fs::path> gamePaths = {
    getDocumentsFolder() / "American Truck Simulator",
    getDocumentsFolder() / "Euro Truck Simulator 2" };

// Lines to find & replace
std::vector<std::wstring> targetLines = {
    L"mix camuilr", L"mix camuiud", L"mix camlr", L"mix camud" };
std::vector<std::wstring> replacementLines = {
    L"mix camuilr `-mouse.rel_position.x?0 * c_msens* mouse.button_right?0`",
    L"mix camuiud `-mouse.rel_position.y?0 * sel(c_minvert, -c_msens, c_msens)* mouse.button_right?0`",
    L"mix camlr `-mouse.rel_position.x?0 * c_msens * mouse.button_right?0`",
    L"mix camud `-mouse.rel_position.y?0 * sel(c_minvert, -c_msens, c_msens) * mouse.button_right?0`" };

// Backup & edit function
#include "Core/FileBackup.cpp"

#include "Input/VJoySetup.cpp"
#include "Input/MouseInput.cpp"
bool noReverseLayout = false;
// Layout types - use one of these
// Define gear layout types
struct GearLayout {
    int id;
    std::wstring name;
};

std::vector<GearLayout> hShifterLayouts = {
    {1, L"6/8-Gear + High R Top-Left"},
    {2, L"6/8-Gear + High No Reverse"},
    {3, L"6/8-Gear + High R Bottom-Left"},
    {4, L"6/8-Gear + High R Bottom-Right"},
    {5, L"5-Gear R Bottom-Right"},
    {6, L"5-Gear R Top-Left"},
    {7, L"4-Gear R Top-Left Only"},
    {8, L"4-Gear R Bottom-Left Only"},
    {9, L"4-Gear Mixed (R Top-Left)"},
    {10, L"6/8-Gear + High R Top-Right"},
    {11, L"PRNDL (Automatic Transmission)"}
};
int currentHShifterLayout = 1; // Default to Normal Layout
int layoutType = 1; // Default to Normal Layout
bool hShifterLayoutDropdownOpen = false;
RECT hShifterLayoutButtonRect;
int hoveredGearLayoutIndex = -1;
#include "UI/Math/LayoutComputations.cpp"

#include "Core/ConfigManager.cpp"
