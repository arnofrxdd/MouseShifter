        extern void DeleteProfile(int index, HWND hwnd);
        extern void DuplicateProfile(int index, HWND hwnd);
        extern void SwitchProfile(int index, HWND hwnd);

        // Handle ALL dropdown clicks first and block any other processing when dropdowns are open
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            bool handledInDropdown = false;

            // Handle profile dropdown clicks
            if (profileDropdownOpen) {
                int listItemHeight = 30;
                int itemGap = 2;
                int listHeightPadding = 8;
                int listY = profileButtonRect.top + 35; // Matches rendering offset

                RECT dropdownRect = {
                    profileButtonRect.left,
                    listY + settingsScrollOffset,
                    profileButtonRect.right,
                    listY + settingsScrollOffset + (int)((listItemHeight + itemGap) * profileNames.size()) + listHeightPadding
                };

                if (PtInRect(&dropdownRect, pt)) {
                    // Check if clicking on existing profiles
                    for (size_t i = 0; i < profileNames.size(); ++i) {
                        RECT itemRect = {
                            dropdownRect.left + 4,
                            dropdownRect.top + 4 + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right - 4,
                            dropdownRect.top + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };

                        if (PtInRect(&itemRect, pt)) {
                            // Check icons
                            int iconW = 20;
                            RECT cloneBtn = { itemRect.right - 50, itemRect.top, itemRect.right - 30, itemRect.bottom };
                            RECT deleteBtn = { itemRect.right - 25, itemRect.top, itemRect.right - 5, itemRect.bottom };

                            if (PtInRect(&cloneBtn, pt)) {
                                DuplicateProfile((int)i, hwnd);
                                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                                profileDropdownOpen = false; // Close after action? Or keep open? Let's close for clarity.
                                handledInDropdown = true;
                                break;
                            }
                            else if (PtInRect(&deleteBtn, pt)) {
                                if (profileNames.size() > 1) { // Don't delete last profile
                                    DeleteProfile((int)i, hwnd);
                                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                                }
                                profileDropdownOpen = false;
                                handledInDropdown = true;
                                break;
                            }
                            else {
                                SwitchProfile((int)i, hwnd);
                                profileDropdownOpen = false;
                                InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                                handledInDropdown = true;
                                break;
                            }
                        }
                    }
                }

                // If we clicked anywhere (inside or outside dropdown), close it and block further processing
                if (!handledInDropdown) {
                    profileDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when profile dropdown was open
            }

            // Handle gear layout dropdown clicks
            if (gearLayoutDropdownOpen)
            {
                int listItemHeight = 30;
                int itemGap = 2;
                int listHeightPadding = 8;
                int listY = gearLayoutButtonRect.top + settingsScrollOffset + 35;
                int totalHeight = (listItemHeight + itemGap) * gearLayouts.size() + listHeightPadding;

                RECT dropdownRect = {
                    gearLayoutButtonRect.left,
                    listY,
                    gearLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < gearLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left + 4,
                            listY + 4 + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right - 4,
                            listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentGearLayout = (int)i;
                            gearLabelOverride = gearLayouts[i];
                            gearLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    gearLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when gear layout dropdown was open
            }

            // Handle h-shifter layout dropdown clicks
            if (hShifterLayoutDropdownOpen)
            {
                int listItemHeight = 30;
                int itemGap = 2;
                int listHeightPadding = 8;
                int listY = hShifterLayoutButtonRect.top + settingsScrollOffset + 35;
                int totalHeight = (listItemHeight + itemGap) * hShifterLayouts.size() + listHeightPadding;

                RECT dropdownRect = {
                    hShifterLayoutButtonRect.left,
                    listY,
                    hShifterLayoutButtonRect.right,
                    listY + totalHeight
                };

                if (PtInRect(&dropdownRect, pt))
                {
                    for (size_t i = 0; i < hShifterLayouts.size(); ++i)
                    {
                        RECT itemRect = {
                            dropdownRect.left + 4,
                            listY + 4 + (LONG)(i * (listItemHeight + itemGap)),
                            dropdownRect.right - 4,
                            listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight)
                        };
                        if (PtInRect(&itemRect, pt))
                        {
                            currentHShifterLayout = hShifterLayouts[i].id;
                            layoutType = currentHShifterLayout;
                            hShifterLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                            ComputeLayout(hwnd);
                            ComputeIntersections();
                            handledInDropdown = true;
                            break;
                        }
                    }
                }

                if (!handledInDropdown) {
                    hShifterLayoutDropdownOpen = false;
                    InvalidateRect(hwnd, &settingsPanelRect, FALSE);
                }
                return 0; // Always return here when h-shifter dropdown was open
            }

            return 0; // Safety return
        }