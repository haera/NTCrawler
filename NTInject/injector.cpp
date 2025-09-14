#include "injector.hpp"

BOOL injector::InjectDLL(
    _In_ DWORD pID,
    _In_ LPCWSTR dllPath,
    _In_opt_ PCWSTR pass
) {
    SIZE_T dllPathLen = (wcslen(dllPath) + 1) * sizeof(wchar_t);

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (hProcess == NULL)
    {
        erro("Injection - Could not get handle w/ OpenProcess");
        return FALSE;
    }

    HMODULE hMod = GetModuleHandle(L"Kernel32.dll");
    if (hMod == NULL)
    {
        erro("Injection - Could not GetModuleHandle");
        CloseHandle(hProcess);
        return FALSE;
    }

    PVOID pLoadLib = GetProcAddress(hMod, "LoadLibraryW");
    if (pLoadLib == NULL)
    {
        erro("Injection - Could not GetModuleHandle");
        CloseHandle(hProcess);
        return FALSE;
    }

    LPVOID pBuf = VirtualAllocEx(hProcess, NULL, dllPathLen, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (pBuf == NULL)
    {
        erro("Injection - Could not VirtualAllocEx");
        CloseHandle(hProcess);
        return FALSE;
    }

    WriteProcessMemory(hProcess, pBuf, (LPVOID)dllPath, dllPathLen, 0);

    DWORD tID;
    
    /*
    *   i want to pass info from the CreateRemoteThread caller to the injected dll.
    *   however, i have a flight in 4 hours so this is as good as it gets
    * 
    *   passing params through the thread description. we call CRT using CREATE_SUSPENDED flag,
    *   write using SetThreadDescription, then resume it. this means the DllMain /Entry can use
    *   GetThreadDescription yippeeee
    * 
    *   NVM!!!
    *       kernel32.dll only exports SetThreadDescription since > Windows 10 1607 just kill me
    */
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLib, pBuf, CREATE_SUSPENDED, &tID);
    if (hThread == NULL)
    {
        erro("Injection - Could not CreateRemoteThread");
        VirtualFreeEx(hProcess, pBuf, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    if (pass)
        SetThreadDescription(hThread, pass);

    info("Waiting for thread desc hacky bullshit...");
    ResumeThread(hThread);
    WaitForSingleObject(hThread, INFINITE);

    okay("yayyy created thread: %ld", tID);
    info("Waiting for thread to finish...");

    WaitForSingleObject(hThread, INFINITE);
    info("Thread finished. Cleaning up...");

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pBuf, 0, MEM_RELEASE);
    //VirtualFree(pBuf, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    okay("All done!");

    return TRUE;
}

ProcInfo injector::GetProcInfo(HANDLE hProcess)
{
    ProcInfo pInfo;
    pInfo.integrityLevel = L"idk";
    pInfo.username = L"idk";

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_QUERY_SOURCE, &hToken)) 
    {
        return pInfo;
    }

    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize);
    TOKEN_MANDATORY_LABEL* pTIL = (TOKEN_MANDATORY_LABEL*)LocalAlloc(LPTR, dwSize);
    if (pTIL && GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize)) 
    {
        DWORD dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));
        if (dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID) 
        {
            pInfo.integrityLevel = L"Shit";
            pInfo.flags |= INTEGRITY_SHIT;
        } 
        else if (dwIntegrityLevel >= SECURITY_MANDATORY_SYSTEM_RID) 
        {
            pInfo.integrityLevel = L"System";
            pInfo.flags |= INTEGRITY_SYSTEM;
        }
        else if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) 
        {
            pInfo.integrityLevel = L"High";
            pInfo.flags |= INTEGRITY_HIGH;
        }
    }
    LocalFree(pTIL);

    dwSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
    TOKEN_USER* pTokenUser = (TOKEN_USER*)LocalAlloc(LPTR, dwSize);
    if (pTokenUser && GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) 
    {
        WCHAR szName[256];
        WCHAR szDomain[256];
        DWORD cchName = 256, cchDomain = 256;
        SID_NAME_USE eUse;
        if (LookupAccountSid(NULL, pTokenUser->User.Sid, szName, &cchName, szDomain, &cchDomain, &eUse)) 
        {
            pInfo.username = std::wstring(szDomain) + L"\\" + std::wstring(szName);
            if (pInfo.username == L"NT AUTHORITY\\SYSTEM")
                pInfo.flags |= USER_SYSTEM;
        }
    }
    LocalFree(pTokenUser);
    CloseHandle(hToken);

    return pInfo;
}