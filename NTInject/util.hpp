#include <iostream>
#include <Windows.h>

#define okay(msg, ...) printf("[+] " msg "\n", ##__VA_ARGS__)
#define info(msg, ...) printf("[i] " msg "\n", ##__VA_ARGS__)
#define warn(msg, ...) printf("[-] " msg "\n", ##__VA_ARGS__)
#define erro(msg, ...) printf("[X] " msg "\n", ##__VA_ARGS__)

namespace util
{
	BOOL SetSeDebugPrivilege();
}
