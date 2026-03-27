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