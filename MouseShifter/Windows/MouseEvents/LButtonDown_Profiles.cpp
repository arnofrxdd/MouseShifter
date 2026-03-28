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

        if (creatingNewProfile) {
            // Check if click is NOT in the profile creation area
            if (!PtInRect(&adjustedProfileButtonRect, pt) && !PtInRect(&adjustedCreateProfileButtonRect, pt)) {
                // Clicked outside profile creation area - create the profile
                if (!newProfileName.empty()) {
                    CreateNewProfile(hwnd);
                }
                creatingNewProfile = false;
                profileTextSelected = false;
                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                return 0; // Block further processing
            }
            // If click is inside profile creation area, continue with normal processing
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
            profileTextSelectionEnd = newProfileName.length();
            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
            return 0;
        }
