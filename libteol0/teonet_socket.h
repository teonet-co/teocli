/**
 * @file teonet_socket.h
 * @brief Cross platform functions for working with sockets.
 */

#ifndef TEONET_SOCKET_H
#define TEONET_SOCKET_H

#include "teonet_platform.h"

#include <stdint.h>

#if defined(TEONET_OS_WINDOWS)
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#endif

#if defined(TEONET_OS_WINDOWS)
// Windows does not define POSIX type ssize_t.
typedef intptr_t ssize_t;
#else
#include <unistd.h>
#endif

/// Alias for socket type on current platform. SOCKET on windows, int on linux.
#if defined(TEONET_OS_WINDOWS)
typedef SOCKET teonetSocket;
#else
typedef int teonetSocket;
#endif

/// Unnamed enumeration with integer constants.
typedef enum {
    TEOSOCK_SOCKET_SUCCESS = 0,  ///< Value for indicating successful result in socket function.

#if defined(TEONET_OS_WINDOWS)
    TEOSOCK_SOCKET_ERROR = SOCKET_ERROR,  ///< Value for indicating error result in socket function.
    TEOSOCK_INVALID_SOCKET = INVALID_SOCKET,  ///< Value for indicating invalid socket descriptor.
#else
    TEOSOCK_SOCKET_ERROR = -1,  ///< Value for indicating error result in socket function.
    TEOSOCK_INVALID_SOCKET = -1,  ///< Value for indicating invalid socket descriptor.
#endif
} TEOSOCK_ERRORS;

/**
 * Creates a TCP socket.
 *
 * @returns TEOSOCK_INVALID_SOCKET on error, socket handle otherwise.
 */
teonetSocket teosockCreateTcp();

/// Result enumeration for teosockConnect() function.
typedef enum teosockConnectResult {
    TEOSOCK_CONNECT_SUCCESS = 1,  ///< Successful connection.
    TEOSOCK_CONNECT_HOST_NOT_FOUND = -1,  ///< Failed to resolve host address.
    TEOSOCK_CONNECT_FAILED = -2,  ///< Failed to connect to server.
} teosockConnectResult;

/**
 * Establishes a connection to a specified server.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 * @param server Server IP address or domain name.
 * @param port Port to connect to.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_CONNECT_SUCCESS if connection successfully established.
 * @retval TEOSOCK_CONNECT_HOST_NOT_FOUND if failed to resolve host address.
 * @retval TEOSOCK_CONNECT_FAILED if failed to connect to server.
 */
teosockConnectResult teosockConnect(teonetSocket socket, const char* server, uint16_t port);

/**
 * Receives data from a connected socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 * @param data A pointer to the buffer to store the data.
 * @param length The length of buffer in bytes.
 *
 * @returns TEOSOCK_SOCKET_ERROR on error, amount of received bytes otherwise.
 */
ssize_t teosockRecv(teonetSocket socket, char* data, size_t length);

/**
 * Sends data on a connected socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 *
 * @returns TEOSOCK_SOCKET_ERROR on error, amount of sent bytes otherwise.
 * @param data A pointer to the buffer with data.
 * @param length The length of data to be transmitted, in bytes.
 *
 * @note Amount of bytes sent can be less than the number requested to be sent
 * in the @p length parameter.
 */
ssize_t teosockSend(teonetSocket socket, const char* data, size_t length);

/// Result enumeration for teosockSelectRead() function.
typedef enum teosockSelectReadResult {
    TEOSOCK_SELECT_READ_READY = 1,  ///< Socket have data ready to be read.
    TEOSOCK_SELECT_READ_TIMEOUT = 0,  ///< No data was received before reaching timeout.
    TEOSOCK_SELECT_READ_ERROR = -1,  ///< An error occured.
} teosockSelectReadResult;

/**
 * Determines the status of the socket, waiting if necessary, to perform synchronous read.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 * @param timeout The amount of time to wait before rturning timeout in milliseconds.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SELECT_READ_READY if socket have data ready to be read.
 * @retval TEOSOCK_SELECT_READ_TIMEOUT if no data was received before reaching timeout.
 * @retval TEOSOCK_SELECT_READ_ERROR if an error occured.
 */
teosockSelectReadResult teosockSelectRead(teonetSocket socket, int timeout);

/**
 * Closes a socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 */
int teosockClose(teonetSocket socket);

/// Enumeration for specifying socket shutdown mode in teosockShutdown() function.
typedef enum teosockShutdownMode {
    /// Shutdown receiving data. SHUT_RD on Unix, SD_RECEIVE on Windows.
    /// @note Data that is already in socket buffer still may be received.
    TEOSOCK_SHUTDOWN_RD = 0,
    /// Shutdown sending data. SHUT_WR on Unix, SD_SEND on Windows.
    TEOSOCK_SHUTDOWN_WR = 1,
    /// Shutdown both receiving and sending data. SHUT_RDWR on Unix, SD_BOTH on Windows.
    TEOSOCK_SHUTDOWN_RDWR = 2,
} teosockShutdownMode;

/**
 * Disables sends and/or receives on a socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 * @param mode Socket shutdown mode. See #teosockShutdownMode.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 */
int teosockShutdown(teonetSocket socket, teosockShutdownMode mode);

/**
 * Enables nonblocking mode on a socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 */
int teosockSetNonblock(teonetSocket socket);

/**
 * Set TCP_NODELAY option on a socket.
 *
 * @param socket Socket descriptor obtained using teosockCreateTcp() function.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 *
 * @warning As stated here https://msdn.microsoft.com/en-us/library/ms740476(v=vs.85).aspx
 * we should not set TCP_NODELAY unless the impact of doing so is well-understood
 * and desired because setting TCP_NODELAY can have a significant negative impact
 * on network and application performance.
 */
int teosockSetTcpNodelay(teonetSocket socket);

/**
 * Initialize socket library.
 *
 * Call this function before any other socket function.
 * On Windows this function initiates use of the Winsock 2 library.
 * This function does nothing on linux.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 */
int teosockInit();

/**
 * Cleanup socket library.
 *
 * Call this function when socket functions are no longer needed.
 * On Windows this function terminates use of the Winsock 2 library.
 * This function does nothing on linux.
 *
 * @returns Result of operation.
 *
 * @retval TEOSOCK_SOCKET_SUCCESS if operation completed successsfully.
 * @retval TEOSOCK_SOCKET_ERROR if operation failed.
 */
int teosockCleanup();
#endif
