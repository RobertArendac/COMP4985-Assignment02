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
int createEvent(WSAEVENT *eventArray);
int acceptingSocket(SOCKET *acceptSocket, SOCKET listenSocket);
SOCKET createSocket(int type, int protocol);
int createWSAEvent(WSAEVENT *event);
int setWSAEvent(WSAEVENT event);
int asyncSend(SOCKET *socket, WSABUF *buff, DWORD *recvd, OVERLAPPED *overlapped);
int asyncRecv(SOCKET *socket, WSABUF *buff, DWORD *recvd, OVERLAPPED *overlapped);

#endif // WRAPPERS_H
