#pragma once
#include <Windows.h>
#include <string>
#include <tlhelp32.h>
#include <sddl.h>
#include "util.hpp"

#define INTEGRITY_SHIT   0x01
#define INTEGRITY_HIGH   0x02
#define INTEGRITY_SYSTEM 0x04

#define USER_SYSTEM      0x08

#define GOAT_STATUS      (INTEGRITY_SYSTEM | USER_SYSTEM)

struct ProcInfo 
{
    std::wstring integrityLevel;
    std::wstring username;
    UINT flags = 0;
};

namespace injector
{
    BOOL InjectDLL(
        _In_ DWORD pID,
        _In_ LPCWSTR dllPath,
        _In_opt_ PCWSTR pass
    );

    ProcInfo GetProcInfo(HANDLE hProcess);
}