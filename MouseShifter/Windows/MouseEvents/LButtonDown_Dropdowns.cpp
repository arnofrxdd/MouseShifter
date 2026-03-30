        extern void DeleteProfile(int index, HWND hwnd);
        extern void DuplicateProfile(int index, HWND hwnd);
        extern void SwitchProfile(int index, HWND hwnd);
        extern void RefreshProfilesList();

        // 1. Handle Visual Shifter Layout Gallery clicks
        if (showLayoutGallery) {
             for (size_t i = 0; i < layoutGalleryItemRects.size(); ++i) {
                if (PtInRect(&layoutGalleryItemRects[i], pt)) {
                    currentHShifterLayout = hShifterLayouts[i].id;
                    layoutType = currentHShifterLayout;
                    showLayoutGallery = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    ComputeLayout(hwnd);
                    ComputeIntersections();
                    return 0;
                }
             }
             // Clicked completely outside the modal? Close gallery.
             if (!PtInRect(&layoutGalleryModalRect, pt)) {
                 showLayoutGallery = false;
                 InvalidateRect(hwnd, NULL, FALSE);
             }
             return 0;
        }

        // 2. Handle ALL dropdown clicks first and block any other processing when dropdowns are open
        if (profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen)
        {
            bool handledInDropdown = false;
            int listItemHeight = 32;
            int itemGap = 2;
            int listPadding = 8;

            // Handle gear label dropdown clicks
            if (gearLayoutDropdownOpen) {
                float menuW = 180.0f;
                int totalHeight = (int)((listItemHeight + itemGap) * gearLayoutNames.size() + listPadding);
                float menuX = topGearLabelBtnRect.left + (topGearLabelBtnRect.right - topGearLabelBtnRect.left - menuW) / 2.0f;
                int listY = topGearLabelBtnRect.top - totalHeight - 5;
                RECT dropdownRect = { (int)menuX, listY, (int)(menuX + menuW), listY + totalHeight };

                if (PtInRect(&dropdownRect, pt)) {
                    for (size_t i = 0; i < gearLayoutNames.size(); ++i) {
                        RECT itemRect = { dropdownRect.left + 4, listY + 4 + (LONG)(i * (listItemHeight + itemGap)), dropdownRect.right - 4, listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight) };
                        if (PtInRect(&itemRect, pt)) {
                            currentGearLayout = (int)i;
                            gearLabelOverride = gearLayouts[i];
                            gearLayoutDropdownOpen = false;
                            InvalidateRect(hwnd, NULL, FALSE);
                            handledInDropdown = true;
                            break;
                        }
                    }
                }
                if (!handledInDropdown) { gearLayoutDropdownOpen = false; InvalidateRect(hwnd, NULL, FALSE); }
                return 0;
            }

            // Handle profile dropdown clicks
            if (profileDropdownOpen) {
                float menuW = 200.0f;
                int totalHeight = (int)((listItemHeight + itemGap) * profileNames.size() + listPadding);
                float menuX = topProfileBtnRect.left + (topProfileBtnRect.right - topProfileBtnRect.left - menuW) / 2.0f;
                int listY = topProfileBtnRect.top - totalHeight - 5;
                RECT dropdownRect = { (int)menuX, listY, (int)(menuX + menuW), listY + totalHeight };

                if (PtInRect(&dropdownRect, pt)) {
                    for (size_t i = 0; i < profileNames.size(); ++i) {
                        RECT itemRect = { dropdownRect.left + 4, listY + 4 + (LONG)(i * (listItemHeight + itemGap)), dropdownRect.right - 4, listY + 4 + (LONG)(i * (listItemHeight + itemGap) + listItemHeight) };
                        if (PtInRect(&itemRect, pt)) {
                            SwitchProfile((int)i, hwnd);
                            profileDropdownOpen = false;
                            InvalidateRect(hwnd, NULL, FALSE);
                            handledInDropdown = true;
                            break;
                        }
                    }
                }
                if (!handledInDropdown) { profileDropdownOpen = false; InvalidateRect(hwnd, NULL, FALSE); }
                return 0; 
            }
            return 0; 
        }

        // --- DOCK TOGGLES ---
        if (PtInRect(&gearCountToggleRect, pt)) {
            is16GearSet = !is16GearSet;
            InvalidateRect(hwnd, NULL, FALSE);
            ComputeLayout(hwnd);
            ComputeIntersections();
            return 0;
        }

        if (PtInRect(&topGearLabelBtnRect, pt)) {
            gearLayoutDropdownOpen = true;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        if (PtInRect(&topNewProfileBtnRect, pt)) {
            creatingNewProfile = true;
            newProfileName = "New Profile";
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        if (PtInRect(&topProfileBtnRect, pt)) {
            RefreshProfilesList();
            profileDropdownOpen = true;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        // 116: Toggle Layout Gallery
        if (PtInRect(&topLayoutBtnRect, pt)) {
            showLayoutGallery = true;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        // 3. Handle Process Picker Modal Clicks
        if (processPickerModalOpen) {
            RECT listArea = { processPickerBoxRect.left + 20, processPickerBoxRect.top + 60, processPickerBoxRect.right - 20, processPickerBoxRect.bottom - 40 };
            if (PtInRect(&listArea, pt)) {
                int listItemHeightP = 35;
                for (size_t i = 0; i < g_processList.size(); ++i) {
                    RECT itemR = { listArea.left, (LONG)(listArea.top + i * listItemHeightP + processPickerScrollOffset), listArea.right, (LONG)(listArea.top + i * listItemHeightP + processPickerScrollOffset + listItemHeightP) };
                    if (PtInRect(&itemR, pt) && PtInRect(&listArea, pt)) {
                        g_selectedProcessId = g_processList[i];
                        processPickerModalOpen = false;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            } else if (PtInRect(&g_modalActionRect, pt)) {
                RefreshProcessList();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            } else if (!PtInRect(&processPickerBoxRect, pt)) {
                processPickerModalOpen = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        // 4. Handle Mouse Device Dropdown Clicks
        if (mouseDeviceDropdownOpen) {
            RECT rS = GetScrolledRect(deviceComboRect);
            int listItemH = 32; int padding = 8;
            int totalH = (int)((listItemH + 2) * (g_mouseDevices.size() + 1) + padding);
            RECT menuRect = { rS.left, rS.bottom + 2, rS.right, rS.bottom + 2 + totalH };
            
            if (PtInRect(&menuRect, pt)) {
                for (size_t i = 0; i < g_mouseDevices.size() + 1; ++i) {
                    RECT itemR = { menuRect.left + 4, menuRect.top + 4 + (LONG)(i * 34), menuRect.right - 4, menuRect.top + 4 + (LONG)(i * 34 + 32) };
                    if (PtInRect(&itemR, pt)) {
                        g_selectedDevice = (i == 0) ? NULL : g_mouseDevices[i-1].hDevice;
                        mouseDeviceDropdownOpen = false;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            }
            mouseDeviceDropdownOpen = false;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        // 5. Handle Steering Mouse Dropdown Clicks
        if (steeringDeviceDropdownOpen) {
            RECT rS = GetScrolledRect(steeringDeviceComboRect);
            int listItemH = 32; int padding = 8;
            int totalH = (int)((listItemH + 2) * (g_mouseDevices.size() + 1) + padding);
            RECT menuRect = { rS.left, rS.bottom + 2, rS.right, rS.bottom + 2 + totalH };
            
            if (PtInRect(&menuRect, pt)) {
                for (size_t i = 0; i < g_mouseDevices.size() + 1; ++i) {
                    RECT itemR = { menuRect.left + 4, menuRect.top + 4 + (LONG)(i * 34), menuRect.right - 4, menuRect.top + 4 + (LONG)(i * 34 + 32) };
                    if (PtInRect(&itemR, pt)) {
                        g_selectedSteeringDevice = (i == 0) ? NULL : g_mouseDevices[i-1].hDevice;
                        steeringDeviceDropdownOpen = false;
                        InvalidateRect(hwnd, NULL, FALSE);
                        return 0;
                    }
                }
            }
            steeringDeviceDropdownOpen = false;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }