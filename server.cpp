#include "server.h"
#include "wrappers.h"
#include "serverwindow.h"
#include <stdio.h>

#define BUFSIZE 2000

SOCKET acceptSocket;
ServerStats stats;

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

DWORD WINAPI udpThread(void *arg) {
    SocketInformation *si;
    DWORD recvBytes, flags = MSG_PARTIAL;

    while (1) {

        si = (SocketInformation *)malloc(sizeof(SocketInformation));

        si->socket = acceptSocket;
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        si->bytesSent = 0;
        si->bytesReceived = 0;
        si->dataBuf.len = BUFSIZE;
        si->dataBuf.buf = si->buffer;

        //resetStats();

        if (WSARecvFrom(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, NULL, NULL, &(si->overlapped), udpRoutine) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
                return 0;
            }
        }

        SleepEx(INFINITE, TRUE);

    }

    return 1;
}

DWORD WINAPI tcpThread(void *arg) {
    WSAEVENT events[1];
    DWORD acceptReady;
    SocketInformation *si;
    DWORD recvBytes, flags = 0;
    threadInfo *ti = (threadInfo *)arg;

    events[0] = ti->event;

    while (1) {
        while (1) {
            if ((acceptReady = waitForEvents(1, events)) == WSA_WAIT_FAILED) {
                return 0;
            }

            if (acceptReady != WAIT_IO_COMPLETION) {
                printf("Ready to start receiving\n");
                break;
            }
        }

        WSAResetEvent(events[0]);

        si = (SocketInformation *)malloc(sizeof(SocketInformation));

        si->socket = acceptSocket;
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));
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

DWORD WINAPI statsThread(void *arg) {
    ServerWindow *sw = (ServerWindow *)arg;

    while (1) {
        sw->updateTime(stats.time);
        sw->updatePackets(stats.numPackets);
        sw->updateSize(stats.size);
    }
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

    printf("%s\n", si->dataBuf.buf);
    stats.numPackets++;
    stats.size += sizeof(si->dataBuf.buf);

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUFSIZE;
    si->dataBuf.buf = si->buffer;

    if (WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), tcpRoutine) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
            return;
        }
    }
}

void CALLBACK udpRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags) {
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

    printf("received\n");
    stats.numPackets++;
    stats.size += sizeof(si->dataBuf.buf);

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));

    si->dataBuf.len = BUFSIZE;
    si->dataBuf.buf = si->buffer;

    if (WSARecvFrom(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, NULL, NULL, &(si->overlapped), udpRoutine) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
            return;
        }
    }
}

void runTCPServer(ServerWindow *sw, int type, int protocol) {
    SOCKET listenSocket;
    SOCKADDR_IN addr;
    WSAEVENT acceptEvent;
    threadInfo *ti;

    CreateThread(NULL, 0, statsThread, (void *)sw, 0, NULL);

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

    ti = (threadInfo *)malloc(sizeof(threadInfo));
    ti->event = acceptEvent;
    ti->protocol = protocol;

    CreateThread(NULL, 0, tcpThread, (void *)ti, 0, NULL);

    while (1) {
        if (!acceptingSocket(&acceptSocket, listenSocket))
            return;

        if (!setWSAEvent(acceptEvent))
            return;
    }

}

void runUDPServer(ServerWindow *sw, int type, int protocol) {
    SOCKADDR_IN addr;
    HANDLE thrdHandle;

    if (!startWinsock())
        return;

    addr = serverCreateAddress();

    if ((acceptSocket = createSocket(type, protocol)) == NULL)
        return;

    if (!bindSocket(acceptSocket, &addr))
        return;

    thrdHandle = CreateThread(NULL, 0, udpThread, NULL, 0, NULL);
    thrdHandle = CreateThread(NULL, 0, statsThread, (void *)sw, 0, NULL);

    WaitForSingleObject(thrdHandle, INFINITE); //So server keeps running
}

void resetStats() {
    stats.time = 0;
    stats.numPackets = 0;
    stats.size = 0;
}
