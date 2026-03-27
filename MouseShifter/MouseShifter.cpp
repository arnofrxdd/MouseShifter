#include "AppGlobals.h"
#include "Core/Updater.h"
#include "UI/ShifterUI.cpp"
// Add these global tracking variables for rails
static float lastClutchNorm = 0.0f;
static int lastRedrawRailCount = 0;
static DWORD lastRailRedrawTime = 0;
#include "UI/Graphics.cpp"


// --- REVERSE LOCK FIX ---

// Helper to check unlock state (since original function might be inaccessible)
#include "Input/InputProcessing.cpp"
#include "Windows/WindowProc.cpp"
}

// ---------------- Main ----------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    using namespace Gdiplus;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // 1️⃣ Register window class

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"HShifterWinClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int winWidth = 1250;
    int winHeight = 830;

    int posX = (screenWidth - winWidth) / 2;
    int posY = (screenHeight - winHeight) / 2;
    // 2️⃣ Create window
    hwndMain = CreateWindowEx(
        0, // no WS_EX_TOPMOST here
        wc.lpszClassName,
        L"MouseShifter",
        WS_POPUP | WS_VISIBLE, // <-- no WS_OVERLAPPEDWINDOW, no default title bar
        posX, posY, winWidth, winHeight,
        nullptr, nullptr, hInst, nullptr);
    // ✨ Edit ATS controls immediately
    processAllFiles(true);

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    InitializeProfiles();

    // 3️⃣ **Load saved config**
    LoadConfig();
    LoadGearLayoutsFromIni(L"gearlayouts.ini");

    // 4️⃣ Compute layout now that settings are loaded
    ComputeLayout(hwndMain);

    ComputeIntersections();

    // 5️⃣ Initialize raw input (if needed)
    InitRawInput(hwndMain);
    InvalidateRect(hwndMain, nullptr, TRUE);
    UpdateWindow(hwndMain);
    // 6️⃣ Message loop
    MSG msg;
    bool running = true;

    while (running)
    {
        // --- Handle Windows messages (non-blocking) ---
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                running = false;
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // --- Handle SDL controller add/remove events ---
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                RefreshGamepads();
                InvalidateRect(hwndMain, nullptr, TRUE); // 🔥 force redraw
            }
        }

        // --- (optional) You can call per-frame updates here ---
        // e.g., UpdateKnobFromXInput(hwndMain);
        Sleep(1); // tiny sleep to avoid 100% CPU
    }


    // 7️⃣ **Save config before exit**
    SaveConfig();
    // ✨ Revert ATS controls to backup before closing
    processAllFiles(false);
    if (pFactory)
        pFactory->Release();
    GdiplusShutdown(gdiplusToken);

    return 0;
}

// ==== EXTRACTED MODULES (UNITY BUILD) ====
#include "Core/Updater.cpp"
#include "Core/Config.cpp"
#include "UI/Transparency.cpp"
