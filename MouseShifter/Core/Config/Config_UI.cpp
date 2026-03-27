bool g_showTooltip = false;
RECT g_tooltipBounds = { 0, 0, 0, 0 };



int settingsScrollMax = 0; // Maximum scrollable offset

// --- Right Panel Scroll variables ---
float rightPanelScrollOffsetF = 0.0f;
int rightPanelScrollOffset = 0;
float rightPanelScrollTarget = 0.0f;
int rightPanelScrollMax = 0;
const float rightPanelScrollSpeed = 0.3f;
// X-bar tracking
static int lastJoyX = 0;
static bool lastShowXBar = false;
static bool lastMouseSteeringEnabled = false;
static POINT lastKnobPos = { 0, 0 };
static int lastDrawRailCount = 0;
struct TooltipInfo {
    std::wstring text;
    RECT bounds;
    bool show;
    UINT_PTR timerId;
};

std::vector<TooltipInfo> tooltips;
TooltipInfo* currentTooltip = nullptr;
void DrawTooltip(Graphics& graphics, const TooltipInfo& tooltip, int scrollOffset, int verticalOffset = 0)
{
    if (!tooltip.show) return;

    FontFamily fontFamily(L"Segoe UI");
    Font tooltipFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush tooltipBgBrush(Color(30, 30, 30));
    SolidBrush tooltipTextBrush(Color(255, 255, 255));
    Pen tooltipBorderPen(Color(0, 255, 136), 1);

    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);

    // Measure text with word wrapping to get proper height
    float maxWidth = 250.0f; // Maximum width for tooltip
    RectF textBounds;
    graphics.MeasureString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(0, 0, maxWidth - 20, 500), &format, &textBounds); // Account for padding

    // Auto-width: use text width + padding, but max 250px
    float tooltipWidth = min(textBounds.Width + 20, maxWidth);

    // Auto-height: use text height + padding
    float tooltipHeight = textBounds.Height + 16;

    // Center the tooltip horizontally within the setting panel
    float centerX = (settingsPanelRect.left + settingsPanelRect.right) / 2.0f;
    float tooltipLeft = centerX - (tooltipWidth / 2.0f);

    // Position tooltip below the setting - USE THE verticalOffset PARAMETER
    RectF tooltipRect(
        tooltipLeft,
        (REAL)(tooltip.bounds.bottom + verticalOffset + scrollOffset), // FIXED: Use verticalOffset instead of hardcoded 25
        tooltipWidth,
        tooltipHeight
    );

    // Draw tooltip
    graphics.FillRectangle(&tooltipBgBrush, tooltipRect);
    graphics.DrawRectangle(&tooltipBorderPen, tooltipRect);

    // Draw centered text with word wrapping
    graphics.DrawString(tooltip.text.c_str(), -1, &tooltipFont,
        RectF(tooltipRect.X + 8, tooltipRect.Y + 8,
            tooltipRect.Width - 16, tooltipRect.Height - 16),
        &format, &tooltipTextBrush);
}
// Add this with your other settings, probably near the top of your drawing function
// ---------------- Paint ----------------
