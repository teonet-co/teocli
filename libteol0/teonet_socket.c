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

#if !defined(TEONET_COMPILER_MINGW)
    int result = inet_pton(AF_INET, server, &serveraddr.sin_addr);

    // Resolve host address if needed.
    if (result != 1) {
        struct hostent* hostp = gethostbyname(server);
        if (hostp == NULL) {
            return TEOSOCK_CONNECT_HOST_NOT_FOUND;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }
#else
    // MinGW compiler still doest not support inet_pton().
    serveraddr.sin_addr.s_addr = inet_addr(server);

    // Resolve host address if needed.
    if (serveraddr.sin_addr.s_addr == (unsigned long)INADDR_NONE) {
        struct hostent* hostp = gethostbyname(server);
        if (hostp == NULL) {
            return TEOSOCK_CONNECT_HOST_NOT_FOUND;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }
#endif

    // Connect to server.
    int connect_result = connect(socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (connect_result != 0 && errno != EINPROGRESS) {
        return TEOSOCK_CONNECT_FAILED;
    }

    return TEOSOCK_CONNECT_SUCCESS;
}

// Establishes a connection to a specified server.
teosockConnectResult teosockConnectTimeout(teonetSocket socket, const char* server, uint16_t port, int timeout) {
    struct sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

#if !defined(TEONET_COMPILER_MINGW)
    int result = inet_pton(AF_INET, server, &serveraddr.sin_addr);

    // Resolve host address if needed.
    if (result != 1) {
        struct hostent* hostp = gethostbyname(server);
        if (hostp == NULL) {
            return TEOSOCK_CONNECT_HOST_NOT_FOUND;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }
#else
    // MinGW compiler still doest not support inet_pton().
    serveraddr.sin_addr.s_addr = inet_addr(server);

    // Resolve host address if needed.
    if (serveraddr.sin_addr.s_addr == (unsigned long)INADDR_NONE) {
        struct hostent* hostp = gethostbyname(server);
        if (hostp == NULL) {
            return TEOSOCK_CONNECT_HOST_NOT_FOUND;
        }

        memcpy(&serveraddr.sin_addr, hostp->h_addr_list[0], sizeof(serveraddr.sin_addr));
    }
#endif

    int set_non_locking_result = teosockSetBlockingMode(socket, TEOSOCK_NON_BLOCKING_MODE);

    if (set_non_locking_result == TEOSOCK_SOCKET_ERROR) {
        teosockClose(socket);
        return TEOSOCK_CONNECT_FAILED;
    }

    // Connect to server.
    int connect_result = connect(socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    if (connect_result == 0) {
        return TEOSOCK_CONNECT_SUCCESS;
    }
    else {
        int in_progress = 0;

#if defined(TEONET_OS_WINDOWS)
        int error_code = WSAGetLastError();

        if (error_code == WSAEWOULDBLOCK) {
            in_progress = 1;
        }
#else
        int error_code = errno;

        if (error_code == EINPROGRESS) {
            in_progress = 1;
        }
#endif

        if (in_progress != 1) {
            teosockClose(socket);
            return TEOSOCK_CONNECT_FAILED;
        }
    }

    int select_result = teosockSelect(socket, TEOSOCK_SELECT_MODE_WRITE | TEOSOCK_SELECT_MODE_ERROR, timeout);

    if (select_result == TEOSOCK_SELECT_TIMEOUT || select_result == TEOSOCK_SELECT_ERROR) {
        teosockClose(socket);
        return TEOSOCK_CONNECT_FAILED;
    }

    int error = 0;
    socklen_t error_len = sizeof(error);

    int getsockopt_result = getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&error, &error_len);

    if (getsockopt_result == TEOSOCK_SOCKET_ERROR || error != 0) {
        teosockClose(socket);
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

// Determines the status of the socket, waiting if necessary, to perform synchronous operation.
teosockSelectResult teosockSelect(teonetSocket socket, int status_mask, int timeout) {
    fd_set socket_fd_set;
    struct timeval timeval_timeout;

    memset(&socket_fd_set, 0, sizeof(socket_fd_set));
    memset(&timeval_timeout, 0, sizeof(timeval_timeout));

    timeval_timeout.tv_sec = timeout / 1000;
    timeval_timeout.tv_usec = (timeout % 1000) * 1000;

    // Create a descriptor set with specified socket.
    FD_ZERO(&socket_fd_set);
    FD_SET(socket, &socket_fd_set);

    fd_set* read_fd_set = (status_mask & TEOSOCK_SELECT_MODE_READ) ? &socket_fd_set : NULL;
    fd_set* write_fd_set = (status_mask & TEOSOCK_SELECT_MODE_WRITE) ? &socket_fd_set : NULL;
    fd_set* error_fd_set = (status_mask & TEOSOCK_SELECT_MODE_ERROR) ? &socket_fd_set : NULL;

#if defined(TEONET_OS_WINDOWS)
    int result = select(0, read_fd_set, write_fd_set, error_fd_set, &timeval_timeout);
#else
    int result = select(socket + 1, read_fd_set, write_fd_set, error_fd_set, &timeval_timeout);
#endif

    // Make sure that return value is correct.
    if (result > 0) {
        result = TEOSOCK_SELECT_READY;
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

// Sets blocking or non-blocking mode for specified socket.
int teosockSetBlockingMode(teonetSocket socket, teosockBlockingMode blocking_mode) {
#if defined(TEONET_OS_WINDOWS)
    u_long mode = (u_long)blocking_mode;

    return ioctlsocket(socket, FIONBIO, &mode);
#else
    int flags = fcntl(socket, F_GETFL, 0);

    if (flags == -1) {
        return TEOSOCK_SOCKET_ERROR;
    } else {
        int new_flags;
        if (blocking_mode == TEOSOCK_BLOCKING_MODE) {
            new_flags = flags & ~O_NONBLOCK;
        } else {
            new_flags = flags | O_NONBLOCK;
        }

        int result;
        if (new_flags == flags) {
            result = TEOSOCK_SOCKET_SUCCESS;
        } else {
            result = fcntl(socket, F_SETFL, new_flags);

            if (result != TEOSOCK_SOCKET_ERROR) {
                result = TEOSOCK_SOCKET_SUCCESS;
            }
        }

        return result;
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
