#ifndef UI_H
#define UI_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "app.h"

#define COLOR_BG            RGB(246, 248, 251)
#define COLOR_SIDEBAR       RGB(255, 255, 255)
#define COLOR_CARD          RGB(255, 255, 255)
#define COLOR_CARD_HOVER    RGB(240, 244, 248)
#define COLOR_BORDER        RGB(226, 232, 240)
#define COLOR_PRIMARY_NAVY  RGB(15, 23, 42)
#define COLOR_SECONDARY_TXT RGB(100, 116, 139)
#define COLOR_ACCENT_BLUE   RGB(37, 99, 235)
#define COLOR_ACCENT_TEAL   RGB(13, 148, 136)
#define COLOR_SUCCESS_GREEN RGB(22, 163, 74)
#define COLOR_WARN_AMBER    RGB(217, 119, 6)
#define COLOR_DANGER_RED    RGB(220, 38, 38)
#define COLOR_NAV_ACTIVE_BG RGB(238, 242, 255)
#define COLOR_CHART_BG      RGB(250, 252, 255)

// Custom Control IDs
#define IDC_NAV_BASE       2000
#define IDC_BTN_START_MON  2101
#define IDC_BTN_STOP_MON   2102
#define IDC_COMBO_TARGET   2103
#define IDC_COMBO_PROFILE  2104
#define IDC_EDIT_INTERVAL  2105
#define IDC_BTN_ADD_TARGET 2106
#define IDC_BTN_DEL_TARGET 2107
#define IDC_BTN_EXPORT_CSV 2108
#define IDC_BTN_ACK_ALERT  2109
#define IDC_LIST_VIEW      2110

void UI_InitGDI(HWND hwnd);
void UI_CleanupGDI(void);

void UI_DrawSidebar(HDC hdc, RECT rect);
void UI_DrawView(HDC hdc, RECT rect, AppNavSection section);

void UI_DrawCard(HDC hdc, int x, int y, int w, int h, const char *title, const char *val, const char *subtitle, COLORREF accentColor);
void UI_DrawLineChart(HDC hdc, RECT chartRect, const double *values, const bool *successes, int count, const char *title, const char *yAxisUnit);

void UI_HandleResize(HWND hwnd, int width, int height);
void UI_HandleCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

#endif /* UI_H */
