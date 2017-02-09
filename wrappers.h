#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <WinSock2.h>
#include <Windows.h>

int startWinsock();
int setupAcceptClose(SOCKET s, WSAEVENT *eventArray, DWORD eventTotal);
int bindSocket(SOCKET s, SOCKADDR_IN *addr);
int listenConnection(SOCKET s);
int connectHost(char *host);
int connectToServer(SOCKET s, SOCKADDR_IN *addr);
DWORD waitForEvents(DWORD eventTotal, WSAEVENT *eventArray);
int discoverEvents(SOCKET s, WSAEVENT event, LPWSANETWORKEVENTS networkEvents);
SOCKET createSocket(int type, int protocol);

#endif // WRAPPERS_H
