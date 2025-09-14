#include "util.hpp"

BOOL util::SetSeDebugPrivilege()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tokenPriv;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        erro("SeDebugPrivilege - Could not OpenProcessToken");
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
    {
        erro("SeDebugPrivilege - Could not LookupPrivilegeValue");
        CloseHandle(hToken);
        return FALSE;
    }

    tokenPriv.PrivilegeCount = 1;
    tokenPriv.Privileges[0].Luid = luid;
    tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL status = AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, sizeof(tokenPriv), NULL, NULL);
    CloseHandle(hToken);
    return status;
}

