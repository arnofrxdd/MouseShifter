#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "libMinHook.x64.lib")
#include "MinHook.h"

#pragma comment(lib, "Xinput9_1_0.lib")
#include <Xinput.h>

// ============================================================================
// XINPUT HOOKING - BLOCK ALL CONTROLLERS
// ============================================================================

typedef DWORD(WINAPI* XInputGetState_t)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI* XInputGetStateEx_t)(DWORD, XINPUT_STATE*);

XInputGetState_t OriginalXInputGetState = NULL;
XInputGetStateEx_t OriginalXInputGetStateEx = NULL;

// Hook for XInputGetState - BLOCK RIGHT STICK ON ALL CONTROLLERS
DWORD WINAPI HookedXInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
    DWORD result = OriginalXInputGetState(dwUserIndex, pState);

    if (result == ERROR_SUCCESS && pState) {
        pState->Gamepad.sThumbRX = 0;
        pState->Gamepad.sThumbRY = 0;
    }

    return result;
}

// Hook for XInputGetStateEx (ordinal 100) - BLOCK ALL CONTROLLERS
DWORD WINAPI HookedXInputGetStateEx(DWORD dwUserIndex, XINPUT_STATE* pState) {
    DWORD result = OriginalXInputGetStateEx(dwUserIndex, pState);

    if (result == ERROR_SUCCESS && pState) {
        pState->Gamepad.sThumbRX = 0;
        pState->Gamepad.sThumbRY = 0;
    }

    return result;
}

// ============================================================================
// HOOK INSTALLATION - ALL XINPUT VERSIONS
// ============================================================================

void HookXInputFunctions(HMODULE hXInput) {
    FARPROC getStateFunc = GetProcAddress(hXInput, "XInputGetState");
    if (getStateFunc) {
        MH_CreateHook(getStateFunc, HookedXInputGetState, (LPVOID*)&OriginalXInputGetState);
        MH_EnableHook(getStateFunc);
    }

    FARPROC ordinal100 = GetProcAddress(hXInput, (LPCSTR)100);
    if (ordinal100) {
        MH_CreateHook(ordinal100, HookedXInputGetStateEx, (LPVOID*)&OriginalXInputGetStateEx);
        MH_EnableHook(ordinal100);
    }
}

BOOL InstallHooks() {
    if (MH_Initialize() != MH_OK) {
        return FALSE;
    }

    const char* xinputVersions[] = {
        "XInput9_1_0.dll", "XInput1_4.dll", "XInput1_3.dll",
        "XInput1_2.dll", "XInput1_1.dll"
    };

    for (int i = 0; i < 5; i++) {
        HMODULE hXInput = GetModuleHandleA(xinputVersions[i]);
        if (!hXInput) hXInput = LoadLibraryA(xinputVersions[i]);

        if (hXInput) {
            HookXInputFunctions(hXInput);
        }
    }

    return TRUE;
}

// ============================================================================
// DLL MAIN
// ============================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InstallHooks, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        break;
    }
    return TRUE;
}