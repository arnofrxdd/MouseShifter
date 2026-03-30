#ifndef SHIFTER_TOP_NAV_BAR_CPP
#define SHIFTER_TOP_NAV_BAR_CPP

// --- Fragment for ShifterUI.cpp ---
{
    using namespace Gdiplus;
    
    graphics.ResetTransform();
    
    RECT rcTop; GetClientRect(hwnd, &rcTop);
    float widthTop = (float)rcTop.right;
    float heightTop = (float)rcTop.bottom;
    
    // --- 2x2 Grid Dock Dimensions (Polished) ---
    float barW = 620.0f; // Slightly wider for better text flow
    float barH = 86.0f;  // Robust height for two rows
    float barX = (widthTop - barW) / 2.0f;
    float barY = heightTop - barH - 25.0f; // 25px bottom margin
    
    topNavBarRect = { (int)barX, (int)barY, (int)(barX + barW), (int)(barY + barH) };
    
    // --- Modern Slate Dock Background ---
    GraphicsPath dockPath;
    float brTop = 10.0f; // Cleaner, more stable radius
    dockPath.AddArc(barX, barY, brTop * 2, brTop * 2, 180, 90);
    dockPath.AddArc(barX + barW - brTop * 2, barY, brTop * 2, brTop * 2, 270, 90);
    dockPath.AddArc(barX + barW - brTop * 2, barY + barH - brTop * 2, brTop * 2, brTop * 2, 0, 90);
    dockPath.AddArc(barX, barY + barH - brTop * 2, brTop * 2, brTop * 2, 90, 90);
    dockPath.CloseFigure();
    
    // Deep slate background with subtle gradient
    LinearGradientBrush dockBg(RectF(barX, barY, barW, barH), Color(250, 15, 15, 15), Color(250, 10, 10, 10), LinearGradientModeVertical);
    graphics.FillPath(&dockBg, &dockPath);
    
    // Crisp Teal Border
    Pen borderPenTop(Color(120, 0, 255, 170), 1.0f);
    graphics.DrawPath(&borderPenTop, &dockPath);
    
    float quadW = barW / 2.0f;
    float quadH = barH / 2.0f;
    POINT mouseTop; GetCursorPos(&mouseTop); ScreenToClient(hwnd, &mouseTop);

    StringFormat centerAlign; centerAlign.SetAlignment(StringAlignmentCenter); centerAlign.SetLineAlignment(StringAlignmentCenter);
    SolidBrush textBrushTop(Color(220, 220, 220));
    SolidBrush accentBrushTop(Color(0, 255, 170));

    // Helper for cell hover
    auto FillCellHover = [&](float x, float y, float w, float h) {
        GraphicsPath hp; float hr = 6.0f;
        hp.AddArc(x, y, hr * 2, hr * 2, 180, 90);
        hp.AddArc(x + w - hr * 2, y, hr * 2, hr * 2, 270, 90);
        hp.AddArc(x + w - hr * 2, y + h - hr * 2, hr * 2, hr * 2, 0, 90);
        hp.AddArc(x, y + h - hr * 2, hr * 2, hr * 2, 90, 90);
        hp.CloseFigure();
        SolidBrush hBrush(Color(25, 255, 255, 255));
        graphics.FillPath(&hBrush, &hp);
    };

    // --- TOP ROW ---
    
    // 1. Gear Label (Top Left)
    topGearLabelBtnRect = { (int)barX, (int)barY, (int)(barX + quadW), (int)(barY + quadH) };
    bool hoverLabel = PtInRect(&topGearLabelBtnRect, mouseTop);
    if (hoverLabel) FillCellHover(barX + 4, barY + 4, quadW - 6, quadH - 6);
    
    std::wstring labelDisplay = L"Gear Label: " + gearLayoutNames[currentGearLayout] + L" \x25BE";
    graphics.DrawString(labelDisplay.c_str(), -1, &rowFont, RectF(barX, barY, quadW, quadH), &centerAlign, hoverLabel ? &accentBrushTop : &textBrushTop);

    // 2. Profile (Top Right)
    topProfileBtnRect = { (int)(barX + quadW), (int)barY, (int)(barX + barW), (int)(barY + quadH) };
    bool hoverProf = PtInRect(&topProfileBtnRect, mouseTop);
    
    float plusW = 34.0f;
    topNewProfileBtnRect = { (int)(barX + barW - plusW - 10), (int)(barY + 6), (int)(barX + barW - 10), (int)(barY + quadH - 6) };
    bool hoverPlus = PtInRect(&topNewProfileBtnRect, mouseTop);

    if (hoverProf && !hoverPlus) FillCellHover(barX + quadW + 2, barY + 4, quadW - plusW - 15, quadH - 6);

    std::string pName = "Standard";
    if (!profileNames.empty() && currentProfileIndex < (int)profileNames.size()) {
        pName = profileNames[currentProfileIndex];
        if (pName.size() > 4 && pName.substr(pName.size() - 4) == ".ini") pName = pName.substr(0, pName.size() - 4);
    }
    std::wstring pNameW(pName.begin(), pName.end());
    std::wstring profDisplay = L"Profile: " + pNameW + L" \x25BE";
    
    graphics.DrawString(profDisplay.c_str(), -1, &rowFont, RectF(barX + quadW, barY, quadW - plusW - 10, quadH), &centerAlign, (hoverProf && !hoverPlus) ? &accentBrushTop : &textBrushTop);

    // Plus Button (Integrated better)
    if (hoverPlus) {
        graphics.FillEllipse(&highlightBrush, (REAL)topNewProfileBtnRect.left, (REAL)topNewProfileBtnRect.top, plusW, (REAL)(topNewProfileBtnRect.bottom - topNewProfileBtnRect.top));
    }
    graphics.DrawString(L"+", -1, &titleFont, RectF((REAL)topNewProfileBtnRect.left, (REAL)topNewProfileBtnRect.top, (REAL)plusW, (REAL)(topNewProfileBtnRect.bottom - topNewProfileBtnRect.top)), &centerAlign, hoverPlus ? &accentBrushTop : &textBrushTop);

    // --- BOTTOM ROW ---

    // 3. Gear Count (Bottom Left)
    gearCountToggleRect = { (int)barX, (int)(barY + quadH), (int)(barX + quadW), (int)(barY + barH) };
    bool hoverGear = PtInRect(&gearCountToggleRect, mouseTop);
    if (hoverGear) FillCellHover(barX + 4, barY + quadH + 2, quadW - 6, quadH - 6);
    
    std::wstring gearText = is16GearSet ? L"16 Gears \x25CF" : L"12 Gears \x25CF";
    graphics.DrawString(gearText.c_str(), -1, &rowFont, RectF(barX, barY + quadH, quadW, quadH), &centerAlign, is16GearSet ? &accentBrushTop : &textBrushTop);

    // 4. Gallery (Bottom Right)
    topLayoutBtnRect = { (int)(barX + quadW), (int)(barY + quadH), (int)(barX + barW), (int)(barY + barH) };
    bool hoverLay = PtInRect(&topLayoutBtnRect, mouseTop);
    if (hoverLay) FillCellHover(barX + quadW + 4, barY + quadH + 2, quadW - 8, quadH - 6);
    
    graphics.DrawString(L"Layout Gallery \x2637", -1, &rowFont, RectF(barX + quadW, barY + quadH, quadW, quadH), &centerAlign, hoverLay ? &accentBrushTop : &textBrushTop);

    // Refined Internal Separators (Very faint for stability)
    Pen sepPenTop(Color(30, 255, 255, 255), 1.0f);
    graphics.DrawLine(&sepPenTop, barX + quadW, barY + 12, barX + quadW, barY + barH - 12);
    graphics.DrawLine(&sepPenTop, barX + 15, barY + quadH, barX + barW - 15, barY + quadH);

}

#endif
