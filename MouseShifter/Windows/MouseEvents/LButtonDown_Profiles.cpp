        RECT clientRect; GetClientRect(hwnd, &clientRect);
        int width = clientRect.right;
        int height = clientRect.bottom;

        if (creatingNewProfile) {
            // Check Modal Buttons
            if (PtInRect(&g_modalActionRect, pt)) {
                if (!newProfileName.empty()) {
                    CreateNewProfile(hwnd);
                }
                creatingNewProfile = false;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (PtInRect(&g_modalCancelRect, pt)) {
                creatingNewProfile = false;
                newProfileName = "New Profile";
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            // Clicked outside modal - ignore or close? Let's close for UX.
            RECT modalR = { (width - 400) / 2, (height - 220) / 2, (width + 400) / 2, (height + 220) / 2 };
            if (!PtInRect(&modalR, pt)) {
                creatingNewProfile = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0; // Block all other clicks while modal is open
        }

        // Process button clicks
        if (PtInRect(&topProfileBtnRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, NULL, FALSE); // Redraw for dropdown overlay
            return 0;
        }
