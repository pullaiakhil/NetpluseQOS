#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void UI_DrawLineChart(HDC hdc, RECT chartRect, const double *values, const bool *successes, int count, const char *title, const char *yAxisUnit) {
    // Fill chart background
    HBRUSH hBgBrush = CreateSolidBrush(COLOR_CHART_BG);
    FillRect(hdc, &chartRect, hBgBrush);
    DeleteObject(hBgBrush);

    // Subtle border
    HPEN hBorderPen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hBorderPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, chartRect.left, chartRect.top, chartRect.right, chartRect.bottom);

    // Title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_PRIMARY_NAVY);
    SelectObject(hdc, g_app.hFontBold);
    RECT titleRect = { chartRect.left + 14, chartRect.top + 10, chartRect.right - 14, chartRect.top + 32 };
    DrawTextA(hdc, title, -1, &titleRect, DT_LEFT | DT_SINGLELINE);

    int plotLeft = chartRect.left + 50;
    int plotRight = chartRect.right - 20;
    int plotTop = chartRect.top + 45;
    int plotBottom = chartRect.bottom - 30;

    if (plotRight <= plotLeft || plotBottom <= plotTop) {
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBorderPen);
        return;
    }

    // Grid lines
    HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(220, 226, 235));
    SelectObject(hdc, hGridPen);

    for (int i = 0; i <= 4; i++) {
        int y = plotTop + (plotBottom - plotTop) * i / 4;
        MoveToEx(hdc, plotLeft, y, NULL);
        LineTo(hdc, plotRight, y);
    }
    DeleteObject(hGridPen);

    if (count <= 0 || !values) {
        SelectObject(hdc, g_app.hFontBody);
        SetTextColor(hdc, COLOR_SECONDARY_TXT);
        RECT emptyRect = { plotLeft, plotTop, plotRight, plotBottom };
        DrawTextA(hdc, "No measurement data available yet — start monitoring to observe live metrics.",
                  -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBorderPen);
        return;
    }

    // Find min and max for scaling
    double minVal = 0.0;
    double maxVal = 10.0;
    bool foundFirst = false;

    for (int i = 0; i < count; i++) {
        if (successes && !successes[i]) continue;
        if (!foundFirst) {
            minVal = values[i];
            maxVal = values[i];
            foundFirst = true;
        } else {
            if (values[i] < minVal) minVal = values[i];
            if (values[i] > maxVal) maxVal = values[i];
        }
    }

    if (!foundFirst) {
        // All probes failed
        SelectObject(hdc, g_app.hFontBody);
        SetTextColor(hdc, COLOR_DANGER_RED);
        RECT emptyRect = { plotLeft, plotTop, plotRight, plotBottom };
        DrawTextA(hdc, "All recent probes timed out or encountered errors (100% loss).",
                  -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBorderPen);
        return;
    }

    // Expand bounds slightly for clean margin
    if (minVal > 0.0) minVal = 0.0;
    maxVal = maxVal * 1.25 + 1.0;

    // Draw Y-axis labels
    SelectObject(hdc, g_app.hFontSmall);
    SetTextColor(hdc, COLOR_SECONDARY_TXT);
    for (int i = 0; i <= 4; i++) {
        int y = plotTop + (plotBottom - plotTop) * (4 - i) / 4;
        double val = minVal + (maxVal - minVal) * (double)i / 4.0;
        char lbl[32];
        snprintf(lbl, sizeof(lbl), "%.1f", val);
        RECT yRect = { chartRect.left + 5, y - 8, plotLeft - 5, y + 8 };
        DrawTextA(hdc, lbl, -1, &yRect, DT_RIGHT | DT_SINGLELINE);
    }

    // Plot line
    HPEN hLinePen = CreatePen(PS_SOLID, 2, COLOR_ACCENT_BLUE);
    SelectObject(hdc, hLinePen);

    int prevX = 0, prevY = 0;
    bool hadPrev = false;

    for (int i = 0; i < count; i++) {
        int x = plotLeft + (int)((double)(plotRight - plotLeft) * (double)i / (double)(count > 1 ? count - 1 : 1));
        if (successes && !successes[i]) {
            // Drop marker for failed probe
            hadPrev = false;
            HBRUSH hDropBrush = CreateSolidBrush(COLOR_DANGER_RED);
            HPEN hDropPen = CreatePen(PS_SOLID, 1, COLOR_DANGER_RED);
            SelectObject(hdc, hDropBrush);
            SelectObject(hdc, hDropPen);
            Ellipse(hdc, x - 3, plotBottom - 6, x + 3, plotBottom);
            DeleteObject(hDropBrush);
            DeleteObject(hDropPen);
            SelectObject(hdc, hLinePen);
            continue;
        }

        double normalized = (values[i] - minVal) / (maxVal - minVal);
        if (normalized < 0.0) normalized = 0.0;
        if (normalized > 1.0) normalized = 1.0;
        int y = plotBottom - (int)(normalized * (double)(plotBottom - plotTop));

        if (!hadPrev) {
            MoveToEx(hdc, x, y, NULL);
            hadPrev = true;
        } else {
            LineTo(hdc, x, y);
        }
        prevX = x;
        prevY = y;

        // Point circle
        HBRUSH hDotBrush = CreateSolidBrush(COLOR_ACCENT_BLUE);
        SelectObject(hdc, hDotBrush);
        Ellipse(hdc, x - 2, y - 2, x + 2, y + 2);
        DeleteObject(hDotBrush);
        SelectObject(hdc, hLinePen);
    }

    DeleteObject(hLinePen);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBorderPen);
}
