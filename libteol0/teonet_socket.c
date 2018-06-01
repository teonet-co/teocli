#include "teonet_socket.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "teonet_platform.h"

#if defined(TEONET_OS_WINDOWS)
// TODO: Stop using deprecated functions and remove this define.
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>  // To be compatible with historical (BSD) implementations.
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

// Creates a TCP socket.
teonetSocket teosockCreateTcp() {
    return socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

// Establishes a connection to a specified server.
teosockConnectResult teosockConnect(teonetSocket socket, const char* server, uint16_t port) {
    struct sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    int result = inet_pton(AF_INET, server, &serveraddr.sin_addr);

    // Resolve host address if needed.
    if (result != 1) {
        struct hostent* hostp = gethostbyname(server);
        if (hostp == NULL) {
            return TEOSOCK_CONNECT_HOST_NOT_FOUND;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }

    // Connect to server.
    result = connect(socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (result != 0 && errno != EINPROGRESS) {
        return TEOSOCK_CONNECT_FAILED;
    }

    return TEOSOCK_CONNECT_SUCCESS;
}

// Receives data from a connected socket.
ssize_t teosockRecv(teonetSocket socket, char* data, size_t length) {
#if defined(TEONET_OS_WINDOWS)
    if (length > (ssize_t)INT_MAX) {
        // Can't receive this much data.
        return TEOSOCK_SOCKET_ERROR;
    }

    return recv(socket, data, (int)length, 0);
#else
    return read(socket, data, length);
#endif
}

// Sends data on a connected socket.
ssize_t teosockSend(teonetSocket socket, const char* data, size_t length) {
#if defined(TEONET_OS_WINDOWS)
    if (length > (ssize_t)INT_MAX) {
        // Can't send this much data.
        return TEOSOCK_SOCKET_ERROR;
    }

    return send(socket, data, (int)length, 0);
#else
    return write(socket, data, length);
#endif
}

// Determines the status of the socket, waiting if necessary, to perform synchronous read.
teosockSelectReadResult teosockSelectRead(teonetSocket socket, int timeout) {
    fd_set readfds;
    struct timeval timeval_timeout;

    memset(&readfds, 0, sizeof(readfds));
    memset(&timeval_timeout, 0, sizeof(timeval_timeout));
    timeval_timeout.tv_usec = timeout * 1000;

    // Create a descriptor set with specified socket.
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

#if defined(TEONET_OS_WINDOWS)
    int result = select(0, &readfds, NULL, NULL, &timeval_timeout);
#else
    int result = select(socket + 1, &readfds, NULL, NULL, &timeval_timeout);
#endif

    // Make sure that return value is correct.
    if (result > 0) {
        result = TEOSOCK_SELECT_READ_READY;
    }

    return result;
}

// Closes a socket.
int teosockClose(teonetSocket socket) {
#if defined(TEONET_OS_WINDOWS)
    return closesocket(socket);
#else
    return close(socket);
#endif
}

// Disables sends and/or receives on a socket.
int teosockShutdown(teonetSocket socket, teosockShutdownMode mode) {
    return shutdown(socket, mode);
}

// Enables nonblocking mode on specified socket.
int teosockSetNonblock(teonetSocket socket) {
#if defined(TEONET_OS_WINDOWS)
    u_long mode = 1;  // != 0 to enable non-blocking mode.

    return ioctlsocket(socket, FIONBIO, &mode);
#else
    int flags = fcntl(socket, F_GETFL, 0);

    if (flags == -1) {
        return TEOSOCK_SOCKET_ERROR;
    } else {
        return fcntl(socket, F_SETFL, flags | O_NONBLOCK);
    }
#endif
}

// Set TCP_NODELAY option on specified socket.
int teosockSetTcpNodelay(teonetSocket socket) {
    int flag = 1;

    int result = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    if (result != TEOSOCK_SOCKET_SUCCESS) {
        result = TEOSOCK_SOCKET_ERROR;
    }

    return result;
}

// Initialize socket library.
int teosockInit() {
#if defined(TEONET_OS_WINDOWS)
    WORD required_version = MAKEWORD(2, 2);

    WSADATA wsa_data;
    memset(&wsa_data, 0, sizeof(wsa_data));

    int result = WSAStartup(required_version, &wsa_data);

    if (result == TEOSOCK_SOCKET_SUCCESS) {
        // Check that windows socket library support v 2.2.
        if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
            WSACleanup();
            result = TEOSOCK_SOCKET_ERROR;
        }
    } else {
        result = TEOSOCK_SOCKET_ERROR;
    }

    return result;
#else
    return TEOSOCK_SOCKET_SUCCESS;
#endif
}

// Cleanup socket library.
int teosockCleanup() {
#if defined(TEONET_OS_WINDOWS)
    return WSACleanup();
#else
    return TEOSOCK_SOCKET_SUCCESS;
#endif
}
