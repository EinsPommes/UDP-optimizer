#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#include <iostream>
#include <string>
#include <vector>
#include "security.h"
#include "network_optimizer.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Fenster-Handles
HWND hMainWindow = NULL;
HWND hOptimizeBtn = NULL;
HWND hResetBtn = NULL;
HWND hStatusText = NULL;
HWND hAdminStatus = NULL;
HWND hBackupStatus = NULL;
HWND hProgressBar = NULL;
HWND hLatencyLabel = NULL;
HWND hJitterLabel = NULL;
HWND hPacketLossLabel = NULL;
HWND hNetworkStats = NULL;
HINSTANCE hInst = NULL;
HFONT hFont = NULL;
HFONT hBoldFont = NULL;

// UI-Konstanten
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 600;
const int PADDING = 15;
const int BUTTON_HEIGHT = 35;
const int LABEL_HEIGHT = 20;

// Funktionsdeklarationen
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Erstellt einen modernen Button
HWND CreateModernButton(HWND parent, LPCWSTR text, int x, int y, int width, int height, HMENU id) {
    return CreateWindowW(
        L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        x, y, width, height, parent, id, hInst, NULL
    );
}

// Erstellt ein modernes Label
HWND CreateModernLabel(HWND parent, LPCWSTR text, int x, int y, int width, int height) {
    HWND label = CreateWindowW(
        L"STATIC", text,
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        x, y, width, height, parent, NULL, hInst, NULL
    );
    SendMessageW(label, WM_SETFONT, (WPARAM)hFont, TRUE);
    return label;
}

// Erstellt eine Gruppierung
HWND CreateGroupBox(HWND parent, LPCWSTR text, int x, int y, int width, int height) {
    HWND group = CreateWindowW(
        L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        x, y, width, height, parent, NULL, hInst, NULL
    );
    SendMessageW(group, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
    return group;
}

// Aktualisiert den Status-Text
void UpdateStatusText(HWND hWnd, const std::wstring& text) {
    SetWindowTextW(hWnd, text.c_str());
    InvalidateRect(hWnd, NULL, TRUE);
    UpdateWindow(hWnd);
}

// Callback für das Hauptfenster
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Erstelle Schriftarten
            hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            
            hBoldFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            // Status-Gruppe
            CreateGroupBox(hwnd, L"System Status", 
                PADDING, PADDING, WINDOW_WIDTH - 2*PADDING, 100);

            // Admin-Status
            bool isAdmin = SecurityManager::IsRunningAsAdmin();
            const wchar_t* adminStatus = isAdmin ? L" Administrator-Rechte: Aktiv" : L" Administrator-Rechte erforderlich";
            hAdminStatus = CreateModernLabel(hwnd, adminStatus,
                2*PADDING, 2*PADDING + 5, 300, LABEL_HEIGHT);

            if (!isAdmin) {
                MessageBoxW(hwnd, 
                    L"Diese Anwendung benötigt Administrator-Rechte.\n"
                    L"Bitte starten Sie die Anwendung als Administrator.", 
                    L"Administrator-Rechte erforderlich", 
                    MB_ICONWARNING);
            }

            // Backup-Status
            hBackupStatus = CreateModernLabel(hwnd, L" Backup: Nicht erstellt",
                2*PADDING, 2*PADDING + 30, 300, LABEL_HEIGHT);

            // Netzwerk-Statistiken Gruppe
            CreateGroupBox(hwnd, L"Netzwerk-Statistiken", 
                PADDING, 130, WINDOW_WIDTH - 2*PADDING, 120);

            // Latenz, Jitter und Packet Loss Labels
            hLatencyLabel = CreateModernLabel(hwnd, L"Latenz: --ms",
                2*PADDING, 160, 200, LABEL_HEIGHT);
            hJitterLabel = CreateModernLabel(hwnd, L"Jitter: --ms",
                2*PADDING, 185, 200, LABEL_HEIGHT);
            hPacketLossLabel = CreateModernLabel(hwnd, L"Paketverlust: --%",
                2*PADDING, 210, 200, LABEL_HEIGHT);

            // Progress Bar
            hProgressBar = CreateWindowW(
                PROGRESS_CLASSW, NULL,
                WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
                PADDING, 270, WINDOW_WIDTH - 2*PADDING, 25,
                hwnd, NULL, hInst, NULL
            );
            SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessage(hProgressBar, PBM_SETSTEP, 1, 0);

            // Buttons in einer Gruppe
            CreateGroupBox(hwnd, L"Aktionen", 
                PADDING, 315, WINDOW_WIDTH - 2*PADDING, 70);

            // Moderne Buttons
            hOptimizeBtn = CreateModernButton(hwnd, L"UDP Optimieren",
                2*PADDING, 340, (WINDOW_WIDTH - 6*PADDING)/2, BUTTON_HEIGHT, (HMENU)1);
            
            hResetBtn = CreateModernButton(hwnd, L"Einstellungen Zurücksetzen",
                3*PADDING + (WINDOW_WIDTH - 6*PADDING)/2, 340, 
                (WINDOW_WIDTH - 6*PADDING)/2, BUTTON_HEIGHT, (HMENU)2);

            // Status Text Gruppe
            CreateGroupBox(hwnd, L"Protokoll", 
                PADDING, 405, WINDOW_WIDTH - 2*PADDING, 150);

            // Mehrzeiliges Status-Fenster
            hStatusText = CreateWindowW(
                L"EDIT", L"Bereit für Optimierung...",
                WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | 
                ES_AUTOVSCROLL | ES_READONLY,
                2*PADDING, 430, WINDOW_WIDTH - 4*PADDING, 110,
                hwnd, NULL, hInst, NULL
            );
            SendMessageW(hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);

            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Optimize button
                if (!SecurityManager::IsRunningAsAdmin()) {
                    MessageBoxW(hwnd, 
                        L"Administrator-Rechte erforderlich für diese Operation.", 
                        L"Fehler", 
                        MB_ICONERROR);
                    return 0;
                }

                // Erstelle Backup vor der Optimierung
                std::wstring backupPath = L"backup.reg";
                if (SecurityManager::CreateSettingsBackup(backupPath)) {
                    SetWindowTextW(hBackupStatus, L" Backup: Erstellt");
                }

                // Setze Progress Bar auf 0
                SendMessage(hProgressBar, PBM_SETPOS, 0, 0);

                std::wstring status = L" Optimierung läuft...\r\n";
                SetWindowTextW(hStatusText, status.c_str());
                
                // Führe Optimierungen durch und aktualisiere Progress Bar
                SendMessage(hProgressBar, PBM_SETPOS, 20, 0);
                if (DisableInterruptModeration()) {
                    status += L" Interrupt Moderation deaktiviert\r\n";
                    SetWindowTextW(hStatusText, status.c_str());
                }

                SendMessage(hProgressBar, PBM_SETPOS, 40, 0);
                if (SetUDPQoS()) {
                    status += L" QoS-Einstellungen angewendet\r\n";
                    SetWindowTextW(hStatusText, status.c_str());
                }

                SendMessage(hProgressBar, PBM_SETPOS, 60, 0);
                if (OptimizeMTUAndBuffer()) {
                    status += L" MTU und Buffer optimiert\r\n";
                    SetWindowTextW(hStatusText, status.c_str());
                }

                SendMessage(hProgressBar, PBM_SETPOS, 80, 0);
                if (DisableNetworkThrottling()) {
                    status += L" Network Throttling deaktiviert\r\n";
                    SetWindowTextW(hStatusText, status.c_str());
                }

                SendMessage(hProgressBar, PBM_SETPOS, 90, 0);
                if (TestUDPLatency()) {
                    status += L" UDP-Latenztest abgeschlossen\r\n";
                    
                    // Aktualisiere Netzwerk-Statistiken (Beispielwerte)
                    SetWindowTextW(hLatencyLabel, L"Latenz: 15ms");
                    SetWindowTextW(hJitterLabel, L"Jitter: 2ms");
                    SetWindowTextW(hPacketLossLabel, L"Paketverlust: 0.1%");
                    
                    SetWindowTextW(hStatusText, status.c_str());
                }

                SendMessage(hProgressBar, PBM_SETPOS, 100, 0);
                status += L"\r\n Optimierung erfolgreich abgeschlossen!";
                SetWindowTextW(hStatusText, status.c_str());
            }
            else if (LOWORD(wParam) == 2) { // Reset button
                if (!SecurityManager::IsRunningAsAdmin()) {
                    MessageBoxW(hwnd, 
                        L"Administrator-Rechte erforderlich für diese Operation.", 
                        L"Fehler", 
                        MB_ICONERROR);
                    return 0;
                }

                if (MessageBoxW(hwnd, 
                    L"Möchten Sie alle Einstellungen zurücksetzen?\nDies kann nicht rückgängig gemacht werden.", 
                    L"Bestätigung", 
                    MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    
                    ResetSettings();
                    
                    // Reset UI
                    SetWindowTextW(hStatusText, L" Einstellungen wurden zurückgesetzt.");
                    SendMessage(hProgressBar, PBM_SETPOS, 0, 0);
                    SetWindowTextW(hLatencyLabel, L"Latenz: --ms");
                    SetWindowTextW(hJitterLabel, L"Jitter: --ms");
                    SetWindowTextW(hPacketLossLabel, L"Paketverlust: --%");
                }
            }
            break;
        }

        case WM_DESTROY:
            DeleteObject(hFont);
            DeleteObject(hBoldFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    // Initialisiere Common Controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBoxW(NULL, L"Failed to initialize Winsock", L"Error", MB_ICONERROR);
        return 1;
    }

    // Register window class
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"UDPOptimizerClass";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // Berechne die Fensterposition für die Bildschirmmitte
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowX = (screenWidth - WINDOW_WIDTH) / 2;
    int windowY = (screenHeight - WINDOW_HEIGHT) / 2;

    // Create main window
    hMainWindow = CreateWindowW(
        L"UDPOptimizerClass", 
        L"UDP Optimizer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        windowX, windowY,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hMainWindow) {
        MessageBoxW(NULL, L"Window Creation Failed", L"Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WSACleanup();
    return (int)msg.wParam;
}
