#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// --- ACCENT_POLICY and related structs/enums ---
enum ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};

struct ACCENT_POLICY
{
    ACCENT_STATE nAccentState;
    int nFlags;
    int nColor;
    int nAnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    int nAttribute;
    void* pData;
    size_t ulDataSize;
};

// --- Transparency state (Moved from AppGlobals.h) ---
extern bool isTransparent;
extern bool dynamicTransparencyEnabled;
extern BYTE currentAlpha;
extern BYTE maxAlpha;
extern BYTE minAlpha;
extern DWORD transparencyFadeDelay;
extern RECT transparencyFadeDelaySliderRect;
extern bool draggingTransparencyFadeDelaySlider;
extern RECT dynamicTransparencyToggleRect;
extern RECT transparencySliderRect;
extern RECT minTransparencySliderRect;
extern bool draggingMinTransparencySlider;
extern int yBarAlpha;
extern bool useYbarFixedTransparency;
extern RECT yBarFixedTransToggle;
extern RECT yBarAlphaSlider;
extern HWND g_yBarHwnd;

// --- Function prototypes ---
bool EnableWin11Blur(HWND hwnd, bool acrylic = false);
bool IsTransparencyEnabled();
void FadeLayeredWindow(HWND hwnd, BYTE startAlpha, BYTE endAlpha, int durationMs);
void ToggleBorderless(HWND hwnd);
void CreateYBarWindow(HWND parentHwnd);
void DestroyYBarWindow();
void UpdateYBarWindowPosition(HWND parentHwnd);
LRESULT CALLBACK YBarWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void DrawYBarOnly(HDC hdc, int width, int height);
