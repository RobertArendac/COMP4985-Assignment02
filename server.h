#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>
#include "serverwindow.h"

#define DATA_BUFSIZE 65000
#define PORT 5150

typedef struct SocketInformation {
    OVERLAPPED overlapped;
    char buffer[DATA_BUFSIZE];
    WSABUF dataBuf;
    SOCKET socket;
    DWORD bytesSent;
    DWORD bytesReceived;
} SocketInformation;

typedef struct {
    int protocol;
    WSAEVENT event;
} threadInfo;

void runTCPServer(ServerWindow *sw, int type, int protocol);
void runUDPServer(ServerWindow *sw, int type, int protocol);
SOCKADDR_IN serverCreateAddress();
int createSocketInfo(SOCKET socket, WSAEVENT *eventArray,
                     SocketInformation **sockArray, DWORD *eventTotal);

DWORD WINAPI udpThread(void *arg);
DWORD WINAPI tcpThread(void *arg);
void CALLBACK tcpRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);
void CALLBACK udpRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

#endif
