#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#include <iostream>
#include <string>
#include <iphlpapi.h>
#include <netioapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")git config --global user.name "EinsPommes"

HWND hMainWindow, hOptimizeBtn, hResetBtn, hStatusText;
HINSTANCE hInst;

// Function declarations
bool DisableInterruptModeration();
bool SetUDPQoS();
bool OptimizeMTUAndBuffer();
bool DisableNetworkThrottling();
bool TestUDPLatency();
void ResetSettings();

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create buttons and status text
            hOptimizeBtn = CreateWindow(
                "BUTTON", "Optimize UDP",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 10, 150, 30, hwnd, (HMENU)1, hInst, NULL
            );

            hResetBtn = CreateWindow(
                "BUTTON", "Reset Settings",
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                170, 10, 150, 30, hwnd, (HMENU)2, hInst, NULL
            );

            hStatusText = CreateWindow(
                "STATIC", "Ready to optimize...",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                10, 50, 310, 200, hwnd, (HMENU)3, hInst, NULL
            );
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Optimize button
                std::string status = "Optimizing...\r\n";
                
                if (DisableInterruptModeration())
                    status += "✓ Interrupt Moderation disabled\r\n";
                if (SetUDPQoS())
                    status += "✓ QoS settings applied\r\n";
                if (OptimizeMTUAndBuffer())
                    status += "✓ MTU and Buffer optimized\r\n";
                if (DisableNetworkThrottling())
                    status += "✓ Network Throttling disabled\r\n";
                if (TestUDPLatency())
                    status += "✓ UDP Latency test completed\r\n";

                SetWindowText(hStatusText, status.c_str());
            }
            else if (LOWORD(wParam) == 2) { // Reset button
                ResetSettings();
                SetWindowText(hStatusText, "Settings reset to default");
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, "Failed to initialize Winsock", "Error", MB_ICONERROR);
        return 1;
    }

    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "UDPOptimizerClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONERROR);
        return 1;
    }

    // Create main window
    hMainWindow = CreateWindow(
        "UDPOptimizerClass", "UDP Optimizer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 300,
        NULL, NULL, hInstance, NULL
    );

    if (!hMainWindow) {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONERROR);
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
