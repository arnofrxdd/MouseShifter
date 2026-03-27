    // --- Draw dynamic elements (knob and ghost knob) ---
    // Draw ghost knob (assist pointer)
    if (useAssistPointer)
    {
        float ghostRadius = knobRadius * 0.70f;
        Gdiplus::Color ghostColor = ghostSnappedGear.empty() ? Gdiplus::Color(150, 150, 255, 128)
            : Gdiplus::Color(200, 200, 255, 180);
        Gdiplus::SolidBrush ghostBrush(ghostColor);
        Gdiplus::Pen ghostPen(ghostColor, 2);

        Gdiplus::Rect ghostRect(
            ghostKnobPos.x - ghostRadius,
            ghostKnobPos.y - ghostRadius,
            ghostRadius * 2,
            ghostRadius * 2);

        graphics.FillEllipse(&ghostBrush, ghostRect);
        graphics.DrawEllipse(&ghostPen, ghostRect);

        if (!ghostSnappedGear.empty())
        {
            Gdiplus::Font font(L"Segoe UI", static_cast<INT>(ghostRadius / 2), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
            Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 220));
            std::string gearLabel = ghostSnappedGear;
            if (gearLabelOverride.find(ghostSnappedGear) != gearLabelOverride.end())
                gearLabel = gearLabelOverride[ghostSnappedGear];

            std::wstring wGear(gearLabel.begin(), gearLabel.end());

            Gdiplus::RectF textRect(
                ghostKnobPos.x - ghostRadius,
                ghostKnobPos.y - ghostRadius / 2,
                ghostRadius * 2,
                ghostRadius);

            Gdiplus::StringFormat format;
            format.SetAlignment(Gdiplus::StringAlignmentCenter);
            format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
            graphics.DrawString(wGear.c_str(), -1, &font, textRect, &format, &textBrush);
        }
    }
