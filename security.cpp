#include "security.h"
#include <shlobj.h>

bool SecurityManager::IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    // Erstelle SID für die Administratoren-Gruppe
    if (!AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup)) {
        return false;
    }

    // Prüfe, ob der aktuelle Prozess zur Administratoren-Gruppe gehört
    if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
        isAdmin = FALSE;
    }

    if (adminGroup) {
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

bool SecurityManager::CreateSettingsBackup(const std::wstring& backupPath) {
    const wchar_t* keys[] = {
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
        L"SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters",
        L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters"
    };

    bool success = true;
    for (const auto& key : keys) {
        if (!ExportRegistryKey(key, backupPath)) {
            success = false;
            break;
        }
    }

    return success;
}

bool SecurityManager::RestoreSettingsFromBackup(const std::wstring& backupPath) {
    return ImportRegistryKey(backupPath);
}

bool SecurityManager::ExportRegistryKey(const std::wstring& keyPath, const std::wstring& fileName) {
    std::wstring command = L"reg export \"" + keyPath + L"\" \"" + fileName + L"\" /y";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()),
        NULL, NULL, FALSE, CREATE_NO_WINDOW,
        NULL, NULL, &si, &pi)) {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode == 0;
}

bool SecurityManager::ImportRegistryKey(const std::wstring& fileName) {
    std::wstring command = L"reg import \"" + fileName + L"\"";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessW(NULL, const_cast<LPWSTR>(command.c_str()),
        NULL, NULL, FALSE, CREATE_NO_WINDOW,
        NULL, NULL, &si, &pi)) {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode == 0;
}
