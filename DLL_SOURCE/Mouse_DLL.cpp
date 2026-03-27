#include <windows.h>
#include <stdio.h>

// Add MinHook library - UPDATE PATH IF NEEDED
#pragma comment(lib, "libMinHook.x64.lib")

// Include MinHook - UPDATE PATH IF NEEDED  
#include "MinHook.h"

// Global variables
bool g_blockRawInput = false;

// Original function pointer
typedef UINT(WINAPI* GetRawInputData_t)(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader
    );

GetRawInputData_t OriginalGetRawInputData = NULL;

// Our hooked function
UINT WINAPI HookedGetRawInputData(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader
) {
    // Call original function first
    UINT result = OriginalGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

    if (g_blockRawInput && pData && uiCommand == RID_INPUT) {
        RAWINPUT* raw = (RAWINPUT*)pData;
        if (raw->header.dwType == RIM_TYPEMOUSE) {
            // ZERO OUT MOUSE MOVEMENT - THIS BLOCKS AIMING!
            raw->data.mouse.lLastX = 0;
            raw->data.mouse.lLastY = 0;
            raw->data.mouse.usFlags = 0; // Block relative movement

            OutputDebugStringA("[MINHOOK] BLOCKED MOUSE AIMING COMPLETELY!");
        }
    }

    return result;
}

// Set blocking state
extern "C" __declspec(dllexport) void SetBlockRawInput(bool block) {
    g_blockRawInput = block;

    char debugMsg[256];
    sprintf_s(debugMsg, "[MINHOOK] Blocking set to: %d", block);
    OutputDebugStringA(debugMsg);
}

// Install hook using MinHook
extern "C" __declspec(dllexport) void InstallHook() {
    if (MH_Initialize() != MH_OK) {
        OutputDebugStringA("[MINHOOK] Failed to initialize MinHook");
        return;
    }

    // Create hook for GetRawInputData
    if (MH_CreateHook(&GetRawInputData, &HookedGetRawInputData,
        reinterpret_cast<LPVOID*>(&OriginalGetRawInputData)) != MH_OK) {
        OutputDebugStringA("[MINHOOK] Failed to create hook");
        return;
    }

    // Enable the hook
    if (MH_EnableHook(&GetRawInputData) != MH_OK) {
        OutputDebugStringA("[MINHOOK] Failed to enable hook");
        return;
    }

    OutputDebugStringA("[MINHOOK] HOOK INSTALLED SUCCESSFULLY!");
}

// DLL Main
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        // Don't call here - wait for main thread
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_PROCESS_DETACH:
        // Cleanup
        MH_DisableHook(&GetRawInputData);
        MH_Uninitialize();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}