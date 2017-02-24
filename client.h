#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>
#include "clientwindow.h"

typedef struct {
    int time;
} ClientStats;

SOCKADDR_IN clientCreateAddress(char *host);
void runClient(ClientWindow *cw, int type, int protocol, char *ip, int size, int times);
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD);
DWORD WINAPI statThread(void *arg);

#endif // CLIENT_H
