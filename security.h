#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class SecurityManager {
public:
    // Prüft, ob das Programm mit Admin-Rechten läuft
    static bool IsRunningAsAdmin();
    
    // Erstellt ein Backup der aktuellen Netzwerkeinstellungen
    static bool CreateSettingsBackup(const std::wstring& backupPath);
    
    // Stellt die ursprünglichen Einstellungen wieder her
    static bool RestoreSettingsFromBackup(const std::wstring& backupPath);

private:
    static bool ExportRegistryKey(const std::wstring& keyPath, const std::wstring& fileName);
    static bool ImportRegistryKey(const std::wstring& fileName);
};
