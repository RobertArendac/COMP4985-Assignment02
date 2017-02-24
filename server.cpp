#include "server.h"
#include "wrappers.h"
#include "serverwindow.h"
#include <stdio.h>

// Max packet size is ~65K
#define BUFSIZE 65000

SOCKET acceptSocket;
ServerStats stats;
SYSTEMTIME start, end;

/**
 * @brief serverCreateAddress Creates an address struct to use by the server
 * @author Robert Arendac
 * @return Address struct that the server will use
 */
SOCKADDR_IN serverCreateAddress() {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5150);

    return addr;
}

/**
 * @brief udpThread Thread function that will be run when a UDP server is selected.  Calls a WSARecvFrom that executes
 * a completion routine.
 * @author Robert Arendac
 * @param arg unused
 * @return Thread exit status
 */
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

/**
 * @brief tcpThread Thread function that will be executed when a TCP server is selected.  Waits for an event to
 * occur to signify a connection has been made.  Then will execute WSARecv until nothing is left to receive
 * @author Robert Arendac
 * @param arg Structure that holds the event, doesn't need to be a structure, I just forgot to change it from a previous
 * version.
 * @return Thread exit status
 */
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
        resetStats();

        si = (SocketInformation *)malloc(sizeof(SocketInformation));

        si->socket = acceptSocket;
        ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
        memset(si->buffer, 0, sizeof(si->buffer));
        si->bytesSent = 0;
        si->bytesReceived = 0;
        si->dataBuf.len = BUFSIZE;
        si->dataBuf.buf = si->buffer;


        GetSystemTime(&start);
        if (WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), tcpRoutine) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
                return 1;
            }
        }

    }

    return 0;
}


/**
 * @brief statsThread Thread function that constantly updates the GUI
 * @author Robert Arendac
 * @param arg ServerWindow to update GUI
 * @return Thread exit status
 */
DWORD WINAPI statsThread(void *arg) {
    ServerWindow *sw = (ServerWindow *)arg;

    while (1) {
        sw->updateTime(stats.time);
        sw->updatePackets(stats.numPackets);
        sw->updateSize(stats.size);
    }

    return 0;
}

/**
 * @brief tcpRoutine Completion routine for TCP server.  Checks for error or end of transmission, updates stats,
 * and then posts another recv
 * @author Robert Arendac
 * @param error Any error that occured during a recieve
 * @param bytesTransferred Number of bytes transferred during read
 * @param overlapped Overlapped structure for overlapped I/O
 * @param flags Any flags to be set
 */
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
    GetSystemTime(&end);

    stats.time += delay(start, end) - 65;
    stats.numPackets++;
    stats.size += (int)strlen(si->dataBuf.buf) + 1;

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    memset(si->buffer, 0, sizeof(si->buffer));

    si->dataBuf.len = BUFSIZE;
    si->dataBuf.buf = si->buffer;

    GetSystemTime(&start);
    if (WSARecv(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, &(si->overlapped), tcpRoutine) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
            return;
        }
    }
}

/**
 * @brief udpRoutine Completion routine that gets executed on UDP servers.  Checks for error, updates stats,
 * and then posts another receive
 * @author Robert Arendac
 * @param error Any error that occured
 * @param bytesTransferred Number of bytes transferred
 * @param overlapped Overlapped struct for overlapped I/O
 * @param flags Any flags to be set
 */
void CALLBACK udpRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags) {
    DWORD recvBytes;

    GetSystemTime(&start);

    SocketInformation *si = (SocketInformation *)overlapped;

    if (error != 0 || bytesTransferred == 0) {
        if (error) {
            fprintf(stderr, "Error: %d\n", error);
        }
        fprintf(stderr, "Closing socket: %d\n", si->socket);
        closesocket(si->socket);
        return;
    }
    GetSystemTime(&end);

    stats.time += delay(start, end);
    stats.numPackets++;
    stats.size += (int)strlen(si->dataBuf.buf) + 1;

    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));

    si->dataBuf.len = BUFSIZE;
    si->dataBuf.buf = si->buffer;

    GetSystemTime(&start);
    if (WSARecvFrom(si->socket, &(si->dataBuf), 1, &recvBytes, &flags, NULL, NULL, &(si->overlapped), udpRoutine) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            fprintf(stderr, "WSARecv failed: %d\n", WSAGetLastError());
            return;
        }
    }
}

/**
 * @brief runTCPServer Driver for TCP server.  Start winsock, create socket, bind, listen, wait for accept
 * @author Robert Arendac
 * @param sw ServerWindow pointer in order to update GUI
 * @param type STREAM or DGRAM
 * @param protocol TCP or UDP
 */
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

/**
 * @brief runUDPServer Driver for UDP server.  Start winsock, create socket, create work threads
 * @author Robert Arendac
 * @param sw ServerWindow pointer in order to update GUI
 * @param type STREAM or DGRAM
 * @param protocol TCP or UDP
 */
void runUDPServer(ServerWindow *sw, int type, int protocol) {
    SOCKADDR_IN addr;
    HANDLE tcpHandle, statsHandle;

    stats.time = 0;
    stats.numPackets = 0;
    stats.size = 0;

    if (!startWinsock())
        return;

    addr = serverCreateAddress();

    if ((acceptSocket = createSocket(type, protocol)) == NULL)
        return;

    if (!bindSocket(acceptSocket, &addr))
        return;

    tcpHandle = CreateThread(NULL, 0, udpThread, NULL, 0, NULL);
    statsHandle = CreateThread(NULL, 0, statsThread, (void *)sw, 0, NULL);

    WaitForSingleObject(tcpHandle, INFINITE); //So server keeps running
}

/**
 * @brief resetStats Resets statistics back to zero
 * @author Robert Arendac
 */
void resetStats() {
    stats.time = 0;
    stats.numPackets = 0;
    stats.size = 0;
}

/**
 * @brief delay time between two GetTime calls
 * @author Aman Abdulla
 * @param t1 Start time
 * @param t2 End time
 * @return difference between t1 and t2
 *
 * Taken straight from one of the examples on milliways
 */
long delay (SYSTEMTIME t1, SYSTEMTIME t2)
{
    long d;

    d = (t2.wSecond - t1.wSecond) * 1000;
    d += (t2.wMilliseconds - t1.wMilliseconds);
    return(d);
}
