        RECT adjustedProfileButtonRect = {
            profileButtonRect.left,
            profileButtonRect.top + settingsScrollOffset,
            profileButtonRect.right,
            profileButtonRect.bottom + settingsScrollOffset
        };
        RECT adjustedCreateProfileButtonRect = {
            createProfileButtonRect.left,
            createProfileButtonRect.top + settingsScrollOffset,
            createProfileButtonRect.right,
            createProfileButtonRect.bottom + settingsScrollOffset
        };

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
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
            // (Modal is usually 400x220 in middle)
            RECT modalR = { (width - 400) / 2, (height - 220) / 2, (width + 400) / 2, (height + 220) / 2 };
            if (!PtInRect(&modalR, pt)) {
                creatingNewProfile = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0; // Block all other clicks while modal is open
        }

        // Process button clicks
        if (PtInRect(&adjustedProfileButtonRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }

        if (PtInRect(&adjustedCreateProfileButtonRect, pt)) {
            creatingNewProfile = true;
            newProfileName = "New Profile";
            profileTextSelected = true;
            profileTextSelectionStart = 0;
            profileTextSelectionEnd = (int)newProfileName.length();
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
