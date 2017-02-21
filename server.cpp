#include "server.h"
#include "wrappers.h"
#include "serverwindow.h"
#include <stdio.h>

#define BUFSIZE 5000

SOCKET acceptSocket;

SOCKADDR_IN serverCreateAddress() {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5150);

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

DWORD WINAPI serviceClient(void *arg) {
    threadInfo *ti = (threadInfo *)arg;

    return 0;
}

DWORD WINAPI workThread(void *arg) {
    WSAEVENT events[1];
    DWORD acceptReady;
    SocketInformation *si;
    DWORD recvBytes, flags = 0;

    events[0] = (WSAEVENT)arg;

    while (1) {
        while (1) {
            if (!(acceptReady = waitForEvents(1, events))) {
                return 0;
            }

            if (acceptReady != WAIT_IO_COMPLETION) {
                printf("Connection established!!");
                break;
            }
        }

        WSAResetEvent(events[0]);

        si->socket = acceptSocket;
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        si->bytesSent = 0;
        si->bytesReceived = 0;
        si->dataBuf.len = BUFSIZE;
        si->dataBuf.buf = si->buffer;

        if (WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), tcpRoutine) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
                return 0;
            }
        }
    }

    return 1;
}

void CALLBACK tcpRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags) {
    DWORD recvBytes;

    SocketInformation *si = (SocketInformation *)overlapped;

    if (error != 0 || bytesTransferred == 0) {
        if (error) {
            fprintf(stderr, "Error: %d\n", error);
        }
        fprintf(stderr, "Closing socket: %d\n", si->socket);
        closesocket(si->socket);
        return;
    }

    fprintf(stdout, "PACKET RECEIVED!!!!!!!");

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));

    si->dataBuf.len = BUFSIZE;
    si->dataBuf.buf = si->buffer;

    if (WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), tcpRoutine) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
            return;
        }
    }
}

void runServer(ServerWindow *sw, int type, int protocol) {
    SOCKET listenSocket;
    SOCKADDR_IN addr;
    WSAEVENT acceptEvent;
    WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
    WSANETWORKEVENTS networkEvents;
    SocketInformation *sockArray[WSA_MAXIMUM_WAIT_EVENTS];
    DWORD eventTotal = 0;
    DWORD event;
    threadInfo *ti;

    if (!startWinsock())
        return;

    if ((listenSocket = createSocket(type, protocol)) == NULL)
        return;

    addr = serverCreateAddress();

    if (!bindSocket(listenSocket, &addr))
        return;

    if (!listenConnection(listenSocket))
        return;

    if (!createWSAEvent(&acceptEvent))
        return;

    //CreateThread(NULL, 0, workThread, (void *)acceptEvent, 0, NULL);

    while (1) {
        if (!acceptingSocket(&acceptSocket, listenSocket))
            return;

        if (!setWSAEvent(acceptEvent))
            return;

        fprintf(stdout, "Event set, accept worked");
    }
}
