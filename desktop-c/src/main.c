#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include "app.h"
#include "ui.h"
#include "monitor.h"
#include "database.h"
#include <stdio.h>
#include <stdlib.h>

AppContext g_app;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            UI_InitGDI(hwnd);
            Monitor_Init(hwnd);
            return 0;

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            UI_HandleResize(hwnd, width, height);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);

            // Handle sidebar navigation clicks
            if (mouseX >= 0 && mouseX <= 240) {
                int itemY = 92;
                int itemHeight = 38;
                for (int i = 0; i < NAV_COUNT; i++) {
                    if (mouseY >= itemY && mouseY <= itemY + itemHeight) {
                        g_app.currentSection = (AppNavSection)i;
                        InvalidateRect(hwnd, NULL, TRUE);
                        break;
                    }
                    itemY += itemHeight + 4;
                }
            }
            return 0;
        }

        case WM_COMMAND:
            UI_HandleCommand(hwnd, wParam, lParam);
            return 0;

        case WM_NETPULSE_SAMPLE_UPDATE:
        case WM_NETPULSE_SESSION_STATE:
            // Trigger repaint on live data updates from worker thread
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double buffering for smooth flicker-free rendering
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // Render Sidebar
            RECT sidebarRect = { 0, 0, 240, height };
            UI_DrawSidebar(memDC, sidebarRect);

            // Render Main View Area
            RECT viewRect = { 240, 0, width, height };
            UI_DrawView(memDC, viewRect, g_app.currentSection);

            // Blit buffer to screen
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; // Prevent flicker

        case WM_DESTROY:
            Monitor_Cleanup();
            DB_Close();
            UI_CleanupGDI();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBoxA(NULL, "WSAStartup initialization failed.", "NetPulse Fatal Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Initialize SQLite database
    if (!DB_Init(NULL)) {
        MessageBoxA(NULL, "Failed to initialize local SQLite database (netpulse_desktop.db).", "Database Error", MB_OK | MB_ICONERROR);
    }

    g_app.hInstance = hInstance;
    g_app.currentSection = NAV_OVERVIEW;

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = APP_CLASS;

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Failed to register Windows window class.", "Window Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        APP_CLASS,
        APP_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        APP_DEFAULT_WIDTH, APP_DEFAULT_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create main desktop application window.", "Window Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_app.hwndMain = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    WSACleanup();
    return (int)msg.wParam;
}

// Fallback entry point when building with console subsystem
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}
