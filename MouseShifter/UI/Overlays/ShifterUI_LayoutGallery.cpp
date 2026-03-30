#ifndef SHIFTER_UI_LAYOUT_GALLERY_CPP
#define SHIFTER_UI_LAYOUT_GALLERY_CPP

// --- Shifter Layout Gallery Modal Fragment ---
if (showLayoutGallery)
{
    using namespace Gdiplus;
    graphics.ResetTransform();
    SolidBrush subtitleBrush(Color(180, 180, 180));

    RECT parentRect; GetClientRect(hwnd, &parentRect);
    int width = parentRect.right;
    int height = parentRect.bottom;

    // --- Dimmed Background ---
    SolidBrush dimBrush(Color(220, 0, 0, 0));
    graphics.FillRectangle(&dimBrush, 0, 0, width, height);

    // --- Gallery Modal Dimensions ---
    int modalW = 920;
    int modalH = min(580, height - 100); 
    int modalX = (width - modalW) / 2;
    int modalY = (height - modalH) / 2;

    RectF modalRectF((REAL)modalX, (REAL)modalY, (REAL)modalW, (REAL)modalH);
    layoutGalleryModalRect = { modalX, modalY, modalX + modalW, modalY + modalH };
    
    // Glass Modal Background
    GraphicsPath mPath; float mr = 20.0f;
    mPath.AddArc(modalRectF.X, modalRectF.Y, mr * 2, mr * 2, 180, 90);
    mPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y, mr * 2, mr * 2, 270, 90);
    mPath.AddArc(modalRectF.X + modalRectF.Width - mr * 2, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 0, 90);
    mPath.AddArc(modalRectF.X, modalRectF.Y + modalRectF.Height - mr * 2, mr * 2, mr * 2, 90, 90);
    mPath.CloseFigure();

    SolidBrush modalBg(Color(255, 25, 25, 25));
    graphics.FillPath(&modalBg, &mPath);
    graphics.DrawPath(&accentPen, &mPath);

    // --- Header ---
    int headerHeight = 70;
    StringFormat centerF; centerF.SetAlignment(StringAlignmentCenter); centerF.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(L"Visual Shifter Layout Gallery", -1, &headingFont, RectF(modalRectF.X, modalRectF.Y + 20, modalRectF.Width, 40), &centerF, &accentBrush);
    graphics.DrawLine(&accentPen, modalRectF.X + 100, modalRectF.Y + headerHeight - 5, modalRectF.X + modalRectF.Width - 100, modalRectF.Y + headerHeight - 5);

    // --- Footer ---
    int footerHeight = 40;
    graphics.DrawString(L"Press ESC or Click Outside to Close", -1, &rowFont, RectF(modalRectF.X, modalRectF.Y + modalRectF.Height - footerHeight, modalRectF.Width, 30), &centerF, &subtitleBrush);

    // --- Grid Metrics ---
    int cols = 3;
    int cardW = 260;
    int cardH = 140;
    int gapX = 30;
    int gapY = 25;
    
    int totalItems = (int)hShifterLayouts.size();
    int rowCountG = (totalItems + cols - 1) / cols;
    
    int contentAreaY = modalY + headerHeight;
    int contentAreaHeight = modalH - headerHeight - footerHeight;
    int totalContentHeight = rowCountG * (cardH + gapY) + gapY;
    float maxScrollGal = (float)max(0, totalContentHeight - contentAreaHeight);
    
    layoutGalleryScrollOffset += (layoutGalleryScrollTarget - layoutGalleryScrollOffset) * 0.2f;
    if (layoutGalleryScrollTarget < -maxScrollGal) layoutGalleryScrollTarget = -maxScrollGal;
    if (layoutGalleryScrollTarget > 0) layoutGalleryScrollTarget = 0;

    RectF clipRect((REAL)modalX, (REAL)contentAreaY, (REAL)modalW, (REAL)contentAreaHeight);
    graphics.SetClip(clipRect);
    graphics.TranslateTransform(0, layoutGalleryScrollOffset);

    int startX = modalX + (modalW - (cols * cardW + (cols - 1) * gapX)) / 2;
    int startY = contentAreaY + gapY;

    layoutGalleryItemRects.clear();
    POINT glMouse; GetCursorPos(&glMouse); ScreenToClient(hwnd, &glMouse);
    
    // Helper to draw mini rails
    auto DrawMiniShifter = [&](Graphics& g, RectF r, int type, bool isHovered, bool isSelected) {
        GraphicsPath cp; float cr = 10.0f;
        cp.AddArc(r.X, r.Y, cr * 2, cr * 2, 180, 90);
        cp.AddArc(r.X + r.Width - cr * 2, r.Y, cr * 2, cr * 2, 270, 90);
        cp.AddArc(r.X + r.Width - cr * 2, r.Y + r.Height - cr * 2, cr * 2, cr * 2, 0, 90);
        cp.AddArc(r.X, r.Y + r.Height - cr * 2, cr * 2, cr * 2, 90, 90);
        cp.CloseFigure();

        SolidBrush cardBrush(isHovered ? Color(60, 60, 60) : Color(40, 40, 40));
        g.FillPath(&cardBrush, &cp);
        Pen bPen(isSelected ? Color(0, 255, 170) : (isHovered ? Color(100, 100, 100) : Color(60, 60, 60)), isSelected ? 2.0f : 1.0f);
        g.DrawPath(&bPen, &cp);

        std::wstring name = hShifterLayouts[type - 1].name;
        if (name.size() > 22) name = name.substr(0, 20) + L"..";
        Font nameFont(L"Segoe UI", 11, FontStyleBold, UnitPixel);
        g.DrawString(name.c_str(), -1, &nameFont, RectF(r.X + 10, r.Y + 5, r.Width - 20, 25), &centerF, &valueBrush);

        RectF railArea(r.X + 60, r.Y + 40, r.Width - 120, r.Height - 65);
        float midY = railArea.Y + railArea.Height / 2.0f;
        Pen railPen(isSelected ? Color(0, 255, 170) : Color(150, 150, 150), 3.0f);

        int miniRailCount = 4;
        if (type == 2) miniRailCount = 3; 
        if (type == 5 || type == 6 || type == 7 || type == 8 || type == 9) miniRailCount = 3;
        if (type == 11) miniRailCount = 1;

        float railSpacing = railArea.Width / (float)max(1, miniRailCount - 1);
        if (miniRailCount == 1) railSpacing = 0;

        if (type != 11) {
            g.DrawLine(&railPen, railArea.X, midY, railArea.X + railArea.Width, midY);
        }

        for (int i = 0; i < miniRailCount; ++i) {
            float rx = (miniRailCount == 1) ? (railArea.X + railArea.Width / 2.0f) : (railArea.X + i * railSpacing);
            
            float yStart = railArea.Y;
            float yEnd = railArea.Y + railArea.Height;

            // Handle dedicated R gear rails (Half-height)
            if (type == 1 || type == 10 || type == 6 || type == 9) {
                // R is on first rail (Type 1, 6, 9) or last rail (Type 10)
                if ((i == 0 && (type == 1 || type == 6 || type == 9)) || (i == miniRailCount-1 && type == 10)) {
                    yEnd = midY; // R is Top, only draw to bottom
                }
            } else if (type == 3 || type == 4 || type == 5 || type == 8) {
                 // R is Bottom on first rail (Type 3, 8) or last rail (Type 4, 5)
                if ((i == 0 && (type == 3 || type == 8)) || (i == miniRailCount-1 && (type == 4 || type == 5))) {
                    yStart = midY; // R is Bottom, only draw from middle
                }
            }

            g.DrawLine(&railPen, rx, yStart, rx, yEnd);
        }
    };

    for (size_t i = 0; i < hShifterLayouts.size(); ++i)
    {
        int col = (int)i % cols;
        int row = (int)i / cols;
        float bx = (float)(startX + col * (cardW + gapX));
        float by = (float)(startY + row * (cardH + gapY));

        RectF cardRectF(bx, by, (REAL)cardW, (REAL)cardH);
        
        RECT cardRectWin = { (int)bx, (int)(by + layoutGalleryScrollOffset), (int)(bx + cardW), (int)(by + cardH + layoutGalleryScrollOffset) };
        layoutGalleryItemRects.push_back(cardRectWin);

        bool hovered = false;
        if (glMouse.y >= contentAreaY && glMouse.y <= contentAreaY + contentAreaHeight) {
             hovered = PtInRect(&cardRectWin, glMouse);
        }
        
        bool selected = (hShifterLayouts[i].id == currentHShifterLayout);
        DrawMiniShifter(graphics, cardRectF, hShifterLayouts[i].id, hovered, selected);
    }
    
    graphics.ResetTransform();
    graphics.ResetClip();

    // Scrollbar
    if (maxScrollGal > 0)
    {
        float sbX = modalRectF.X + modalRectF.Width - 15.0f;
        float sbY = (float)contentAreaY + 5.0f;
        float sbH = (float)contentAreaHeight - 10.0f;
        float sbW = 5.0f;
        SolidBrush sbThumbBrush(Color(120, 255, 255, 255));
        float thumbH = max(30.0f, (contentAreaHeight / (float)totalContentHeight) * sbH);
        float scrollPct = -layoutGalleryScrollOffset / maxScrollGal;
        float thumbY = sbY + scrollPct * (sbH - thumbH);
        graphics.FillRectangle(&sbThumbBrush, sbX, thumbY, sbW, thumbH);
    }
}
#endif
