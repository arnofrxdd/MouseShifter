// ---------------- Window Proc ----------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        ComputeLayout(hwnd);

        ComputeIntersections();
        for (auto& inter : intersections)
        {
            char dbg[128];
            sprintf_s(dbg, "[DEBUG] diamond intersection center=(%d,%d) radius=%d\n",
                inter.x, inter.y, inter.radius);
            OutputDebugStringA(dbg);
        }

        if (!rawInputInitialized)
            InitRawInput(hwnd);
        if (!InitVJoy())
        {
            PostQuitMessage(1);
            return 0;
        }
        {
            int vJoyButtonCount = GetVJoyButtonCount(vjoyDeviceId);
            char dbg[128];
            sprintf_s(dbg, "[vJoy] Device %d has %d buttons\n", vjoyDeviceId, vJoyButtonCount);
            OutputDebugStringA(dbg);
        }
        // ----- ADD DIRECTINPUT INITIALIZATION HERE -----
        if (!InitDirectInput(hwnd))
        {
            OutputDebugStringA("[DirectInput] Failed to initialize\n");
        }
        else
        {
            if (!SetupJoystick(hwnd))
            {
                OutputDebugStringA("[DirectInput] No joystick/pedals detected\n");
            }
        }
        InitGamepads();
        SDL_InitSubSystem(SDL_INIT_EVENTS); // make sure events are enabled
        StartThrottleBrakeThread();

        SetTimer(hwnd, 1, 10, nullptr); // 10 ms = ~100 FPS max achievable
        RefreshProcessList();  // <-- ADD THIS LINE HERE
        tooltips.resize(60);
        // Update check thread - now safely scoped
        std::thread updateThread([]() {
            CheckForUpdates();
            });
        updateThread.detach();

        // SetTimer(hwnd, 1, 16, nullptr); // 60FPS
        break;
    }

#include "Handlers/WindowProc_Input.cpp"
    case WM_INPUT_DEVICE_CHANGE:
    {
        RefreshMouseDeviceList(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        // Handle gear keybind
        if (!keybindBeingSet.empty() && !showVJoyPicker)
        {
            // Set new binding
            gearInputMap[keybindBeingSet] = { KEYBOARD, (WORD)wParam };
            keybindBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        // Handle input panel keybind
        if (!inputBeingSet.empty() && !showVJoyPicker)
        {
            int idx = std::stoi(inputBeingSet);
            inputMap[idx].type = KEYBOARD;
            inputMap[idx].code = (WORD)wParam;
            inputBeingSet.clear();
            InvalidateRect(hwnd, nullptr, FALSE);
        }

        if (toggleInputBeingSet)
        {
            g_knobToggleKey = (int)wParam;
            g_knobToggleType = TOGGLE_KEYBOARD;
            toggleInputBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        if (reverseUnlockBeingSet)
        {
            g_reverseUnlockKey = (int)wParam;
            g_reverseUnlockType = TOGGLE_KEYBOARD;
            reverseUnlockBeingSet = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }


        else if (wParam == VK_TAB)
        {
            ToggleBorderless(hwnd);
        }
        if (creatingNewProfile) {
            switch (wParam) {
            case VK_LEFT:
                if (profileTextSelectionStart > 0) {
                    profileTextSelectionStart--;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_RIGHT:
                if (profileTextSelectionStart < newProfileName.length()) {
                    profileTextSelectionStart++;
                    profileTextSelectionEnd = profileTextSelectionStart;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0;

            case VK_HOME:
                profileTextSelectionStart = 0;
                profileTextSelectionEnd = 0;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_END:
                profileTextSelectionStart = newProfileName.length();
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;

            case VK_ESCAPE:
                creatingNewProfile = false;
                profileTextSelected = false;
                newProfileName = "New Profile";
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0;
            }
        }
        break;
    case WM_CHAR:
    {
        if (creatingNewProfile) {
            if (wParam == VK_RETURN) {
                // Enter pressed - create the profile
                if (!newProfileName.empty()) { // Removed the "New Profile" check
                    CreateNewProfile(hwnd);
                    creatingNewProfile = false;
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam == VK_BACK) {
                // Backspace - delete selected text or character before cursor
                if (!newProfileName.empty()) {
                    if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                        // Delete selected text
                        newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    else if (profileTextSelectionStart > 0) {
                        // Delete character before cursor
                        newProfileName.erase(profileTextSelectionStart - 1, 1);
                        profileTextSelectionStart--;
                        profileTextSelectionEnd = profileTextSelectionStart;
                    }
                    profileTextSelected = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
            }
            else if (wParam >= 32 && wParam <= 126) {
                // Printable characters - replace selection or insert
                char newChar = (char)wParam;

                if (profileTextSelected && profileTextSelectionStart < profileTextSelectionEnd) {
                    // Replace selected text
                    newProfileName.erase(profileTextSelectionStart, profileTextSelectionEnd - profileTextSelectionStart);
                    profileTextSelectionEnd = profileTextSelectionStart;
                }

                // Insert new character
                newProfileName.insert(profileTextSelectionStart, 1, newChar);
                profileTextSelectionStart++;
                profileTextSelectionEnd = profileTextSelectionStart;
                profileTextSelected = false;

                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            }
            return 0;
        }
        break;
    }
#include "Handlers/WindowProc_MouseEvents.cpp"
    case WM_CAPTURECHANGED:
    {
        // If capture was lost for any reason, stop all dragging
        g_draggingElement = nullptr;
        draggingKnobSlider = false;
        draggingSensSlider = false;
        draggingDiagSlider = false;
        draggingSnapInSlider = false;
        draggingSnapSpeedSlider = false;
        draggingLayoutSlider = false;
        isDraggingGearRadius = false;
        draggingTransparencySlider = false;
        controllerDraggingSensSlider = false;
        draggingSteeringSlider = false;
        draggingAccBrakeSlider = false;
        draggingMinTransparencySlider = false;
        draggingScrollSensSlider = false;
        draggingSteeringDegreesSlider = false;
        draggingTransparencyFadeDelaySlider = false;
        draggingAxisSmoothingFactorSlider = false;
        draggingYBarAlphaSlider = false;  // Add this

        break;
    }
    case WM_TIMER:
    {
#include "Handlers/WindowProc_Timer.cpp"
        break;
    }
    case WM_SIZE: // <-- new case added here
        ComputeLayout(hwnd);

        ComputeIntersections();

        InvalidateRect(hwnd, nullptr, FALSE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int width = rc.right;
        int height = rc.bottom;

        // Always use double buffering
        HDC memDC = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = nullptr;
        HBITMAP memBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        if (isBorderless)
        {
            // BORDERLESS MODE: Draw elements without any background
            // No background clearing - completely transparent
            DrawBorderless(memDC, width, height);
        }
        else
        {
            // WINDOWED MODE: Draw full interface
            DrawShifterGDIPlus(hwnd, memDC);
        }

        // Blit to screen
        BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_NCHITTEST:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int border = 8; // resize border thickness
        const int titleHeight = 30;

        // Top-left corner
        if (pt.x < border && pt.y < border)
            return HTTOPLEFT;
        // Top-right corner
        if (pt.x >= rc.right - border && pt.y < border)
            return HTTOPRIGHT;
        // Bottom-left corner
        if (pt.x < border && pt.y >= rc.bottom - border)
            return HTBOTTOMLEFT;
        // Bottom-right corner
        if (pt.x >= rc.right - border && pt.y >= rc.bottom - border)
            return HTBOTTOMRIGHT;

        // Top edge
        if (pt.y < border)
            return HTTOP;
        // Bottom edge
        if (pt.y >= rc.bottom - border)
            return HTBOTTOM;
        // Left edge
        if (pt.x < border)
            return HTLEFT;
        // Right edge
        if (pt.x >= rc.right - border)
            return HTRIGHT;

        // Custom title bar area (excluding buttons)
        if (!isBorderless && pt.y < titleHeight && pt.x < g_MaxButtonRect.left)
            return HTCAPTION;

        // ADD THIS: Bottom drag area (bottom 30 pixels)
        if (!isBorderless && pt.y >= rc.bottom - 30)
            return HTCAPTION;

        return HTCLIENT;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 1250; // minimum width
        mmi->ptMinTrackSize.y = 830;  // minimum height
        // optional: maximum size
        // mmi->ptMaxTrackSize.x = 1920;
        // mmi->ptMaxTrackSize.y = 1080;
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        POINT pt;
        GetCursorPos(&pt);           
        ScreenToClient(hwnd, &pt);   

        // 1. Layout Gallery Scrolling (Highest Priority Overlay)
        if (showLayoutGallery)
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            layoutGalleryScrollTarget += delta; 
            
            // Note: Bounds clamping will be handled dynamically in ShifterUI_LayoutGallery.cpp
            // since the max scroll depends on grid size and window height.
            if (layoutGalleryScrollTarget > 0) layoutGalleryScrollTarget = 0; 
            
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // 2. Process Picker Scrolling
        if (processPickerModalOpen) {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            processPickerScrollTarget += delta;
            
            int listItemHeightP = 35;
            int totalH = (int)(g_processList.size() * listItemHeightP);
            int viewportH = 300; // matching listRect.Height from TopDropdowns
            int maxScroll = max(0, totalH - viewportH);
            
            if (processPickerScrollTarget < -maxScroll) processPickerScrollTarget = (float)-maxScroll;
            if (processPickerScrollTarget > 0) processPickerScrollTarget = 0;
            
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // 2. Settings Panel Scrolling
        if (PtInRect(&settingsPanelRect, pt))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam); 
            settingsScrollTarget += delta / 2.0f;       

            if (settingsScrollTarget < -settingsScrollMax)
                settingsScrollTarget = -settingsScrollMax;
            if (settingsScrollTarget > 0)
                settingsScrollTarget = 0;

            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
        }
        // 3. Right Panel Scrolling
        else if (pt.x >= (int)panelRect.X && pt.x <= (int)(panelRect.X + panelRect.Width) &&
                 pt.y >= (int)panelRect.Y && pt.y <= (int)(panelRect.Y + panelRect.Height))
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            rightPanelScrollTarget += delta; 

            if (rightPanelScrollTarget < -rightPanelScrollMax)
                rightPanelScrollTarget = -rightPanelScrollMax;
            if (rightPanelScrollTarget > 0)
                rightPanelScrollTarget = 0;

            InvalidateRect(hwnd, NULL, FALSE); 
        }
    }
    break;


    case WM_DESTROY:
        // ============================================================================
        // UNINJECT ALL DLLs FROM TARGET PROCESSES BEFORE EXITING
        // ============================================================================

        // Uninject from currently selected process
        if (g_selectedProcessId != 0) {
            if (g_lastInjectedMouseProcessId != 0) {
                UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
                OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected\n");
            }
            if (g_lastInjectedXinputProcessId != 0) {
                UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
                OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected\n");
            }
        }

        // Also clean up from any previously injected processes
        if (g_lastInjectedMouseProcessId != 0 && g_lastInjectedMouseProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedMouseProcessId, L"RawMouseInput.dll");
            OutputDebugString(L"[Cleanup] RawMouseInput.dll uninjected from previous process\n");
        }
        if (g_lastInjectedXinputProcessId != 0 && g_lastInjectedXinputProcessId != g_selectedProcessId) {
            UninjectDLL(g_lastInjectedXinputProcessId, L"xInputBlocker.dll");
            OutputDebugString(L"[Cleanup] xInputBlocker.dll uninjected from previous process\n");
        }

        // Reset injection tracking
        g_lastInjectedMouseProcessId = 0;
        g_lastInjectedXinputProcessId = 0;
        DestroyYBarWindow();

        // If neutral key was held, release it
        if (neutralHeld)
        {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = neutralKey;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
        }

        // --- RELEASE ALL HELD BUTTONS BEFORE EXITING ---
        for (int btn = 1; btn <= vJoyButtonCount; ++btn)
        {
            SetBtn(FALSE, vjoyDeviceId, btn); // Release button
        }
        RelinquishVJD(vjoyDeviceId); // Release vJoy device

        OutputDebugString(L"[vJoy] All buttons released and device relinquished.\n");

        // ----- RELEASE DIRECTINPUT RESOURCES -----
        if (g_pJoystick)
        {
            g_pJoystick->Unacquire();
            g_pJoystick->Release();
            g_pJoystick = nullptr;
        }
        if (g_pDI)
        {
            g_pDI->Release();
            g_pDI = nullptr;
        }
        // ----------------------------------------

        // Save configuration if needed
        SaveConfig();
        StopThrottleBrakeThread();

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
