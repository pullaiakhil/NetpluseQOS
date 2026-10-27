#include "ui.h"
#include "monitor.h"
#include "database.h"
#include "reports.h"
#include "alerts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *s_nav_names[NAV_COUNT] = {
    "Overview",
    "Live Monitoring",
    "Targets",
    "QoS Analytics",
    "Network Interfaces",
    "Alerts",
    "History",
    "Reports",
    "Settings",
    "About"
};

// Controls
static HWND s_hwndBtnStart = NULL;
static HWND s_hwndBtnStop = NULL;
static HWND s_hwndComboTarget = NULL;
static HWND s_hwndComboProfile = NULL;
static HWND s_hwndEditInterval = NULL;

static HWND s_hwndEditTName = NULL;
static HWND s_hwndEditTHost = NULL;
static HWND s_hwndComboTType = NULL;
static HWND s_hwndEditTPort = NULL;
static HWND s_hwndBtnAddTarget = NULL;

static HWND s_hwndEditExportPath = NULL;
static HWND s_hwndBtnExport = NULL;

static HWND s_hwndEditThreshRTT = NULL;
static HWND s_hwndEditThreshLoss = NULL;
static HWND s_hwndEditThreshJitter = NULL;
static HWND s_hwndBtnSaveSettings = NULL;

void UI_InitGDI(HWND hwnd) {
    // Fonts
    g_app.hFontHeading = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_app.hFontSubheading = CreateFontA(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_app.hFontBody = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_app.hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_app.hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_app.hFontMono = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    // Brushes
    g_app.hBrushBg = CreateSolidBrush(COLOR_BG);
    g_app.hBrushSidebar = CreateSolidBrush(COLOR_SIDEBAR);
    g_app.hBrushCard = CreateSolidBrush(COLOR_CARD);
    g_app.hBrushAccent = CreateSolidBrush(COLOR_ACCENT_BLUE);
    g_app.hBrushBorder = CreateSolidBrush(COLOR_BORDER);

    // Pens
    g_app.hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    g_app.hPenAccent = CreatePen(PS_SOLID, 2, COLOR_ACCENT_BLUE);
    g_app.hPenTeal = CreatePen(PS_SOLID, 2, COLOR_ACCENT_TEAL);
    g_app.hPenWarning = CreatePen(PS_SOLID, 2, COLOR_WARN_AMBER);
    g_app.hPenCritical = CreatePen(PS_SOLID, 2, COLOR_DANGER_RED);
    g_app.hPenSuccess = CreatePen(PS_SOLID, 2, COLOR_SUCCESS_GREEN);

    // Child Controls for Live Monitoring & Forms
    s_hwndBtnStart = CreateWindowExA(0, "BUTTON", "Start Monitoring",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 130, 32, hwnd, (HMENU)IDC_BTN_START_MON, g_app.hInstance, NULL);
    SendMessage(s_hwndBtnStart, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndBtnStop = CreateWindowExA(0, "BUTTON", "Stop",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 80, 32, hwnd, (HMENU)IDC_BTN_STOP_MON, g_app.hInstance, NULL);
    SendMessage(s_hwndBtnStop, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndComboTarget = CreateWindowExA(0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        0, 0, 200, 200, hwnd, (HMENU)IDC_COMBO_TARGET, g_app.hInstance, NULL);
    SendMessage(s_hwndComboTarget, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndComboProfile = CreateWindowExA(0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        0, 0, 170, 200, hwnd, (HMENU)IDC_COMBO_PROFILE, g_app.hInstance, NULL);
    SendMessage(s_hwndComboProfile, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    SendMessageA(s_hwndComboProfile, CB_ADDSTRING, 0, (LPARAM)"video_conferencing");
    SendMessageA(s_hwndComboProfile, CB_ADDSTRING, 0, (LPARAM)"voip");
    SendMessageA(s_hwndComboProfile, CB_ADDSTRING, 0, (LPARAM)"gaming");
    SendMessageA(s_hwndComboProfile, CB_ADDSTRING, 0, (LPARAM)"browsing");
    SendMessageA(s_hwndComboProfile, CB_ADDSTRING, 0, (LPARAM)"file_download");
    SendMessageA(s_hwndComboProfile, CB_SETCURSEL, 0, 0);

    s_hwndEditInterval = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1.0",
        WS_CHILD | WS_VISIBLE | ES_LEFT,
        0, 0, 60, 26, hwnd, (HMENU)IDC_EDIT_INTERVAL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditInterval, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    // Target Management Inputs
    s_hwndEditTName = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Cloudflare DNS",
        WS_CHILD | ES_LEFT, 0, 0, 160, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditTName, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndEditTHost = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1.1.1.1",
        WS_CHILD | ES_LEFT, 0, 0, 160, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditTHost, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndComboTType = CreateWindowExA(0, "COMBOBOX", "",
        WS_CHILD | CBS_DROPDOWNLIST, 0, 0, 100, 100, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndComboTType, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);
    SendMessageA(s_hwndComboTType, CB_ADDSTRING, 0, (LPARAM)"icmp");
    SendMessageA(s_hwndComboTType, CB_ADDSTRING, 0, (LPARAM)"tcp");
    SendMessageA(s_hwndComboTType, CB_SETCURSEL, 0, 0);

    s_hwndEditTPort = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "80",
        WS_CHILD | ES_LEFT, 0, 0, 60, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditTPort, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndBtnAddTarget = CreateWindowExA(0, "BUTTON", "+ Add Target",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 110, 28, hwnd, (HMENU)IDC_BTN_ADD_TARGET, g_app.hInstance, NULL);
    SendMessage(s_hwndBtnAddTarget, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    // Reports inputs
    s_hwndEditExportPath = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "netpulse_export.csv",
        WS_CHILD | ES_LEFT, 0, 0, 280, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditExportPath, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndBtnExport = CreateWindowExA(0, "BUTTON", "Export CSV",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 110, 28, hwnd, (HMENU)IDC_BTN_EXPORT_CSV, g_app.hInstance, NULL);
    SendMessage(s_hwndBtnExport, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    // Settings inputs
    s_hwndEditThreshRTT = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "150.0",
        WS_CHILD | ES_LEFT, 0, 0, 90, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditThreshRTT, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndEditThreshLoss = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "5.0",
        WS_CHILD | ES_LEFT, 0, 0, 90, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditThreshLoss, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndEditThreshJitter = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "30.0",
        WS_CHILD | ES_LEFT, 0, 0, 90, 26, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndEditThreshJitter, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    s_hwndBtnSaveSettings = CreateWindowExA(0, "BUTTON", "Save Alert Settings",
        WS_CHILD | BS_PUSHBUTTON, 0, 0, 150, 28, hwnd, NULL, g_app.hInstance, NULL);
    SendMessage(s_hwndBtnSaveSettings, WM_SETFONT, (WPARAM)g_app.hFontBody, TRUE);

    // Populate targets in combobox
    DbTarget targets[32];
    int tcount = 0;
    if (DB_GetTargets(targets, 32, &tcount)) {
        for (int i = 0; i < tcount; i++) {
            char label[256];
            snprintf(label, sizeof(label), "%s (%s [%s])", targets[i].name, targets[i].host, targets[i].target_type);
            SendMessageA(s_hwndComboTarget, CB_ADDSTRING, 0, (LPARAM)label);
        }
        if (tcount > 0) {
            SendMessageA(s_hwndComboTarget, CB_SETCURSEL, 0, 0);
        }
    }
}

void UI_CleanupGDI(void) {
    if (g_app.hFontHeading) DeleteObject(g_app.hFontHeading);
    if (g_app.hFontSubheading) DeleteObject(g_app.hFontSubheading);
    if (g_app.hFontBody) DeleteObject(g_app.hFontBody);
    if (g_app.hFontBold) DeleteObject(g_app.hFontBold);
    if (g_app.hFontSmall) DeleteObject(g_app.hFontSmall);
    if (g_app.hFontMono) DeleteObject(g_app.hFontMono);

    if (g_app.hBrushBg) DeleteObject(g_app.hBrushBg);
    if (g_app.hBrushSidebar) DeleteObject(g_app.hBrushSidebar);
    if (g_app.hBrushCard) DeleteObject(g_app.hBrushCard);
    if (g_app.hBrushAccent) DeleteObject(g_app.hBrushAccent);
    if (g_app.hBrushBorder) DeleteObject(g_app.hBrushBorder);

    if (g_app.hPenBorder) DeleteObject(g_app.hPenBorder);
    if (g_app.hPenAccent) DeleteObject(g_app.hPenAccent);
    if (g_app.hPenTeal) DeleteObject(g_app.hPenTeal);
    if (g_app.hPenWarning) DeleteObject(g_app.hPenWarning);
    if (g_app.hPenCritical) DeleteObject(g_app.hPenCritical);
    if (g_app.hPenSuccess) DeleteObject(g_app.hPenSuccess);
}

void UI_DrawSidebar(HDC hdc, RECT rect) {
    FillRect(hdc, &rect, g_app.hBrushSidebar);

    // Right border
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
    MoveToEx(hdc, rect.right - 1, rect.top, NULL);
    LineTo(hdc, rect.right - 1, rect.bottom);

    // App Brand Logo & Title
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, g_app.hFontHeading);
    SetTextColor(hdc, COLOR_PRIMARY_NAVY);
    RECT brandRect = { rect.left + 20, rect.top + 20, rect.right - 20, rect.top + 50 };
    DrawTextA(hdc, "NetPulse QoS", -1, &brandRect, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, g_app.hFontSmall);
    SetTextColor(hdc, COLOR_SECONDARY_TXT);
    RECT tagRect = { rect.left + 20, rect.top + 50, rect.right - 20, rect.top + 70 };
    DrawTextA(hdc, "Native SLA Observability", -1, &tagRect, DT_LEFT | DT_SINGLELINE);

    // Divider line
    MoveToEx(hdc, rect.left + 15, rect.top + 78, NULL);
    LineTo(hdc, rect.right - 15, rect.top + 78);

    // Nav Items
    int itemY = rect.top + 92;
    int itemHeight = 38;

    for (int i = 0; i < NAV_COUNT; i++) {
        RECT itemRect = { rect.left + 12, itemY, rect.right - 12, itemY + itemHeight };

        if ((int)g_app.currentSection == i) {
            // Active background pill
            HBRUSH hActiveBrush = CreateSolidBrush(COLOR_NAV_ACTIVE_BG);
            HPEN hActivePen = CreatePen(PS_SOLID, 1, RGB(199, 210, 254));
            HBRUSH hPrevB = (HBRUSH)SelectObject(hdc, hActiveBrush);
            HPEN hPrevP = (HPEN)SelectObject(hdc, hActivePen);
            RoundRect(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, 8, 8);
            SelectObject(hdc, hPrevB);
            SelectObject(hdc, hPrevP);
            DeleteObject(hActiveBrush);
            DeleteObject(hActivePen);

            // Left accent indicator
            HBRUSH hIndicatorBrush = CreateSolidBrush(COLOR_ACCENT_BLUE);
            RECT indRect = { itemRect.left + 2, itemRect.top + 6, itemRect.left + 6, itemRect.bottom - 6 };
            FillRect(hdc, &indRect, hIndicatorBrush);
            DeleteObject(hIndicatorBrush);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_ACCENT_BLUE);
        } else {
            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
        }

        RECT textRect = { itemRect.left + 16, itemRect.top, itemRect.right - 10, itemRect.bottom };
        DrawTextA(hdc, s_nav_names[i], -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        itemY += itemHeight + 4;
    }

    SelectObject(hdc, hOldPen);
}

void UI_DrawCard(HDC hdc, int x, int y, int w, int h, const char *title, const char *val, const char *subtitle, COLORREF accentColor) {
    RECT cardRect = { x, y, x + w, y + h };

    // Fill white card
    FillRect(hdc, &cardRect, g_app.hBrushCard);

    // Subtle border
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, x, y, x + w, y + h, 8, 8);

    // Left accent stripe
    HBRUSH hStripe = CreateSolidBrush(accentColor);
    RECT stripeRect = { x + 1, y + 8, x + 5, y + h - 8 };
    FillRect(hdc, &stripeRect, hStripe);
    DeleteObject(hStripe);

    // Title
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_SECONDARY_TXT);
    SelectObject(hdc, g_app.hFontSmall);
    RECT tRect = { x + 16, y + 10, x + w - 16, y + 26 };
    DrawTextA(hdc, title, -1, &tRect, DT_LEFT | DT_SINGLELINE);

    // Value
    SetTextColor(hdc, COLOR_PRIMARY_NAVY);
    SelectObject(hdc, g_app.hFontHeading);
    RECT vRect = { x + 16, y + 28, x + w - 16, y + 58 };
    DrawTextA(hdc, val, -1, &vRect, DT_LEFT | DT_SINGLELINE);

    // Subtitle
    SetTextColor(hdc, COLOR_SECONDARY_TXT);
    SelectObject(hdc, g_app.hFontSmall);
    RECT sRect = { x + 16, y + 58, x + w - 16, y + 76 };
    DrawTextA(hdc, subtitle, -1, &sRect, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
}

static void UI_DrawOverview(HDC hdc, RECT rect) {
    bool is_running = false;
    int session_id = 0;
    DbTarget target;
    ProbeSample samples[MAX_SAMPLES_BUFFER];
    int count = 0;
    QoSMetrics metrics;
    QoSAssessmentResult assess;
    InterfaceSnapshot if_stats;

    Monitor_GetSnapshot(samples, MAX_SAMPLES_BUFFER, &count, &metrics, &assess, &if_stats, &target, &is_running, &session_id);

    // Header bar
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, g_app.hFontHeading);
    SetTextColor(hdc, COLOR_PRIMARY_NAVY);
    RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
    DrawTextA(hdc, "Executive QoS Overview", -1, &hRect, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, g_app.hFontBody);
    SetTextColor(hdc, COLOR_SECONDARY_TXT);
    char statusStr[256];
    if (is_running) {
        snprintf(statusStr, sizeof(statusStr), "● Monitoring ACTIVE — Target: %s (%s) | Session #%d | Profile: %s",
                 target.name, target.host, session_id, assess.profile_name);
    } else {
        snprintf(statusStr, sizeof(statusStr), "○ Idle — Select a target and start monitoring to view genuine live statistics.");
    }
    RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
    DrawTextA(hdc, statusStr, -1, &subRect, DT_LEFT | DT_SINGLELINE);

    // Summary KPI Cards (Row 1)
    int cardY = rect.top + 86;
    int cardW = (rect.right - rect.left - 48 - 36) / 4;
    int cardH = 88;

    char rttVal[32] = "-- ms", rttSub[64] = "Min / Max: --";
    char lossVal[32] = "-- %", lossSub[64] = "Probes: 0 tx / 0 rx";
    char jitVal[32] = "-- ms", jitSub[64] = "RTT Variation (RFC 3550)";
    char statusVal[48] = "Idle", statusSub[64] = "No active session";

    if (metrics.transmitted > 0) {
        if (metrics.received > 0) {
            snprintf(rttVal, sizeof(rttVal), "%.2f ms", metrics.mean_rtt_ms);
            snprintf(rttSub, sizeof(rttSub), "Min: %.1f | Max: %.1f ms", metrics.min_rtt_ms, metrics.max_rtt_ms);
        } else {
            strncpy(rttVal, "Timeout", sizeof(rttVal));
            strncpy(rttSub, "No ICMP Echo replies", sizeof(rttSub));
        }

        snprintf(lossVal, sizeof(lossVal), "%.1f %%", metrics.packet_loss_pct);
        snprintf(lossSub, sizeof(lossSub), "%d dropped of %d tx", metrics.failed, metrics.transmitted);

        if (metrics.jitter_ms >= 0) {
            snprintf(jitVal, sizeof(jitVal), "%.2f ms", metrics.jitter_ms);
        }

        strncpy(statusVal, assess.overall_status_str, sizeof(statusVal) - 1);
        snprintf(statusSub, sizeof(statusSub), "Profile: %s", assess.profile_name);
    }

    UI_DrawCard(hdc, rect.left + 24, cardY, cardW, cardH, "MEAN RTT LATENCY", rttVal, rttSub, COLOR_ACCENT_BLUE);
    UI_DrawCard(hdc, rect.left + 24 + cardW + 12, cardY, cardW, cardH, "PACKET LOSS", lossVal, lossSub,
                (metrics.packet_loss_pct > 5.0) ? COLOR_DANGER_RED : COLOR_SUCCESS_GREEN);
    UI_DrawCard(hdc, rect.left + 24 + (cardW + 12) * 2, cardY, cardW, cardH, "JITTER ESTIMATE", jitVal, jitSub, COLOR_ACCENT_TEAL);
    UI_DrawCard(hdc, rect.left + 24 + (cardW + 12) * 3, cardY, cardW, cardH, "QoS SLA STATUS", statusVal, statusSub,
                (assess.overall_status == QOS_STATUS_POOR) ? COLOR_DANGER_RED : COLOR_SUCCESS_GREEN);

    // Chart Area
    RECT chartRect = { rect.left + 24, cardY + cardH + 18, rect.right - 24, cardY + cardH + 280 };

    double rtts[200];
    bool succs[200];
    int plotCount = (count > 200) ? 200 : count;
    int startIdx = count - plotCount;

    for (int i = 0; i < plotCount; i++) {
        rtts[i] = samples[startIdx + i].rtt_ms;
        succs[i] = samples[startIdx + i].success;
    }

    char chartTitle[128];
    snprintf(chartTitle, sizeof(chartTitle), "Real-Time RTT Latency Progression (Recent %d Probes)", plotCount);
    UI_DrawLineChart(hdc, chartRect, rtts, succs, plotCount, chartTitle, "ms");

    // Live Observations Table / Log Box
    int logY = chartRect.bottom + 16;
    RECT tableHeaderRect = { rect.left + 24, logY, rect.right - 24, rect.bottom - 20 };

    FillRect(hdc, &tableHeaderRect, g_app.hBrushCard);
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
    Rectangle(hdc, tableHeaderRect.left, tableHeaderRect.top, tableHeaderRect.right, tableHeaderRect.bottom);

    SelectObject(hdc, g_app.hFontBold);
    SetTextColor(hdc, COLOR_PRIMARY_NAVY);
    RECT tTitle = { tableHeaderRect.left + 14, tableHeaderRect.top + 8, tableHeaderRect.right - 14, tableHeaderRect.top + 28 };
    DrawTextA(hdc, "Recent Live Network Observations", -1, &tTitle, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, g_app.hFontMono);
    int rowY = tableHeaderRect.top + 34;
    int maxRows = (tableHeaderRect.bottom - rowY - 10) / 20;

    int displayRows = (count < maxRows) ? count : maxRows;
    for (int i = 0; i < displayRows; i++) {
        ProbeSample *s = &samples[count - 1 - i];
        char rowText[256];
        if (s->success) {
            snprintf(rowText, sizeof(rowText), "[#%04d] %s | SUCCESS | RTT: %6.2f ms | IP: %s",
                     s->sequence, s->timestamp_iso, s->rtt_ms, s->resolved_ip);
            SetTextColor(hdc, COLOR_SUCCESS_GREEN);
        } else {
            snprintf(rowText, sizeof(rowText), "[#%04d] %s | FAILED  | %s: %s",
                     s->sequence, s->timestamp_iso, s->error_type, s->error_detail);
            SetTextColor(hdc, COLOR_DANGER_RED);
        }
        RECT rRect = { tableHeaderRect.left + 14, rowY, tableHeaderRect.right - 14, rowY + 18 };
        DrawTextA(hdc, rowText, -1, &rRect, DT_LEFT | DT_SINGLELINE);
        rowY += 20;
    }

    if (displayRows == 0) {
        SelectObject(hdc, g_app.hFontBody);
        SetTextColor(hdc, COLOR_SECONDARY_TXT);
        RECT rRect = { tableHeaderRect.left + 14, rowY + 10, tableHeaderRect.right - 14, rowY + 30 };
        DrawTextA(hdc, "No measurements yet — click 'Start Monitoring' above to begin capturing genuine data.",
                  -1, &rRect, DT_LEFT | DT_SINGLELINE);
    }

    SelectObject(hdc, hOldPen);
}

void UI_DrawView(HDC hdc, RECT rect, AppNavSection section) {
    FillRect(hdc, &rect, g_app.hBrushBg);

    // Show/hide live controls based on view
    bool showLiveBar = (section == NAV_OVERVIEW || section == NAV_LIVE_MONITORING);
    ShowWindow(s_hwndBtnStart, showLiveBar ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndBtnStop, showLiveBar ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndComboTarget, showLiveBar ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndComboProfile, showLiveBar ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndEditInterval, showLiveBar ? SW_SHOW : SW_HIDE);

    // Show/hide target controls
    bool showTargets = (section == NAV_TARGETS);
    ShowWindow(s_hwndEditTName, showTargets ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndEditTHost, showTargets ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndComboTType, showTargets ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndEditTPort, showTargets ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndBtnAddTarget, showTargets ? SW_SHOW : SW_HIDE);

    // Show/hide reports controls
    bool showReports = (section == NAV_REPORTS);
    ShowWindow(s_hwndEditExportPath, showReports ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndBtnExport, showReports ? SW_SHOW : SW_HIDE);

    // Show/hide settings controls
    bool showSettings = (section == NAV_SETTINGS);
    ShowWindow(s_hwndEditThreshRTT, showSettings ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndEditThreshLoss, showSettings ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndEditThreshJitter, showSettings ? SW_SHOW : SW_HIDE);
    ShowWindow(s_hwndBtnSaveSettings, showSettings ? SW_SHOW : SW_HIDE);

    switch (section) {
        case NAV_OVERVIEW:
        case NAV_LIVE_MONITORING:
            UI_DrawOverview(hdc, rect);
            break;

        case NAV_TARGETS: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "Configured Monitoring Targets", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Manage target endpoints (ICMP Ping or TCP Handshake).", -1, &subRect, DT_LEFT | DT_SINGLELINE);

            // Add Target Card
            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.top + 160 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT formTitle = { cardRect.left + 16, cardRect.top + 10, cardRect.right - 16, cardRect.top + 30 };
            DrawTextA(hdc, "Add New Target Endpoint", -1, &formTitle, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontSmall);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            TextOutA(hdc, cardRect.left + 16, cardRect.top + 40, "Target Name:", 12);
            TextOutA(hdc, cardRect.left + 190, cardRect.top + 40, "Host / IP:", 10);
            TextOutA(hdc, cardRect.left + 365, cardRect.top + 40, "Type:", 5);
            TextOutA(hdc, cardRect.left + 480, cardRect.top + 40, "Port:", 5);

            // Targets List Box
            RECT listCard = { rect.left + 24, rect.top + 180, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &listCard, g_app.hBrushCard);
            Rectangle(hdc, listCard.left, listCard.top, listCard.right, listCard.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT listTitle = { listCard.left + 16, listCard.top + 12, listCard.right - 16, listCard.top + 32 };
            DrawTextA(hdc, "Configured Target Catalog", -1, &listTitle, DT_LEFT | DT_SINGLELINE);

            DbTarget targets[32];
            int count = 0;
            DB_GetTargets(targets, 32, &count);

            SelectObject(hdc, g_app.hFontMono);
            int rowY = listCard.top + 45;
            for (int i = 0; i < count; i++) {
                char row[256];
                snprintf(row, sizeof(row), "ID: %02d  |  %-22s  |  %-18s  |  Type: %-4s  |  Port: %-5d  |  Added: %s",
                         targets[i].id, targets[i].name, targets[i].host, targets[i].target_type, targets[i].port, targets[i].created_at);
                SetTextColor(hdc, COLOR_PRIMARY_NAVY);
                TextOutA(hdc, listCard.left + 16, rowY, row, (int)strlen(row));
                rowY += 26;
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_QOS_ANALYTICS: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "Quality of Service (QoS) SLA Profiles", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Application-level performance standards and latency/jitter bounds.", -1, &subRect, DT_LEFT | DT_SINGLELINE);

            // Display Profile Matrix
            RECT matCard = { rect.left + 24, rect.top + 80, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &matCard, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, matCard.left, matCard.top, matCard.right, matCard.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT mTitle = { matCard.left + 16, matCard.top + 14, matCard.right - 16, matCard.top + 34 };
            DrawTextA(hdc, "Supported QoS Application Profiles & SLA Threshold Matrix", -1, &mTitle, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontMono);
            int y = matCard.top + 50;
            const char *lines[] = {
                "PROFILE             | GOOD LATENCY | DEGRADED LATENCY | GOOD JITTER | DEGRADED JITTER | GOOD LOSS | DEGRADED LOSS",
                "--------------------+--------------+------------------+-------------+-----------------+-----------+--------------",
                "video_conferencing  | <= 150.0 ms  | <= 300.0 ms      | <= 30.0 ms  | <= 50.0 ms      | <= 1.0 %  | <= 5.0 %",
                "voip                | <= 100.0 ms  | <= 200.0 ms      | <= 20.0 ms  | <= 40.0 ms      | <= 0.5 %  | <= 2.0 %",
                "gaming              | <=  50.0 ms  | <= 100.0 ms      | <= 10.0 ms  | <= 25.0 ms      | <= 0.1 %  | <= 2.0 %",
                "browsing            | <= 250.0 ms  | <= 500.0 ms      | <= 50.0 ms  | <= 100.0 ms     | <= 2.0 %  | <= 8.0 %",
                "file_download       | <= 400.0 ms  | <= 800.0 ms      | <= 100.0 ms | <= 200.0 ms     | <= 3.0 %  | <= 10.0 %"
            };

            for (size_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
                if (i == 0) SetTextColor(hdc, COLOR_ACCENT_BLUE);
                else SetTextColor(hdc, COLOR_PRIMARY_NAVY);
                TextOutA(hdc, matCard.left + 16, y, lines[i], (int)strlen(lines[i]));
                y += 24;
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_NETWORK_INTERFACES: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "Windows Network Interface Hardware Statistics", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Hardware octet counters and live traffic rates queried via IP Helper GetIfTable2.",
                      -1, &subRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            InterfaceSnapshot if_snap;
            InterfaceStats_Poll(&if_snap);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT cardT = { cardRect.left + 16, cardRect.top + 14, cardRect.right - 16, cardRect.top + 34 };
            DrawTextA(hdc, "Enumerated Active Network Adapters", -1, &cardT, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontMono);
            int y = cardRect.top + 50;
            char header[256];
            snprintf(header, sizeof(header), "%-4s | %-28s | %-8s | %-14s | %-14s",
                     "IDX", "INTERFACE NAME", "STATUS", "RX RATE", "TX RATE");
            SetTextColor(hdc, COLOR_ACCENT_BLUE);
            TextOutA(hdc, cardRect.left + 16, y, header, (int)strlen(header));
            y += 22;

            for (int i = 0; i < if_snap.count; i++) {
                NetworkInterfaceInfo *info = &if_snap.interfaces[i];
                char row[256];
                snprintf(row, sizeof(row), "#%02lu | %-28.28s | %-8s | %8.2f Mbps | %8.2f Mbps",
                         info->if_index, info->name, info->is_up ? "UP" : "DOWN",
                         info->rx_rate_mbps, info->tx_rate_mbps);
                SetTextColor(hdc, info->is_up ? COLOR_PRIMARY_NAVY : COLOR_SECONDARY_TXT);
                TextOutA(hdc, cardRect.left + 16, y, row, (int)strlen(row));
                y += 24;
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_ALERTS: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "QoS Alert Event Feed", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Real-time alerts triggered by persistent latency, reachability, or loss violations.",
                      -1, &subRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            DbAlert alerts[64];
            int count = 0;
            DB_GetAlerts(alerts, 64, &count);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT cardT = { cardRect.left + 16, cardRect.top + 14, cardRect.right - 16, cardRect.top + 34 };
            DrawTextA(hdc, "Historical Alert Log", -1, &cardT, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontMono);
            int y = cardRect.top + 50;

            for (int i = 0; i < count && i < 20; i++) {
                char row[256];
                snprintf(row, sizeof(row), "[%s] %s | %-8s | Obs: %6.1f (Thresh: %6.1f) | %s",
                         alerts[i].timestamp, alerts[i].severity, alerts[i].metric,
                         alerts[i].observed_value, alerts[i].threshold_value, alerts[i].message);
                if (strcmp(alerts[i].severity, "critical") == 0) {
                    SetTextColor(hdc, COLOR_DANGER_RED);
                } else {
                    SetTextColor(hdc, COLOR_WARN_AMBER);
                }
                TextOutA(hdc, cardRect.left + 16, y, row, (int)strlen(row));
                y += 24;
            }

            if (count == 0) {
                SelectObject(hdc, g_app.hFontBody);
                SetTextColor(hdc, COLOR_SECONDARY_TXT);
                TextOutA(hdc, cardRect.left + 16, y, "No alert events recorded. All measurements within normal limits.", 68);
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_HISTORY: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "Historical Monitoring Sessions", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Persistent sessions saved in SQLite local database.", -1, &subRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            DbSession sessions[64];
            int count = 0;
            DB_GetSessions(sessions, 64, &count);

            SelectObject(hdc, g_app.hFontMono);
            int y = cardRect.top + 30;
            for (int i = 0; i < count && i < 22; i++) {
                char row[256];
                snprintf(row, sizeof(row), "Session #%03d | TargetID: %02d | Start: %-19s | End: %-19s | Status: %-8s | Profile: %s",
                         sessions[i].id, sessions[i].target_id, sessions[i].start_time,
                         sessions[i].end_time[0] ? sessions[i].end_time : "active",
                         sessions[i].status, sessions[i].profile);
                SetTextColor(hdc, COLOR_PRIMARY_NAVY);
                TextOutA(hdc, cardRect.left + 16, y, row, (int)strlen(row));
                y += 24;
            }

            if (count == 0) {
                SelectObject(hdc, g_app.hFontBody);
                SetTextColor(hdc, COLOR_SECONDARY_TXT);
                TextOutA(hdc, cardRect.left + 16, y, "No past sessions found. Start a session to store monitoring history.", 68);
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_REPORTS: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "QoS Compliance Reports & CSV Export", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Export raw probe observations to RFC 4180 standard CSV file.", -1, &subRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.top + 220 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT cTitle = { cardRect.left + 16, cardRect.top + 14, cardRect.right - 16, cardRect.top + 34 };
            DrawTextA(hdc, "Export Session Probes to CSV", -1, &cTitle, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            TextOutA(hdc, cardRect.left + 16, cardRect.top + 45, "Output Filepath:", 16);

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_SETTINGS: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "Thresholds & Alert Configuration", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            RECT subRect = { rect.left + 24, rect.top + 50, rect.right - 24, rect.top + 72 };
            DrawTextA(hdc, "Configure sensitivity limits and consecutive-violation hysteresis.", -1, &subRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.top + 240 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT cTitle = { cardRect.left + 16, cardRect.top + 14, cardRect.right - 16, cardRect.top + 34 };
            DrawTextA(hdc, "Alert Trigger Limits", -1, &cTitle, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            TextOutA(hdc, cardRect.left + 16, cardRect.top + 45, "Max RTT Latency Limit (ms):", 27);
            TextOutA(hdc, cardRect.left + 16, cardRect.top + 85, "Max Packet Loss Limit (%):", 26);
            TextOutA(hdc, cardRect.left + 16, cardRect.top + 125, "Max Jitter Limit (ms):", 22);

            SelectObject(hdc, hOldPen);
            break;
        }

        case NAV_ABOUT: {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_app.hFontHeading);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT hRect = { rect.left + 24, rect.top + 20, rect.right - 24, rect.top + 50 };
            DrawTextA(hdc, "About NetPulse QoS", -1, &hRect, DT_LEFT | DT_SINGLELINE);

            RECT cardRect = { rect.left + 24, rect.top + 80, rect.right - 24, rect.bottom - 24 };
            FillRect(hdc, &cardRect, g_app.hBrushCard);
            HPEN hOldPen = (HPEN)SelectObject(hdc, g_app.hPenBorder);
            Rectangle(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom);

            SelectObject(hdc, g_app.hFontBold);
            SetTextColor(hdc, COLOR_PRIMARY_NAVY);
            RECT cTitle = { cardRect.left + 20, cardRect.top + 20, cardRect.right - 20, cardRect.top + 45 };
            DrawTextA(hdc, "NetPulse QoS — Network Observability & Diagnostics System", -1, &cTitle, DT_LEFT | DT_SINGLELINE);

            SelectObject(hdc, g_app.hFontBody);
            SetTextColor(hdc, COLOR_SECONDARY_TXT);
            int y = cardRect.top + 60;
            const char *aboutText[] = {
                "Academic Project: B.Tech CSE (Computer Networks)",
                "Developer: Akhil Pullai",
                "Language: Pure C (C11 standard)",
                "User Interface: Native Win32 API with custom GDI rendering",
                "Network Engines: Windows ICMP API, Winsock2 TCP sockets, IP Helper API",
                "Storage: SQLite 3 Amalgamation C API",
                "Compliance: RFC 1889 / 3550 Inter-Packet Jitter calculations",
                "",
                "This application performs genuine network probes with microsecond-grade monotonic timing.",
                "No simulated values or mock data are used."
            };

            for (size_t i = 0; i < sizeof(aboutText)/sizeof(aboutText[0]); i++) {
                TextOutA(hdc, cardRect.left + 20, y, aboutText[i], (int)strlen(aboutText[i]));
                y += 24;
            }

            SelectObject(hdc, hOldPen);
            break;
        }

        default:
            break;
    }
}

void UI_HandleResize(HWND hwnd, int width, int height) {
    (void)hwnd;
    g_app.clientWidth = width;
    g_app.clientHeight = height;

    int sidebarWidth = 240;
    int topOffset = 20;

    // Reposition live monitoring top bar controls
    SetWindowPos(s_hwndComboTarget, NULL, sidebarWidth + 24 + 230, topOffset, 200, 200, SWP_NOZORDER);
    SetWindowPos(s_hwndComboProfile, NULL, sidebarWidth + 24 + 440, topOffset, 160, 200, SWP_NOZORDER);
    SetWindowPos(s_hwndEditInterval, NULL, sidebarWidth + 24 + 610, topOffset, 50, 28, SWP_NOZORDER);
    SetWindowPos(s_hwndBtnStart, NULL, sidebarWidth + 24 + 670, topOffset, 130, 28, SWP_NOZORDER);
    SetWindowPos(s_hwndBtnStop, NULL, sidebarWidth + 24 + 810, topOffset, 70, 28, SWP_NOZORDER);

    // Reposition Target form controls
    SetWindowPos(s_hwndEditTName, NULL, sidebarWidth + 40, 140, 160, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndEditTHost, NULL, sidebarWidth + 215, 140, 160, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndComboTType, NULL, sidebarWidth + 390, 140, 90, 100, SWP_NOZORDER);
    SetWindowPos(s_hwndEditTPort, NULL, sidebarWidth + 490, 140, 60, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndBtnAddTarget, NULL, sidebarWidth + 560, 139, 110, 28, SWP_NOZORDER);

    // Reposition Reports controls
    SetWindowPos(s_hwndEditExportPath, NULL, sidebarWidth + 40, 140, 280, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndBtnExport, NULL, sidebarWidth + 330, 139, 110, 28, SWP_NOZORDER);

    // Reposition Settings controls
    SetWindowPos(s_hwndEditThreshRTT, NULL, sidebarWidth + 240, 120, 80, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndEditThreshLoss, NULL, sidebarWidth + 240, 160, 80, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndEditThreshJitter, NULL, sidebarWidth + 240, 200, 80, 26, SWP_NOZORDER);
    SetWindowPos(s_hwndBtnSaveSettings, NULL, sidebarWidth + 40, 240, 150, 28, SWP_NOZORDER);
}

void UI_HandleCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    int wmId = LOWORD(wParam);

    if (wmId == IDC_BTN_START_MON) {
        int curSel = (int)SendMessageA(s_hwndComboTarget, CB_GETCURSEL, 0, 0);
        DbTarget targets[32];
        int count = 0;
        DB_GetTargets(targets, 32, &count);

        if (curSel >= 0 && curSel < count) {
            char profile[32] = "video_conferencing";
            int profSel = (int)SendMessageA(s_hwndComboProfile, CB_GETCURSEL, 0, 0);
            if (profSel >= 0) {
                SendMessageA(s_hwndComboProfile, CB_GETLBTEXT, profSel, (LPARAM)profile);
            }

            char intText[32] = "1.0";
            GetWindowTextA(s_hwndEditInterval, intText, sizeof(intText));
            double interval = atof(intText);
            if (interval < 0.2) interval = 1.0;

            Monitor_Start(&targets[curSel], interval, profile);
            InvalidateRect(hwnd, NULL, FALSE);
        }
    } else if (wmId == IDC_BTN_STOP_MON) {
        Monitor_Stop();
        InvalidateRect(hwnd, NULL, FALSE);
    } else if (wmId == IDC_BTN_ADD_TARGET) {
        char name[128] = {0}, host[256] = {0}, portStr[16] = "80";
        GetWindowTextA(s_hwndEditTName, name, sizeof(name));
        GetWindowTextA(s_hwndEditTHost, host, sizeof(host));
        GetWindowTextA(s_hwndEditTPort, portStr, sizeof(portStr));
        int port = atoi(portStr);

        char type[16] = "icmp";
        int tSel = (int)SendMessageA(s_hwndComboTType, CB_GETCURSEL, 0, 0);
        if (tSel == 1) strncpy(type, "tcp", sizeof(type));

        if (name[0] && host[0]) {
            int newId = 0;
            if (DB_AddTarget(name, host, type, port, &newId)) {
                // Refresh combobox
                SendMessageA(s_hwndComboTarget, CB_RESETCONTENT, 0, 0);
                DbTarget targets[32];
                int count = 0;
                DB_GetTargets(targets, 32, &count);
                for (int i = 0; i < count; i++) {
                    char label[256];
                    snprintf(label, sizeof(label), "%s (%s [%s])", targets[i].name, targets[i].host, targets[i].target_type);
                    SendMessageA(s_hwndComboTarget, CB_ADDSTRING, 0, (LPARAM)label);
                }
                SendMessageA(s_hwndComboTarget, CB_SETCURSEL, count - 1, 0);
                MessageBoxA(hwnd, "Target added successfully!", "NetPulse QoS", MB_OK | MB_ICONINFORMATION);
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
    } else if (wmId == IDC_BTN_EXPORT_CSV) {
        char path[260] = "netpulse_export.csv";
        GetWindowTextA(s_hwndEditExportPath, path, sizeof(path));

        DbSession sessions[1];
        int scount = 0;
        DB_GetSessions(sessions, 1, &scount);
        if (scount > 0) {
            if (Reports_ExportSessionCSV(sessions[0].id, path)) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Successfully exported Session #%d probes to:\n%s", sessions[0].id, path);
                MessageBoxA(hwnd, msg, "CSV Export Completed", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(hwnd, "Failed exporting session probes to CSV.", "Export Error", MB_OK | MB_ICONERROR);
            }
        } else {
            MessageBoxA(hwnd, "No monitoring sessions exist to export yet.", "Export Notice", MB_OK | MB_ICONWARNING);
        }
    }
}
