#include "client.h"
#include "wrappers.h"
#include <stdio.h>
#include <time.h>

SOCKADDR_IN clientCreateAddress(char *host) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(5150);

    return addr;
}

void runClient(int type, int protocol, char *ip, int size, int times) {
    SOCKET sck;
    SOCKADDR_IN addr;
    time_t start, end, total;
    char *data;

    if (!startWinsock())
        return;

    if ((sck = createSocket(type, protocol)) == NULL)
        return;

    if (!connectHost(ip))
        return;

    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip);

    if (!connectToServer(sck, &addr)) {
        return;
    }

    data = (char *)malloc(size);
    if (data == NULL) {
        perror("malloc failed");
        return;
    }

    strnset(data, 'a', size-1);
    data[size - 1] = '\0';

    start = time(NULL);
    if (protocol == IPPROTO_TCP) {
        for (size_t i = 0; i < times; i++) {
            send(sck, data, size, MSG_OOB);
        }
    } else {
        for (size_t i = 0; i < times; i++) {
            sendto(sck, data, size, 0, (SOCKADDR *)&addr, sizeof(addr));
        }
    }
    end = time(NULL);

    total = end - start;

    free(data);
    closesocket(sck);
    WSACleanup();
}
