#include "server.h"
#include "wrappers.h"
#include <stdio.h>

SOCKADDR_IN serverCreateAddress() {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    return addr;
}

int createSocketInfo(SOCKET socket, WSAEVENT *eventArray,
                     SocketInformation **sockArray, DWORD *eventTotal) {
    SocketInformation *s;

    if ((eventArray[*eventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT) {
        fprintf(stderr, "WSACreateEvent failed with error %d\n", WSAGetLastError());
        return 0;
    }

    if ((s = (SocketInformation *)malloc(sizeof(SocketInformation))) == NULL) {
        fprintf(stderr, "malloc failed with error %d\n", GetLastError());
        return 0;
    }

    s->socket = socket;
    s->bytesSent = 0;
    s->bytesReceived = 0;

    sockArray[(*eventTotal)++] = s;

    return 1;
}

void runServer(int type, int protocol) {
    SOCKET listenSocket;
    SOCKET acceptSocket;
    SOCKADDR_IN addr;
    WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
    WSANETWORKEVENTS networkEvents;
    SocketInformation *sockArray[WSA_MAXIMUM_WAIT_EVENTS];
    DWORD eventTotal = 0;
    DWORD event;

    if (!startWinsock())
        return;

    if ((listenSocket = createSocket(type, protocol)) == NULL)
        return;

    if (!createSocketInfo(listenSocket, eventArray, sockArray, &eventTotal))
        return;

    if (!setupAcceptClose(listenSocket, eventArray, eventTotal))
        return;

    addr = serverCreateAddress();

    if (!bindSocket(listenSocket, &addr))
        return;

    if (!listenConnection(listenSocket))
        return;

    while (1) {
        if ((event = waitForEvents(eventTotal, eventArray)) == WSA_WAIT_FAILED)
            return;

        if (!discoverEvents(sockArray[event - WSA_WAIT_EVENT_0]->socket,
                            eventArray[event - WSA_WAIT_EVENT_0], &networkEvents))
            return;

        if (networkEvents.lNetworkEvents & FD_ACCEPT) {
            acceptSocket = accept(sockArray[event - WSA_WAIT_EVENT_0]->socket, NULL, NULL);
            printf("Socket accepted!!");
            break;
        }

    }
}
