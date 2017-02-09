#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>

SOCKADDR_IN clientCreateAddress(char *host);
void runClient(int type, int protocol, char *ip, int size, int times);

#endif // CLIENT_H
