int currentGearLayout = 0; // index of active layout

void LoadGearLayoutsFromIni(const std::wstring& filename)
{
    gearLayouts.clear();
    gearLayoutNames.clear();

    std::wifstream file(filename);
    if (!file.is_open())
        return;

    std::wstring line;
    int layoutCount = 0;

    while (std::getline(file, line))
    {
        // Remove BOM if present
        if (!line.empty() && line[0] == 0xFEFF)
            line.erase(0, 1);

        // Trim whitespace
        line.erase(0, line.find_first_not_of(L" \t\r\n"));
        line.erase(line.find_last_not_of(L" \t\r\n") + 1);

        if (line.empty() || line[0] == L';' || line[0] == L'#')
            continue;

        if (line.find(L"count=") == 0)
        {
            layoutCount = std::stoi(line.substr(6));
        }
    }

    file.clear();
    file.seekg(0, std::ios::beg);

    for (int i = 0; i < layoutCount; ++i)
    {
        std::map<std::string, std::string> layout;
        std::wstring layoutName;
        std::wstring sectionHeader = L"[Layout" + std::to_wstring(i) + L"]";

        bool inSection = false;
        while (std::getline(file, line))
        {
            line.erase(0, line.find_first_not_of(L" \t\r\n"));
            line.erase(line.find_last_not_of(L" \t\r\n") + 1);

            if (line.empty() || line[0] == L';' || line[0] == L'#')
                continue;

            if (line == sectionHeader)
            {
                inSection = true;
                continue;
            }

            if (inSection)
            {
                if (line[0] == L'[')
                    break; // next section

                size_t eqPos = line.find(L'=');
                if (eqPos == std::wstring::npos)
                    continue;

                std::wstring key = line.substr(0, eqPos);
                std::wstring value = line.substr(eqPos + 1);

                key.erase(0, key.find_first_not_of(L" \t\r\n"));
                key.erase(key.find_last_not_of(L" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(L" \t\r\n"));
                value.erase(value.find_last_not_of(L" \t\r\n") + 1);

                if (key == L"name")
                    layoutName = value;
                else
                {
                    std::string keyStr(key.begin(), key.end());
                    std::string valueStr(value.begin(), value.end());
                    layout[keyStr] = valueStr;
                }
            }
        }

        if (!layout.empty())
        {
            gearLayouts.push_back(layout);
            gearLayoutNames.push_back(layoutName);
        }

        file.clear();
        file.seekg(0, std::ios::beg); // reset for next layout
    }
}

#include "Config.h"
// Forward declaration
bool HasVisibleWindow(DWORD processId);
bool IsGameProcess(DWORD processId, const std::wstring& exeName);

// DLL Injection functions
void RefreshProcessList()
{
    g_processList.clear();

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe))
    {
        do {
            // Check if process has a visible window AND is a game (not browser/system app)
            if (HasVisibleWindow(pe.th32ProcessID) && IsGameProcess(pe.th32ProcessID, pe.szExeFile))
            {
                g_processList.push_back(pe.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);

    // Reverse the list to invert the cycling order
}

bool IsGameProcess(DWORD processId, const std::wstring& exeName)
{
    // Convert to lowercase for case-insensitive comparison
    std::wstring lowerExe = exeName;
    std::transform(lowerExe.begin(), lowerExe.end(), lowerExe.begin(), ::towlower);

    // Exclude browsers
    static const std::vector<std::wstring> excludedBrowsers = {
        L"msedge.exe", L"chrome.exe", L"firefox.exe", L"opera.exe",
        L"safari.exe", L"brave.exe", L"vivaldi.exe", L"browser.exe"
    };

    for (const auto& browser : excludedBrowsers) {
        if (lowerExe == browser) {
            return false;
        }
    }

    // Exclude development tools and IDEs
    static const std::vector<std::wstring> excludedDevTools = {
        L"devenv.exe",          // Visual Studio
        L"vcsexpress.exe",      // Visual Studio Express
        L"wdexpress.exe",       // Visual Studio WD Express
        L"code.exe",            // VS Code
        L"vscode.exe",          // VS Code (alternative)
        L"notepad++.exe",       // Notepad++
        L"sublime_text.exe",    // Sublime Text
        L"atom.exe",            // Atom
        L"pycharm.exe",         // PyCharm
        L"idea.exe",            // IntelliJ IDEA
        L"clion.exe",           // CLion
        L"rider.exe",           // Rider
        L"webstorm.exe",        // WebStorm
        L"eclipse.exe",         // Eclipse
        L"netbeans.exe",        // NetBeans
        L"androidstudio.exe",   // Android Studio
        L"codeblocks.exe",      // Code::Blocks
        L"dev-cpp.exe",         // Dev-C++
    };

    for (const auto& devTool : excludedDevTools) {
        if (lowerExe == devTool) {
            return false;
        }
    }

    // Exclude system applications and utilities
// Exclude system applications and utilities
    static const std::vector<std::wstring> excludedSystemApps = {
        L"explorer.exe", L"taskmgr.exe", L"cmd.exe", L"powershell.exe",
        L"notepad.exe", L"calc.exe", L"mspaint.exe", L"winword.exe",
        L"excel.exe", L"powerpnt.exe", L"outlook.exe", L"teams.exe",
        L"discord.exe", L"spotify.exe", L"vlc.exe", L"photoshop.exe",
        L"MouseShifter.exe",    // Exclude your own app
        L"obs64.exe", L"obs.exe", // OBS Studio
        L"fraps.exe",           // Fraps
        L"msiafterburner.exe",  // MSI Afterburner
        L"rtss.exe",            // RivaTuner Statistics Server
        L"nvidia container.exe", // NVIDIA services
        L"geforce experience.exe", // GeForce Experience
        L"epicgameslauncher.exe", // Epic Games Launcher
        L"steam.exe",           // Steam client
        L"battle.net.exe",      // Battle.net
        L"ubisoftconnect.exe",  // Ubisoft Connect
        L"eadesktop.exe",       // EA Desktop
        L"galaxyclient.exe",    // GOG Galaxy
        L"origin.exe",          // Origin
        L"bethesdanetlauncher.exe", // Bethesda Launcher
        L"amazon games.exe",    // Amazon Games
        L"xboxapp.exe",         // Xbox App
        L"xboxpcapp.exe",       // Xbox PC App
        L"nahimic3.exe",         // Nahimic audio service
        L"amtrucks.exe",
        L"eurotrucks2.exe",
        L"ds4windows.exe",
    };
    for (const auto& app : excludedSystemApps) {
        if (lowerExe == app) {
            return false;
        }
    }

    // Exclude Windows store/apps and common non-game applications
    static const std::vector<std::wstring> excludedWindowsApps = {
        L"applicationframehost.exe", L"runtimebroker.exe", L"searchui.exe",
        L"startmenuexperiencehost.exe", L"widgets.exe", L"textinputhost.exe",
        L"lockapp.exe", L"shellexperiencehost.exe",
        L"yourphone.exe", L"phoneexperiencehost.exe",
        L"gamebar.exe", L"gamebarftserver.exe", // Xbox Game Bar
        L"gamingoverlay.exe", L"gamingservices.exe",
    };

    for (const auto& winApp : excludedWindowsApps) {
        if (lowerExe == winApp) {
            return false;
        }
    }

    // Additional check: Look for common game indicators in window titles
    struct WindowData {
        DWORD targetProcessId;
        bool isGame;
    };

    WindowData data = { processId, true }; // Assume it's a game unless proven otherwise

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);

        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);

        if (windowProcessId == data->targetProcessId && IsWindowVisible(hwnd)) {
            wchar_t title[256];
            if (GetWindowTextW(hwnd, title, 256) > 0) {
                std::wstring windowTitle = title;
                std::transform(windowTitle.begin(), windowTitle.end(), windowTitle.begin(), ::towlower);

                // Exclude windows with development/browser-like titles
                static const std::vector<std::wstring> excludedTitles = {
                    L"microsoft edge", L"google chrome", L"firefox", L"opera",
                    L"browser", L"download", L"new tab", L"settings",
                    L"mouseshifter",   // Exclude your own app window
                    L"visual studio",  // Visual Studio
                    L"vscode",         // VS Code
                    L"notepad++",      // Notepad++
                    L"sublime text",   // Sublime Text
                    L"pycharm",        // PyCharm
                    L"intellij",       // IntelliJ
                    L"eclipse",        // Eclipse
                    L"android studio", // Android Studio
                    L"steam",          // Steam client
                    L"epic games",     // Epic Games Launcher
                    L"battle.net",     // Battle.net
                    L"discord",        // Discord
                    L"spotify",        // Spotify
                    L"obs studio",     // OBS
                    L"msi afterburner", // MSI Afterburner
                    L"xbox game bar",  // Xbox Game Bar
                };

                for (const auto& excluded : excludedTitles) {
                    if (windowTitle.find(excluded) != std::wstring::npos) {
                        data->isGame = false;
                        return FALSE; // Found excluded title, stop checking
                    }
                }
            }
        }
        return TRUE; // Continue enumeration
        }, reinterpret_cast<LPARAM>(&data));

    return data.isGame;
}

bool HasVisibleWindow(DWORD processId)
{
    struct WindowData {
        DWORD targetProcessId;
        bool hasVisibleWindow;
    };

    WindowData data = { processId, false };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);

        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);

        if (windowProcessId == data->targetProcessId && IsWindowVisible(hwnd)) {
            // Less restrictive checks - focus on what makes a window "visible" to users
            LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

            // Skip tool windows and some system windows
            if (exStyle & WS_EX_TOOLWINDOW) {
                return TRUE; // Continue looking for other windows
            }

            // Get window title to filter out system windows
            wchar_t title[256];
            int titleLength = GetWindowTextW(hwnd, title, 256);

            // Skip windows with empty titles or known system windows
            if (titleLength > 0) {
                std::wstring windowTitle = title;

                // Common windows to exclude
                static const std::vector<std::wstring> excludedTitles = {
                    L"Default IME",
                    L"MSCTFIME UI",
                    L"",
                    L" "
                };

                bool shouldExclude = false;
                for (const auto& excluded : excludedTitles) {
                    if (windowTitle == excluded) {
                        shouldExclude = true;
                        break;
                    }
                }

                if (!shouldExclude) {
                    // Additional check: window should have some size
                    RECT rect;
                    if (GetWindowRect(hwnd, &rect)) {
                        int width = rect.right - rect.left;
                        int height = rect.bottom - rect.top;

                        // Consider windows with reasonable size as "visible"
                        if (width > 50 && height > 30) {
                            data->hasVisibleWindow = true;
                            // Don't return FALSE here - check all windows
                        }
                    }
                }
            }
        }
        return TRUE; // Always continue enumeration
        }, reinterpret_cast<LPARAM>(&data));

    return data.hasVisibleWindow;
}
// Global variables for both DLLs
bool g_mouseBlockEnabled = false;
bool g_xinputBlockEnabled = false;
DWORD g_lastInjectedMouseProcessId = 0;
DWORD g_lastInjectedXinputProcessId = 0;

// Track XInput blocking state based on controller input
bool g_xinputBlockTemporarilyDisabled = false;
bool g_lastAssistButtonState = false;

bool InjectDLL(DWORD processId, const wchar_t* dllPath)
{
    if (processId == 0) return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    // Allocate memory in target process
    size_t pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        CloseHandle(hProcess);
        return false;
    }

    // Write DLL path
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, pathSize, NULL)) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Create remote thread to load DLL
    LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (!pLoadLibrary) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return true;
}

bool UninjectDLL(DWORD processId, const wchar_t* dllName)
{
    if (processId == 0) return false;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);

    bool found = false;
    HMODULE hModule = NULL;

    if (Module32First(hSnapshot, &me)) {
        do {
            if (wcscmp(me.szModule, dllName) == 0) {
                found = true;
                hModule = me.hModule;
                break;
            }
        } while (Module32Next(hSnapshot, &me));
    }

    CloseHandle(hSnapshot);

    if (!found) return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    LPTHREAD_START_ROUTINE pFreeLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "FreeLibrary");
    if (!pFreeLibrary) {
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pFreeLibrary, hModule, 0, NULL);
    if (!hThread) {
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
}

// Check controller state for assist button
bool IsAssistButtonHeld()
{
    XINPUT_STATE state;
    if (XInputGetState(0, &state) == ERROR_SUCCESS) {
        bool assistButtonHeld = (state.Gamepad.wButtons & assistButton) != 0;
        return assistButtonHeld;
    }
    return false;
}

// Auto injection logic for both DLLs with controller-aware XInput blocking
// Auto injection logic for both DLLs with controller-aware XInput blocking
void UpdateAutoInjection()
{
    static bool lastMouseState = false;
    static bool lastXinputState = false;
    static bool lastKnobState = false;
    static DWORD lastCheckedProcessId = 0;

    if (g_selectedProcessId != 0) {
        // Check if the selected process is still running
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, g_selectedProcessId);
        bool processRunning = (hProcess != NULL);
        if (hProcess) CloseHandle(hProcess);

        if (!processRunning) {
            // Process is no longer running, reset
            g_lastInjectedMouseProcessId = 0;
            g_lastInjectedXinputProcessId = 0;
            lastCheckedProcessId = 0;
            return;
        }

        // ============================================================================
        // MOUSE BLOCKING LOGIC (Original knobMovementEnabled logic)
        // ============================================================================
        if (g_mouseBlockEnabled && knobMovementEnabled && g_lastInjectedMouseProcessId != g_selectedProcessId) {
            // Mouse blocking enabled + knob enabled and not injected yet - inject DLL immediately
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(NULL, dllPath, MAX_PATH);
            wchar_t* lastBackslash = wcsrchr(dllPath, L'\\');
            if (lastBackslash) {
                wcscpy_s(lastBackslash + 1, MAX_PATH - (lastBackslash - dllPath + 1), L"RawMouseInput.dll");
                if (InjectDLL(g_selectedProcessId, dllPath)) {
                    g_lastInjectedMouseProcessId = g_selectedProcessId;
                    OutputDebugString(L"[AutoInject] RawMouseInput.dll injected successfully\n");
                }
            }
        }
        else if ((!g_mouseBlockEnabled || !knobMovementEnabled) && g_lastInjectedMouseProcessId == g_selectedProcessId) {
            // Mouse blocking disabled OR knob disabled and currently injected - uninject DLL immediately
            if (UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll")) {
                OutputDebugString(L"[AutoInject] RawMouseInput.dll uninjected successfully\n");
                g_lastInjectedMouseProcessId = 0;
            }
        }

        // ============================================================================
        // XINPUT BLOCKING LOGIC (New controller-aware logic)
        // ============================================================================
        bool assistButtonHeld = IsAssistButtonHeld();
        bool shouldBlockXInput = g_xinputBlockEnabled;

        // Apply disableRealKnobMovement logic
        if (disableRealKnobMovement) {
            // DEFAULT: UNINJECTED (right stick ALLOWED)
            if (assistButtonHeld) {
                // Assist button held - INJECT DLL (block right stick)
                shouldBlockXInput = true;
            }
            else {
                // Assist button NOT held - UNINJECT DLL (allow right stick)
                shouldBlockXInput = false;
            }
        }

        // Apply invertAssistAxes logic  
        if (invertAssistAxes) {
            // DEFAULT: INJECTED (right stick BLOCKED)
            if (assistButtonHeld) {
                // Assist button held - UNINJECT DLL (allow right stick)
                shouldBlockXInput = false;
            }
            else {
                // Assist button NOT held - INJECT DLL (block right stick)
                shouldBlockXInput = true;
            }
        }

        // Handle XInput DLL injection/uninjection based on calculated state
        if (shouldBlockXInput && g_lastInjectedXinputProcessId != g_selectedProcessId) {
            // Should block and not injected yet - inject DLL
            wchar_t dllPath[MAX_PATH];
            GetModuleFileNameW(NULL, dllPath, MAX_PATH);
            wchar_t* lastBackslash = wcsrchr(dllPath, L'\\');
            if (lastBackslash) {
                wcscpy_s(lastBackslash + 1, MAX_PATH - (lastBackslash - dllPath + 1), L"xInputBlocker.dll");
                if (InjectDLL(g_selectedProcessId, dllPath)) {
                    g_lastInjectedXinputProcessId = g_selectedProcessId;
                    OutputDebugString(L"[AutoInject] xInputBlocker.dll injected (blocking right stick)\n");
                }
            }
        }
        else if (!shouldBlockXInput && g_lastInjectedXinputProcessId == g_selectedProcessId) {
            // Should not block and currently injected - uninject DLL
            if (UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll")) {
                OutputDebugString(L"[AutoInject] xInputBlocker.dll uninjected (allowing right stick)\n");
                g_lastInjectedXinputProcessId = 0;
            }
        }

        // Log state changes for debugging
        if (assistButtonHeld != g_lastAssistButtonState) {
            g_lastAssistButtonState = assistButtonHeld;
            if (assistButtonHeld) {
                OutputDebugString(L"[AutoInject] Assist button pressed\n");
            }
            else {
                OutputDebugString(L"[AutoInject] Assist button released\n");
            }
        }

        lastKnobState = knobMovementEnabled;
    }
    else {
        // No process selected, uninject both DLLs if they're injected
        if (g_lastInjectedMouseProcessId != 0) {
            UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
            g_lastInjectedMouseProcessId = 0;
        }
        if (g_lastInjectedXinputProcessId != 0) {
            UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
            g_lastInjectedXinputProcessId = 0;
        }
    }

    lastMouseState = g_mouseBlockEnabled;
    lastXinputState = g_xinputBlockEnabled;
}
// Add with your other global variables
bool g_showTooltip = false;
RECT g_tooltipBounds = { 0, 0, 0, 0 };



int settingsScrollMax = 0; // Maximum scrollable offset

// --- Right Panel Scroll variables ---
float rightPanelScrollOffsetF = 0.0f;
int rightPanelScrollOffset = 0;
float rightPanelScrollTarget = 0.0f;
int rightPanelScrollMax = 0;
const float rightPanelScrollSpeed = 0.3f;
// X-bar tracking
static int lastJoyX = 0;
static bool lastShowXBar = false;
static bool lastMouseSteeringEnabled = false;
static POINT lastKnobPos = { 0, 0 };
static int lastDrawRailCount = 0;
struct TooltipInfo {
    std::wstring text;
    RECT bounds;
    bool show;
    UINT_PTR timerId;
};

std::vector<TooltipInfo> tooltips;
TooltipInfo* currentTooltip = nullptr;
void DrawTooltip(Graphics& graphics, const TooltipInfo& tooltip, int scrollOffset, int verticalOffset = 0)
{
    if (!tooltip.show) return;

    FontFamily fontFamily(L"Segoe UI");
    Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush tooltipBgBrush(Color(30, 30, 30));
    SolidBrush tooltipTextBrush(Color(255, 255, 255));
    Pen tooltipBorderPen(Color(0, 255, 136), 1);

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    // Measure text with word wrapping to get proper height
    float maxWidth = 250.0f; // Maximum width for tooltip
    RectF textBounds;
    graphics.MeasureString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(0, 0, maxWidth - 20, 500), &format, &textBounds); // Account for padding

    // Auto-width: use text width + padding, but max 250px
    float tooltipWidth = min(textBounds.Width + 20, maxWidth);

    // Auto-height: use text height + padding
    float tooltipHeight = textBounds.Height + 16;

    // Center the tooltip horizontally within the setting panel
    float centerX = (settingsPanelRect.left + settingsPanelRect.right) / 2.0f;
    float tooltipLeft = centerX - (tooltipWidth / 2.0f);

    // Position tooltip below the setting - USE THE verticalOffset PARAMETER
    RectF tooltipRect(
        tooltipLeft,
        (REAL)(tooltip.bounds.bottom + verticalOffset + scrollOffset), // FIXED: Use verticalOffset instead of hardcoded 25
        tooltipWidth,
        tooltipHeight
    );

    // Draw tooltip
    graphics.FillRectangle(&tooltipBgBrush, tooltipRect);
    graphics.DrawRectangle(&tooltipBorderPen, tooltipRect);

    // Draw centered text with word wrapping
    graphics.DrawString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(tooltipRect.X + 8, tooltipRect.Y + 8,
            tooltipRect.Width - 16, tooltipRect.Height - 16),
        &format, &tooltipTextBrush);
}
// Add this with your other settings, probably near the top of your drawing function
// ---------------- Paint ----------------
