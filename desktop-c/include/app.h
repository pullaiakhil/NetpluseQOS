#ifndef APP_H
#define APP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdbool.h>

#define APP_TITLE "NetPulse QoS — Network Observability & Diagnostics"
#define APP_CLASS "NetPulseQoSAppClass"
#define APP_DEFAULT_WIDTH 1280
#define APP_DEFAULT_HEIGHT 820
#define APP_MIN_WIDTH 1000
#define APP_MIN_HEIGHT 650

typedef enum {
    NAV_OVERVIEW = 0,
    NAV_LIVE_MONITORING,
    NAV_TARGETS,
    NAV_QOS_ANALYTICS,
    NAV_NETWORK_INTERFACES,
    NAV_ALERTS,
    NAV_HISTORY,
    NAV_REPORTS,
    NAV_SETTINGS,
    NAV_ABOUT,
    NAV_COUNT
} AppNavSection;

typedef struct {
    HWND hwndMain;
    HINSTANCE hInstance;
    AppNavSection currentSection;
    HFONT hFontHeading;
    HFONT hFontSubheading;
    HFONT hFontBody;
    HFONT hFontSmall;
    HFONT hFontBold;
    HFONT hFontMono;
    HBRUSH hBrushBg;
    HBRUSH hBrushSidebar;
    HBRUSH hBrushCard;
    HBRUSH hBrushAccent;
    HBRUSH hBrushBorder;
    HPEN hPenBorder;
    HPEN hPenAccent;
    HPEN hPenTeal;
    HPEN hPenWarning;
    HPEN hPenCritical;
    HPEN hPenSuccess;
    int clientWidth;
    int clientHeight;
} AppContext;

extern AppContext g_app;

void App_Init(HINSTANCE hInstance);
void App_Cleanup(void);

#endif /* APP_H */
