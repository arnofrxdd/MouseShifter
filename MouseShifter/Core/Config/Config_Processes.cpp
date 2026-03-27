// Forward declarations
bool HasVisibleWindow(DWORD processId);
bool IsGameProcess(DWORD processId, const std::wstring& exeName);

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