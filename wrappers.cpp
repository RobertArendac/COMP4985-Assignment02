#include "wrappers.h"
#include <stdio.h>

int startWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 0;
    }

    return 1;
}

int setupAcceptClose(SOCKET s, WSAEVENT *eventArray, DWORD eventTotal) {
    if (WSAEventSelect(s, eventArray[eventTotal - 1], FD_ACCEPT|FD_CLOSE) == SOCKET_ERROR) {
        fprintf(stderr, "WSAEventSelect failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int bindSocket(SOCKET s, SOCKADDR_IN *addr) {
    if (bind(s, (PSOCKADDR)addr, sizeof(*addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int listenConnection(SOCKET s) {
    if (listen(s, 5)) {
        fprintf(stderr, "listen failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

int connectHost(char *host) {
    if (gethostbyname(host) == NULL) {
        fprintf(stderr, "Unknown Server Address\n");
        return 0;
    }

    return 1;
}

int connectToServer(SOCKET s, SOCKADDR_IN *addr) {
    if (connect(s, (SOCKADDR *)addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        fprintf(stderr, "Cannot connect to server, error: %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

DWORD waitForEvents(DWORD eventTotal, WSAEVENT *eventArray) {
    DWORD result;
    // last param true?
    if ((result = WSAWaitForMultipleEvents(eventTotal, eventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED) {
        fprintf(stderr, "Wait for events failed with error: %d\n", WSAGetLastError());
    }

    return result;

}

int discoverEvents(SOCKET s, WSAEVENT event, LPWSANETWORKEVENTS networkEvents) {
    if (WSAEnumNetworkEvents(s, event, networkEvents) == SOCKET_ERROR) {
        fprintf(stderr, "WSA network events failed with error %d\n", WSAGetLastError());
        return 0;
    }

    return 1;
}

SOCKET createSocket(int type, int protocol) {
    SOCKET s;

    if ((s = socket(AF_INET, type, protocol)) == INVALID_SOCKET) {
        perror("Socket creation failed");
        return NULL;
    }

    return s;
}
