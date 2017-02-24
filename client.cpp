#include "client.h"
#include "wrappers.h"
#include "server.h"
#include "clientwindow.h"
#include <stdio.h>
#include <time.h>
#include <WinBase.h>

ClientStats stats;

/**
 * @brief clientCreateAddress sets up a address structure for the client to use
 * @author Robert Arendac
 * @param host - The IP address of the server the client wants to connect to
 * @return Created address structure
 */
SOCKADDR_IN clientCreateAddress(char *host) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(5150);

    return addr;
}

/**
 * @brief statThread Thread function that continuously updates the GUI time field
 * @author Robert Arendac
 * @param arg The ClientWindow GUI
 * @return Thread exit condition
 */
DWORD WINAPI statThread(void *arg) {
    ClientWindow *cw = (ClientWindow *)arg;

    while (1) {
        cw->updateTime(stats.time);
    }

    return 0;
}

/**
 * @brief clientRoutine callback function that is executed when send operation completes
 * @author Robert Arendac
 * @param error Error code that occurs on Send
 */
void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD) {
    if (error) {
        fprintf(stderr, "Error: %d\n", error);
    }
}

/**
 * @brief runClient main driver of the client side.  Creates an overlapped socket and sends data via the UDP or TCP
 * protocol
 * @author Robert Arendac
 * @param cw Pointer to the ClientWindow GUI
 * @param type Will be either SOCK_STREAM or USER_DGRAM
 * @param protocol TCP or UDP
 * @param ip IP address that cliennt will connect to
 * @param size Size of the packet
 * @param times Times to send the packet
 *
 * I'm really sorry about the sleeps...
 */
void runClient(ClientWindow *cw, int type, int protocol, char *ip, int size, int times) {
    SOCKET sck;
    SOCKADDR_IN addr;
    char *data;
    SocketInformation *si;
    DWORD sendBytes;
    SYSTEMTIME start, end;
    int sleepDelay;

    stats.time = 0;

    if (!startWinsock())
        return;

    if ((sck = createSocket(type, protocol)) == NULL)
        return;

    if (!connectHost(ip))
        return;

    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip);

    if (protocol == IPPROTO_TCP) {
        if (!connectToServer(sck, &addr)) {
            return;
        }
    }

    data = (char *)malloc(size);
    if (data == NULL) {
        perror("malloc failed");
        return;
    }

    strnset(data, 'a', size-1);
    data[size - 1] = '\0';

    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    strcpy(si->buffer, data);
    si->socket = sck;
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.buf = si->buffer;
    si->dataBuf.len = size;

    GetSystemTime(&start);
    /* Sleeps used so that client doesn't get overwhelmed with data, prevents crashing and buff overflow */
    if (protocol == IPPROTO_TCP) {
        Sleep(65);
        for (size_t i = 0; i < times; i++) {
            WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);
            Sleep(65);
        }
    } else {
        Sleep(65);
        for (size_t i = 0; i < times; i++) {
            WSASendTo(si->socket, &(si->dataBuf), 1, &sendBytes, 0, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN), &(si->overlapped), clientRoutine);
            Sleep(65);
        }
    }
    GetSystemTime(&end);

    stats.time = delay(start, end);
    sleepDelay = (times + 1) * 65;
    stats.time -= sleepDelay;

    cw->updateTime(stats.time);

    free(data);
    Sleep(65);    //Allow all data to come thru before closing
    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}
