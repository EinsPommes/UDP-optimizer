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

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

bool DisableInterruptModeration() {
    HKEY hKey;
    const char* keyPath = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueExA(hKey, "*InterruptModeration", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool SetUDPQoS() {
    HKEY hKey;
    const char* keyPath = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\QoS";
    
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 
                       KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueExA(hKey, "Do not use NLA", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool OptimizeMTUAndBuffer() {
    HKEY hKey;
    const char* keyPath = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        // Optimize MTU
        DWORD mtuValue = 1500;
        RegSetValueExA(hKey, "GlobalMaxTcpWindowSize", 0, REG_DWORD, (const BYTE*)&mtuValue, sizeof(mtuValue));
        
        // Optimize Buffer
        DWORD bufferValue = 524288; // 512KB
        RegSetValueExA(hKey, "TcpWindowSize", 0, REG_DWORD, (const BYTE*)&bufferValue, sizeof(bufferValue));
        
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool DisableNetworkThrottling() {
    HKEY hKey;
    const char* keyPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile";
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        DWORD value = 0;
        RegSetValueExA(hKey, "NetworkThrottlingIndex", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool TestUDPLatency() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(53); // DNS port for testing
    
    // Ersetze inet_addr mit inet_pton
    if (inet_pton(AF_INET, "8.8.8.8", &serverAddr.sin_addr) != 1) {
        closesocket(sock);
        return false;
    }

    const int NUM_PINGS = 10;
    std::vector<double> latencies;
    int lost_packets = 0;

    char buffer[32] = "PING";
    char recvBuffer[32];
    
    for (int i = 0; i < NUM_PINGS; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        
        if (sendto(sock, buffer, static_cast<int>(strlen(buffer)), 0, 
                  reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(sock);
            return false;
        }
        
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        if (select(0, &fds, nullptr, nullptr, &timeout) > 0) {
            int fromlen = sizeof(serverAddr);
            if (recvfrom(sock, recvBuffer, sizeof(recvBuffer), 0,
                        reinterpret_cast<sockaddr*>(&serverAddr), &fromlen) != SOCKET_ERROR) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                latencies.push_back(duration.count() / 1000.0); // Convert to milliseconds
            }
        } else {
            lost_packets++;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    closesocket(sock);

    if (!latencies.empty()) {
        double avg_latency = 0;
        for (double lat : latencies) {
            avg_latency += lat;
        }
        avg_latency /= static_cast<double>(latencies.size());
        
        // Calculate jitter
        double jitter = 0;
        for (size_t i = 1; i < latencies.size(); i++) {
            jitter += std::abs(latencies[i] - latencies[i-1]);
        }
        jitter /= static_cast<double>(latencies.size() - 1);
        
        return true;
    }
    
    return false;
}

void ResetSettings() {
    // Reset Interrupt Moderation
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces", 
        0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "*InterruptModeration");
        RegCloseKey(hKey);
    }

    // Reset QoS
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\QoS", 
        0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "Do not use NLA");
        RegCloseKey(hKey);
    }

    // Reset MTU and Buffer
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
        0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "GlobalMaxTcpWindowSize");
        RegDeleteValueA(hKey, "TcpWindowSize");
        RegCloseKey(hKey);
    }

    // Reset Network Throttling
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile", 
        0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueA(hKey, "NetworkThrottlingIndex");
        RegCloseKey(hKey);
    }
}
