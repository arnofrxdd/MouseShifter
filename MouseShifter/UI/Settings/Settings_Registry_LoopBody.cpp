    {
        auto& element = g_settingsRegistry[registryIndex];
        bool anyDropdownOpen = profileDropdownOpen || gearLayoutDropdownOpen || hShifterLayoutDropdownOpen;
        
        // Mouse interaction for hover
        POINT cursor;
        GetCursorPos(&cursor);
        ScreenToClient(hwnd, &cursor);
        POINT adjCursor = { cursor.x, cursor.y };
        POINT itemAdjCursor = { cursor.x, cursor.y - settingsScrollOffset };

        if (element.type == SettingType::HEADING)
        {
            renderingSection = element.label;
            bool isCollapsed = g_collapsedSections[renderingSection];
            
            currentY += 30; 
            element.rect = { settingsPanelRect.left + 10, currentY, settingsPanelRect.right - 10, currentY + 40 };
            RectF headRect((REAL)element.rect.left, (REAL)(element.rect.top + settingsScrollOffset), (REAL)(element.rect.right - element.rect.left), 40.0f);
            
            bool isHovered = !anyDropdownOpen && PtInRect(&element.rect, itemAdjCursor);

            // Modern Heading Card
            GraphicsPath headPath;
            float hr = 8.0f;
            headPath.AddArc(headRect.X, headRect.Y, hr * 2, hr * 2, 180, 90);
            headPath.AddArc(headRect.X + headRect.Width - hr * 2, headRect.Y, hr * 2, hr * 2, 270, 90);
            headPath.AddArc(headRect.X + headRect.Width - hr * 2, headRect.Y + headRect.Height - hr * 2, hr * 2, hr * 2, 0, 90);
            headPath.AddArc(headRect.X, headRect.Y + headRect.Height - hr * 2, hr * 2, hr * 2, 90, 90);
            headPath.CloseFigure();

            graphics.FillPath(isHovered ? &highlightBrush : &darkBrush, &headPath);
            if (isHovered) graphics.DrawPath(&accentPen, &headPath);

            // Sleek Chevron
            float chevSize = 10.0f;
            float chevX = headRect.X + 15;
            float chevY = headRect.Y + headRect.Height / 2;
            PointF chevPts[3];
            if (isCollapsed) {
                chevPts[0] = { chevX, chevY - chevSize / 2 };
                chevPts[1] = { chevX + chevSize * 0.8f, chevY };
                chevPts[2] = { chevX, chevY + chevSize / 2 };
            } else {
                chevPts[0] = { chevX - chevSize / 2, chevY - chevSize * 0.4f };
                chevPts[1] = { chevX + chevSize / 2, chevY - chevSize * 0.4f };
                chevPts[2] = { chevX, chevY + chevSize * 0.4f };
            }
            graphics.FillPolygon(&accentBrush, chevPts, 3);
            
            StringFormat leftFormat;
            leftFormat.SetAlignment(StringAlignmentNear);
            leftFormat.SetLineAlignment(StringAlignmentCenter);
            
            RectF textRect = headRect;
            textRect.X += 40; 
            graphics.DrawString(element.label.c_str(), -1, &headingFont, textRect, &leftFormat, &labelBrush);
            
            currentY += 55;
            continue;
        }

        if (!renderingSection.empty() && g_collapsedSections[renderingSection]) continue;

        element.rect = { settingsPanelRect.left + margin, currentY, settingsPanelRect.right - margin, currentY + (element.type == SettingType::TOGGLE ? 35 : 70) };

        if (element.type == SettingType::SLIDER_INT || element.type == SettingType::SLIDER_FLOAT || element.type == SettingType::SLIDER_BYTE)
        {
            element.rect.top += 10;
            element.rect.bottom = element.rect.top + 60;

            // --- 1. Label and Reset Button ---
            PointF labelPos((REAL)element.rect.left, (REAL)(element.rect.top + settingsScrollOffset));
            graphics.DrawString(element.label.c_str(), -1, &rowFont, labelPos, &labelBrush);

            // Reset Button (Icon Style)
            float resetSize = 18.0f;
            element.resetRect = { element.rect.right - (int)resetSize - 5, element.rect.top, element.rect.right - 5, element.rect.top + (int)resetSize };
            RectF resetRectF((REAL)element.resetRect.left, (REAL)(element.resetRect.top + settingsScrollOffset), resetSize, resetSize);
            bool resetHover = !anyDropdownOpen && PtInRect(&element.resetRect, itemAdjCursor);
            
            if (resetHover) {
                graphics.FillEllipse(&highlightBrush, resetRectF);
                graphics.DrawEllipse(&accentPen, resetRectF);
            }
            
            // Draw ↺ symbol (Explicit Unicode to avoid encoding issues)
            StringFormat centerFormat; centerFormat.SetAlignment(StringAlignmentCenter); centerFormat.SetLineAlignment(StringAlignmentCenter);
            graphics.DrawString(L"\x21BA", -1, &rowFont, resetRectF, &centerFormat, resetHover ? &accentBrush : &valueBrush);

            // --- 2. Track Rendering ---
            float trackHeight = 8.0f;
            float trackY = (REAL)(element.rect.top + 30 + settingsScrollOffset);
            RectF trackRect((REAL)element.rect.left, trackY, (REAL)(element.rect.right - element.rect.left), trackHeight);
            
            GraphicsPath trackPath;
            float tr = trackHeight / 2;
            trackPath.AddArc(trackRect.X, trackRect.Y, tr * 2, tr * 2, 90, 180);
            trackPath.AddArc(trackRect.X + trackRect.Width - tr * 2, trackRect.Y, tr * 2, tr * 2, 270, 180);
            trackPath.CloseFigure();
            graphics.FillPath(&darkBrush, &trackPath);

            float t = 0.0f;
            if (element.type == SettingType::SLIDER_INT) t = (float(*(int*)element.valuePtr) - element.minVal) / (element.maxVal - element.minVal);
            else if (element.type == SettingType::SLIDER_BYTE) t = (float(*(unsigned char*)element.valuePtr) - element.minVal) / (element.maxVal - element.minVal);
            else t = (*(float*)element.valuePtr - element.minVal) / (element.maxVal - element.minVal);
            t = max(0.0f, min(1.0f, t));
            
            if (t > 0.01f)
            {
                float activeWidth = t * trackRect.Width; activeWidth = max(activeWidth, tr * 2);
                RectF activeRect(trackRect.X, trackRect.Y, activeWidth, trackHeight);
                GraphicsPath activePath;
                activePath.AddArc(activeRect.X, activeRect.Y, tr * 2, tr * 2, 90, 180);
                activePath.AddArc(activeRect.X + activeRect.Width - tr * 2, activeRect.Y, tr * 2, tr * 2, 270, 180);
                activePath.CloseFigure();
                graphics.FillPath(&accentBrush, &activePath);
            }

            // --- 3. Knob Rendering ---
            float knobR = 11.0f;
            float knobX = element.rect.left + t * (element.rect.right - element.rect.left);
            RectF knobRect(knobX - knobR, trackY + trackHeight / 2 - knobR, knobR * 2, knobR * 2);
            
            // Glow and Shadow
            if (g_draggingElement == &element || (!anyDropdownOpen && PtInRect(&element.rect, itemAdjCursor))) {
                Pen glowPen(Color(80, 0, 255, 170), 5);
                graphics.DrawEllipse(&glowPen, knobRect);
            }
            graphics.FillEllipse(&valueBrush, knobRect);
            graphics.DrawEllipse(&accentPen, knobRect);

            // --- 4. Value Display ---
            wchar_t valBuf[32];
            if (element.type == SettingType::SLIDER_INT) swprintf_s(valBuf, element.format.c_str(), *(int*)element.valuePtr);
            else if (element.type == SettingType::SLIDER_BYTE) swprintf_s(valBuf, element.format.c_str(), (int)*(unsigned char*)element.valuePtr);
            else swprintf_s(valBuf, element.format.c_str(), *(float*)element.valuePtr);
            
            RectF valRectF((REAL)element.rect.left, (REAL)(trackY + 15), (REAL)(element.rect.right - element.rect.left), 20.0f);
            graphics.DrawString(valBuf, -1, &rowFont, valRectF, &centerFormat, &valueBrush);
            
            if (element.tooltipId >= 0 && element.tooltipId < 50) { 
                tooltips[element.tooltipId].bounds = element.rect; 
                tooltips[element.tooltipId].text = element.tooltip.c_str(); 
            }
            currentY += verticalSpacing;
        }
        else if (element.type == SettingType::TOGGLE)
        {
            element.rect.bottom = element.rect.top + 35;
            bool val = *(bool*)element.valuePtr;
            if (element.label == L"Precision Knob Move") val = !val; 
            
            bool isHovered = !anyDropdownOpen && PtInRect(&element.rect, itemAdjCursor);
            if (isHovered) {
                RectF hoverRect((REAL)element.rect.left - 5, (REAL)(element.rect.top + settingsScrollOffset - 2), (REAL)(element.rect.right - element.rect.left + 10), 32.0f);
                graphics.FillRectangle(&highlightBrush, hoverRect);
            }

            PointF labelPos((REAL)element.rect.left, (REAL)(element.rect.top + settingsScrollOffset + 5));
            graphics.DrawString(element.label.c_str(), -1, &rowFont, labelPos, &labelBrush);
            
            // Modern Switch (Pill Style)
            float swHeight = 18.0f;
            float swWidth = 36.0f;
            RectF swRect(element.rect.right - swWidth - 10, (REAL)(element.rect.top + settingsScrollOffset + 6), swWidth, swHeight);
            
            GraphicsPath swPath;
            float sr = swHeight / 2;
            swPath.AddArc(swRect.X, swRect.Y, sr * 2, sr * 2, 90, 180);
            swPath.AddArc(swRect.X + swRect.Width - sr * 2, swRect.Y, sr * 2, sr * 2, 270, 180);
            swPath.CloseFigure();
            
            graphics.FillPath(val ? &accentBrush : &darkBrush, &swPath);
            graphics.DrawPath(&accentPen, &swPath);
            
            // Knob in switch
            float kR = sr - 3;
            float kX = val ? (swRect.X + swRect.Width - sr) : (swRect.X + sr);
            graphics.FillEllipse(&labelBrush, kX - kR, swRect.Y + swHeight / 2 - kR, kR * 2, kR * 2);

            if (element.tooltipId >= 0 && element.tooltipId < 50) { 
                tooltips[element.tooltipId].bounds = element.rect; 
                tooltips[element.tooltipId].text = element.tooltip.c_str(); 
            }
            currentY += toggleSpacing;
        }
    }
