#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Strukturen für Netzwerkstatistiken
struct NetworkStats {
    double latency;      // in Millisekunden
    double jitter;       // in Millisekunden
    double packetLoss;   // in Prozent
    int mtu;            // in Bytes
    int bufferSize;     // in Bytes
};

// Callback-Typ für Fortschrittsanzeige
typedef void (*ProgressCallback)(const wchar_t* status, int progressPercent);

// Netzwerk-Optimierungsfunktionen
bool DisableInterruptModeration();
bool SetUDPQoS();
bool OptimizeMTUAndBuffer(int& currentMtu, int& currentBuffer);
bool DisableNetworkThrottling();
bool TestUDPLatency(NetworkStats& stats, const wchar_t* testServer = L"8.8.8.8", int testPort = 53, int numPackets = 100);
bool ResetSettings();

// Hilfsfunktionen
bool GetCurrentNetworkStats(NetworkStats& stats);
bool IsNetworkOptimized();
