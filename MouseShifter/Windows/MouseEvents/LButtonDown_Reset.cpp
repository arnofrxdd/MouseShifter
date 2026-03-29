// --- Reset Confirmation Modal Handling ---
if (showResetConfirmation)
{
    if (PtInRect(&g_modalActionRect, pt))
    {
        ResetKeybindsToDefault();
        showResetConfirmation = false;
        InvalidateRect(hwnd, NULL, FALSE);
    }
    else if (PtInRect(&g_modalCancelRect, pt))
    {
        showResetConfirmation = false;
        InvalidateRect(hwnd, NULL, FALSE);
    }
    return 0; // Consume click
}

// --- Global Reset All Button ---
if (showKeybindPanel && PtInRect(&resetAllButtonRect, pt))
{
    showResetConfirmation = true;
    InvalidateRect(hwnd, NULL, FALSE);
    return 0;
}

// --- Individual Row Reset Buttons ---
if (showKeybindPanel)
{
    for (auto const& [gear, rect] : gearResetBtnRects)
    {
        if (PtInRect(&rect, pt))
        {
            ResetGearToDefault(gear);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
    }
    
    // Check Input Map Resets
    if (showInputPanel)
    {
        for (auto const& [index, rect] : inputResetBtnRects)
        {
            if (PtInRect(&rect, pt))
            {
                ResetSpecificInput(index);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }
    }
}
