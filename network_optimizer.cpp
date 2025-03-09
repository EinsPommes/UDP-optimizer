#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include "network_optimizer.h"
#include <mstcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

bool DisableInterruptModeration() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    DWORD value = 0;
    DWORD result = RegSetValueExW(hKey, L"EnableTCPChimney", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool SetUDPQoS() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\QoS",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    DWORD value = 1;
    DWORD result = RegSetValueExW(hKey, L"Do not use NLA", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool OptimizeMTUAndBuffer() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    // Optimale MTU-Größe
    DWORD mtu = 1500;
    RegSetValueExW(hKey, L"GlobalMaxTcpWindowSize", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&mtu), sizeof(mtu));

    // Größerer Buffer
    DWORD buffer = 16384;
    RegSetValueExW(hKey, L"TcpWindowSize", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&buffer), sizeof(buffer));

    RegCloseKey(hKey);
    return true;
}

bool DisableNetworkThrottling() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    // Netzwerk-Drosselung deaktivieren
    DWORD value = 0;
    RegSetValueExW(hKey, L"NetworkThrottlingIndex", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));

    // Systemreaktivität maximieren
    value = 0;
    RegSetValueExW(hKey, L"SystemResponsiveness", 0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));

    RegCloseKey(hKey);
    return true;
}

bool TestUDPLatency() {
    // Dies ist nur ein Platzhalter für einen echten Latenztest
    // In einer vollständigen Implementierung würden wir hier:
    // 1. Eine UDP-Verbindung zu einem Testserver aufbauen
    // 2. Mehrere Pakete senden und die Antwortzeiten messen
    // 3. Jitter und Paketverlust berechnen
    return true;
}

void ResetSettings() {
    // Interrupt Moderation zurücksetzen
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"EnableTCPChimney");
        RegCloseKey(hKey);
    }

    // QoS-Einstellungen zurücksetzen
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\QoS",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"Do not use NLA");
        RegCloseKey(hKey);
    }

    // MTU und Buffer zurücksetzen
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"GlobalMaxTcpWindowSize");
        RegDeleteValueW(hKey, L"TcpWindowSize");
        RegCloseKey(hKey);
    }

    // Network Throttling zurücksetzen
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"NetworkThrottlingIndex");
        RegDeleteValueW(hKey, L"SystemResponsiveness");
        RegCloseKey(hKey);
    }
}
