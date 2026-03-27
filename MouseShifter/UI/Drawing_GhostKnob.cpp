    // --- Draw ghost knob (assist pointer) ---
// Draw ghost knob (assist pointer) - ONLY when assist button is HELD
    if (useAssistPointer && assistButtonHeld)
    {
        // Make ghost slightly smaller than real knob
        float ghostRadius = knobRadius * 0.70f;

        // Brighter color if gear label present
        Color ghostColor = ghostSnappedGear.empty() ? Color(150, 150, 255, 128)
            : Color(200, 200, 255, 180); // brighter and more opaque
        SolidBrush ghostBrush(ghostColor);
        Pen ghostPen(ghostColor, 2);

        Rect ghostRect(
            ghostKnobPos.x - ghostRadius,
            ghostKnobPos.y - ghostRadius,
            ghostRadius * 2,
            ghostRadius * 2);

        graphics.FillEllipse(&ghostBrush, ghostRect);
        graphics.DrawEllipse(&ghostPen, ghostRect);

        if (!ghostSnappedGear.empty())
        {
            Font font(L"Segoe UI", static_cast<INT>(ghostRadius / 2), FontStyleBold, UnitPixel);
            SolidBrush textBrush(Color(255, 255, 255, 220)); // slightly more opaque text
            std::string gearLabel = ghostSnappedGear;
            if (gearLabelOverride.find(ghostSnappedGear) != gearLabelOverride.end())
                gearLabel = gearLabelOverride[ghostSnappedGear];

            std::wstring wGear(gearLabel.begin(), gearLabel.end());

            RectF textRect(
                ghostKnobPos.x - ghostRadius,
                ghostKnobPos.y - ghostRadius / 2,
                ghostRadius * 2,
                ghostRadius);

            StringFormat format;
            format.SetAlignment(StringAlignmentCenter);
            format.SetLineAlignment(StringAlignmentCenter);
            graphics.DrawString(wGear.c_str(), -1, &font, textRect, &format, &textBrush);
        }
    }
    // --- Draw vertical Y-axis bar next to H-Shifter ---




