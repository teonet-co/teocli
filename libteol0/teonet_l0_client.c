/**
 * File:   teonet_lo_client.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on October 12, 2015, 12:32 PM
 */

#include "teonet_platform.h"

#if defined(TEONET_COMPILER_MSVC)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "teonet_l0_client.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#if defined(TEONET_OS_LINUX) || defined(TEONET_OS_MACOS) || defined(TEONET_OS_IOS)
#include <netdb.h>
#include <unistd.h>
#endif

#include "teonet_socket.h"
#include "teonet_time.h"

// Uncomment next line to show debug message
//#define CONNECT_MSG
//#define DEBUG_MSG

// Internal functions
//static ssize_t teoLNullPacketSend(teoLNullConnectData *con, void* pkg, size_t pkg_length);
static ssize_t teoLNullPacketSplit(teoLNullConnectData *con, void* data,
        size_t data_len, ssize_t received);

#if defined(HAVE_MINGW) || defined(_WIN32)
void TEOCLI_API WinSleep(uint32_t dwMilliseconds) {Sleep(dwMilliseconds);}
#endif

// Send connected event
#define send_l0_event(con, event, data, data_length) \
    if(con->event_cb != NULL) { \
        con->event_cb(con, event, data, data_length, con->user_data); \
    }

/**
 * Initialize L0 client library.
 *
 * Startup windows socket library.
 * Calls once per application to initialize this client library.
 */
void teoLNullInit() {
    teosockInit();
}

/**
 * Cleanup L0 client library.
 *
 * Cleanup windows socket library.
 * Calls once per application to cleanup this client library.
 */
void teoLNullCleanup() {
    teosockCleanup();
}

/**
 * Create L0 client packet
 *
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param command Command to peer
 * @param peer Teonet peer
 * @param data Command data
 * @param data_length Command data length
 *
 * @return Length of created packet or zero if buffer to less
 */
size_t teoLNullPacketCreate(void* buffer, size_t buffer_length,
        uint8_t command, const char * peer, const void* data,
        size_t data_length) {

    size_t peer_name_length = strlen(peer) + 1;

    // Check buffer length
    if(buffer_length < sizeof(teoLNullCPacket) + peer_name_length + data_length)
        return 0;

    teoLNullCPacket* pkg = (teoLNullCPacket*) buffer;
    memset(buffer, 0, sizeof(teoLNullCPacket));

    pkg->cmd = command;
    pkg->data_length = (uint16_t)data_length;
    pkg->peer_name_length = (uint8_t)peer_name_length;
    memcpy(pkg->peer_name, peer, pkg->peer_name_length);
    memcpy(pkg->peer_name + pkg->peer_name_length, data, pkg->data_length);
    pkg->checksum = get_byte_checksum(pkg->peer_name, pkg->peer_name_length +
            pkg->data_length);
    pkg->header_checksum = get_byte_checksum(pkg, sizeof(teoLNullCPacket) -
            sizeof(pkg->header_checksum));

    return sizeof(teoLNullCPacket) + pkg->peer_name_length + pkg->data_length;
}

/**
 * Send packet to L0 server/client
 *
 * @param con Pointer to teoLNullConnectData
 * @param pkg Package to send
 * @param pkg_length Package length
 *
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullPacketSend(teoLNullConnectData *con, void* pkg, size_t pkg_length) {
    if (con != NULL) {
        return teosockSend(con->fd, pkg, pkg_length);
    } else {
        return -1;
    }
}

/**
 * Send command to L0 server
 *
 * Create L0 clients packet and send it to L0 server
 *
 * @param con Pointer to teoLNullConnectData
 * @param cmd Command
 * @param peer_name Peer name to send to
 * @param data Pointer to data
 * @param data_length Length of data
 *
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullSend(teoLNullConnectData *con, uint8_t cmd, const char *peer_name,
        void *data, size_t data_length) {

    if(data == NULL) data_length = 0;

    const size_t peer_length = strlen(peer_name) + 1;
    const size_t buf_length = teoLNullBufferSize(peer_length, data_length);
    char *buf = malloc(buf_length);
    ssize_t snd;

    size_t pkg_length = teoLNullPacketCreate(buf, buf_length, cmd, peer_name,
            data, data_length);
    snd = teosockSend(con->fd, buf, pkg_length);

    free(buf);

    return snd;
}

/**
 * Create package for Echo command
 * @param buf Buffer to create packet in
 * @param buf_len Buffer length
 * @param peer_name Peer name to send to
 * @param msg Echo message
 * @return
 */
size_t teoLNullPacketCreateEcho(void *buf, size_t buf_len, const char *peer_name, const char *msg) {

    int64_t current_time = teotimeGetCurrentTime();

    unsigned int time_length = sizeof(current_time);

    const size_t msg_len = strlen(msg) + 1;
    const size_t msg_buf_len = msg_len + time_length;
    void *msg_buf = malloc(msg_buf_len);
    //
    // Fill message buffer
    memcpy(msg_buf, msg, msg_len);
    memcpy((char*)msg_buf + msg_len, &current_time, time_length);
    size_t package_len = teoLNullPacketCreate(buf, buf_len, CMD_L_ECHO, peer_name, msg_buf, msg_buf_len);

    free(msg_buf);

    return package_len;
}

/**
 * Send ECHO command to L0 server
 *
 * Create L0 clients packet and send it to L0 server
 *
 * @param con Pointer to teoLNullConnectData
 * @param peer_name Peer name to send to
 * @param msg Message
 *
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullSendEcho(teoLNullConnectData *con, const char *peer_name,
        const char *msg) {

    // Add current time to the end of message (it should be return
    // back by server)

    char buf[L0_BUFFER_SIZE];
    size_t pkg_length = teoLNullPacketCreateEcho(buf, L0_BUFFER_SIZE, peer_name, msg);

    // Send message with time
    ssize_t snd = teosockSend(con->fd, buf, pkg_length);

    return snd;
}

/**
 * Process ECHO_ANSWER request
 *
 * @param msg Echo answers command data
 * @return Trip time in ms
 */
int64_t teoLNullProccessEchoAnswer(const char *msg) {

    // Get time from answers data
    size_t time_ptr = strlen(msg) + 1;

    const int64_t* time_pointer = (const int64_t*)(msg + time_ptr);
    int64_t time_value = *time_pointer;

    // Calculate trip time
    int64_t trip_time = teotimeGetTimePassed(time_value);

    return trip_time;
}

/**
 * Split or Combine input buffer
 *
 * @param kld Pointer to input buffer
 * @param data Received data buffer
 * @param data_len Received data buffer length
 * @param received Received data length
 *
 * @return Size of packet or Packet state code
 * @retval >0 Packet received
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
static ssize_t teoLNullPacketSplit(teoLNullConnectData *kld, void* data,
        size_t data_len, ssize_t received) {

    ssize_t retval = -1;

    #ifdef DEBUG_MSG
    printf("L0 Client: "
       "Got %" PRId32 " bytes of packet...\n",
       (int)received);
    #endif

    // Check end of previous buffer
    if(kld->last_packet_ptr > 0) {

        kld->read_buffer_ptr = kld->read_buffer_ptr - kld->last_packet_ptr;
        if(kld->read_buffer_ptr > 0) {

            #ifdef DEBUG_MSG
            printf("L0 Client: "
                   "Use %" PRId32 " bytes from previously received data...\n",
                   (int)(kld->read_buffer_ptr));
            #endif

            memmove(kld->read_buffer, (char*)kld->read_buffer +
                    kld->last_packet_ptr, kld->read_buffer_ptr);
        }
        kld->last_packet_ptr = 0;
    }

    // Increase buffer size
    if((size_t)received > kld->read_buffer_size - kld->read_buffer_ptr) {

        // Increase read buffer size
        kld->read_buffer_size += data_len; //received;
        if(kld->read_buffer != NULL)
            kld->read_buffer = realloc(kld->read_buffer, kld->read_buffer_size);
        else
            kld->read_buffer = malloc(kld->read_buffer_size);

        #ifdef DEBUG_MSG
        printf("L0 Client: "
               "Increase read buffer to new size: %" PRId32 " bytes ...\n",
               (int)kld->read_buffer_size);
        #endif
    }

    // Add received data to the read buffer
    if(received > 0) {
        memmove((char*)kld->read_buffer + kld->read_buffer_ptr, data, received);
        kld->read_buffer_ptr += received;
    }

    teoLNullCPacket *packet = (teoLNullCPacket *)kld->read_buffer;
    ssize_t len;

    // \todo Check packet

    // Process read buffer
    if(kld->read_buffer_ptr - kld->last_packet_ptr > sizeof(teoLNullCPacket) &&
       kld->read_buffer_ptr - kld->last_packet_ptr >=
            (size_t)(len = sizeof(teoLNullCPacket) + packet->peer_name_length +
            packet->data_length)) {

        // Check checksum
        uint8_t header_checksum = get_byte_checksum(packet,
                sizeof(teoLNullCPacket) -
                sizeof(packet->header_checksum));
        uint8_t checksum = get_byte_checksum(packet->peer_name,
                packet->peer_name_length + packet->data_length);
        if(packet->header_checksum == header_checksum &&
           packet->checksum == checksum) {

            // Packet has received - return packet size
            retval = len;
            kld->last_packet_ptr += len;

            #ifdef DEBUG_MSG
            printf("L0 Server: "
                "Identify packet %" PRId32 " bytes length ...\n",
                (int)retval);
            #endif
        }

        // Wrong checksum - drop this packet
        else {

            // Wrong packet received - return -2
            kld->read_buffer_ptr = 0;
            kld->last_packet_ptr = 0;
            retval = -2;

            #ifdef DEBUG_MSG
            printf("L0 Client: "
                "Wrong packet %" PRId32 " bytes length; dropped ...\n",
                (int)len);
            #endif
        }
    }
    else {

        #ifdef DEBUG_MSG
        printf("L0 Client: "
               "Wait next part of packet, now it has %" PRId32 " bytes ...\n",
               (int)kld->read_buffer_ptr);
        #endif
    }

    return retval;
}

/**
 * Receive packet from L0 server and split or combine it
 *
 * @param con Pointer to teoLNullConnectData
 *
 * @return Size of packet or Packet state code
 * @retval >0 Packet received
 * @retval  0 Disconnected
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
ssize_t teoLNullRecv(teoLNullConnectData *con) {

    char buf[L0_BUFFER_SIZE];

    ssize_t rc = teosockRecv(con->fd, buf, L0_BUFFER_SIZE);
    if(rc != 0) rc = teoLNullRecvCheck(con, buf, rc);

    return rc;
}

/**
 * Check received packet
 *
 * @param con
 * @param buf
 * @param rc
 *
 * @return
 * @return Size of packet or Packet state code
 * @retval >0 Packet received
 * @retval -1 Packet not receiving yet (got part of packet)
 * @retval -2 Wrong packet received (dropped)
 */
ssize_t teoLNullRecvCheck(teoLNullConnectData *con, char * buf, ssize_t rc) {

    rc = teoLNullPacketSplit(con, buf, L0_BUFFER_SIZE, rc != -1 ? rc : 0);

    // Send echo answer to echo command
    if(rc > 0 && con->fd) {
        teoLNullCPacket *cp = (teoLNullCPacket*) con->read_buffer;
        if(cp->cmd == CMD_L_ECHO) {
            char *data = cp->peer_name + cp->peer_name_length;
            teoLNullSend(con, CMD_L_ECHO_ANSWER, cp->peer_name, data,
                    cp->data_length );
        }
    }

    return rc;
}

/**
 * Receive data from L0 server during timeout
 *
 * @param con Pointer to teoLNullConnectData
 * @param timeout Timeout in uSeconds
 *
 * @return Number of received bytes or -1 at timeout
 */
ssize_t teoLNullRecvTimeout(teoLNullConnectData *con, uint32_t timeout) {

    ssize_t rc;
    uint32_t t = 0;
    const uint32_t wait = 50;

    // Receive answer from server, CMD_L_L0_CLIENTS_ANSWER
    while((rc = teoLNullRecv(con)) == -1 && t < timeout) {
        teoLNullSleep(wait);
        t += wait;
    }

    return rc;
}

/**
 * Create L0 clients initialization packet
 *
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param host_name Name of this L0 client
 *
 * @return Length of created packet or zero if buffer to less
 */
size_t teoLNullPacketCreateLogin(void* buffer, size_t buffer_length,
        const char* host_name) {

    return teoLNullPacketCreate(buffer, buffer_length, 0, "", host_name,
            strlen(host_name) + 1);
}

/**
 * Login to L0 server
 *
 * Create and send L0 clients initialization packet
 *
 * @param con Pointer to teoLNullConnectData
 * @param host_name Client name
 *
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullLogin(teoLNullConnectData *con, const char* host_name) {

    ssize_t snd;
    const size_t buf_len = teoLNullBufferSize(1, strlen(host_name) + 1);

    // Buffer
    #if defined(_WIN32)
    char *buf = malloc(buf_len);
    #else
    char buf[buf_len];
    #endif

    size_t pkg_length = teoLNullPacketCreateLogin(buf, buf_len, host_name);
    if (pkg_length == 0) {
        // Free buffer
        #if defined(_WIN32)
        free(buf);
        #endif
        return 0;
    }
    snd = teosockSend(con->fd, buf, pkg_length);

    // Free buffer
    #if defined(_WIN32)
    free(buf);
    #endif

    return snd;
}

/**
 * Calculate checksum
 *
 * Calculate byte checksum in data buffer
 *
 * @param data Pointer to data buffer
 * @param data_length Length of the data buffer to calculate checksum
 *
 * @return Byte checksum of the input buffer
 */
uint8_t get_byte_checksum(void *data, size_t data_length) {

    int i;
    uint8_t *ch, checksum = 0;
    for(i = 0; i < (int)data_length; i++) {

        ch = (uint8_t*)((char*)data + i);
        checksum += *ch;
    }

    return checksum;
}

/**
 * Set socket or SD to non blocking mode
 *
 * @param sd Socket descriptor
 */
//void set_nonblock(int sd) {
//    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
//    //-------------------------
//    // Set the socket I/O mode: In this case FIONBIO
//    // enables or disables the blocking mode for the
//    // socket based on the numerical value of iMode.
//    // If iMode = 0, blocking is enabled;
//    // If iMode != 0, non-blocking mode is enabled.
//
//    int iResult;
//    u_long iMode = 1;
//
//    iResult = ioctlsocket(sd, FIONBIO, &iMode);
//    if (iResult != NO_ERROR)
//      printf("ioctlsocket failed with error: %d\n", iResult);
//
//    #else
//    int flags;
//
//    flags = fcntl(sd, F_GETFL, 0);
//    fcntl(sd, F_SETFL, flags | O_NONBLOCK);
//    #endif
//}
inline void set_nonblock(int sd) __attribute_deprecated__;
inline void set_nonblock(int sd) { teosockSetBlockingMode(sd, TEOSOCK_NON_BLOCKING_MODE); }

/**
 * Set TCP NODELAY option
 *
 * @param sd TCP socket descriptor
 *
 * @return Result of setting. Success if >= 0.
 */
//int set_tcp_nodelay(int sd) {
//
//    int result = 0;
//    int flag = 1;
//
//    result = setsockopt(sd,           // socket affected
//                        IPPROTO_TCP,     // set option at TCP level
//                        TCP_NODELAY,     // name of option
//                        (char *) &flag,  // the cast is historical cruft
//                        sizeof(flag));   // length of option value
//    if (result < 0) {
//
//        printf("Set TCP_NODELAY of sd %d error\n", sd);
//    }
//
//    return result;
//}
int set_tcp_nodelay(int sd) __attribute_deprecated__;
inline int set_tcp_nodelay(int sd) { return teosockSetTcpNodelay(sd); }

/**
 * Wait socket data during timeout and call callback if data received
 *
 * @param con Pointer to teoLNullConnectData
 * @param timeout Timeout of wait socket read event in ms
 *
 * @return 0 - if disconnected or 1 other way
 */
int teoLNullReadEventLoop(teoLNullConnectData *con, int timeout) {

    int rv, retval = 1;

    rv = teosockSelect(con->fd, TEOSOCK_SELECT_MODE_READ, timeout);

    // Error
    if (rv == TEOSOCK_SELECT_ERROR) {
        int error = errno;
        if (error == EINTR) {
            // just an interrupted system call
        }
        else printf("select(fd = %" PRId32 ") handle error %" PRId32 ": %s\n",
                (int)con->fd, error, strerror(error));
    }

    // Timeout
    else if(rv == TEOSOCK_SELECT_TIMEOUT) { // Idle or Timeout event

        send_l0_event(con, EV_L_IDLE, NULL, 0);
    }

    // There is a data in sd
    else {

        ssize_t rc;
        while((rc = teoLNullRecv(con)) != -1) {

            if(rc > 0) {
                send_l0_event(con, EV_L_RECEIVED, con->read_buffer, rc);
            } else if(rc == 0) {
                send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
                con->status = CON_STATUS_NOT_CONNECTED;
                retval = 0;
                break;
            }
        }
    }

    // Send Tick event
    send_l0_event(con, EV_L_TICK, NULL, 0);

    return retval;
}

/**
 * Create TCP client and connect to server with event callback
 *
 * @param server Server IP or name
 * @param port Server port
 * @param event_cb Pointer to event callback function
 * @param user_data Pointer to user data which will be send to event callback
 *
 * @return Pointer to teoLNullConnectData. Null if no memory error
 * @retval teoLNullConnectData::status== 1   - Success connection
 * @retval teoLNullConnectData::status==-1 - Create socket error
 * @retval teoLNullConnectData::status==-2 - HOST NOT FOUND error
 * @retval teoLNullConnectData::status==-3 - Client-connect() error
 */
teoLNullConnectData* teoLNullConnectE(const char *server, uint16_t port,
        teoLNullEventsCb event_cb, void *user_data) {

    int result;
    teoLNullConnectData *con = malloc(sizeof(teoLNullConnectData));
    if(con == NULL) return con;

    con->last_packet_ptr = 0;
    con->read_buffer = NULL;
    con->read_buffer_ptr = 0;
    con->read_buffer_size = 0;
    con->event_cb = event_cb;
    con->user_data = user_data;

    con->fd = teosockCreateTcp();

    if(con->fd == TEOSOCK_INVALID_SOCKET) {
        printf("Client-socket() error\n");
        con->fd = 0;
        con->status = CON_STATUS_SOCKET_ERROR;
        send_l0_event(con, EV_L_CONNECTED, &con->status, sizeof(con->status));
        return con;
    }
    else {
        con->status = CON_STATUS_CONNECTED;
        #ifdef DEBUG_MSG
        printf("Client-socket() OK\n");
        #endif
    }

    #ifdef CONNECT_MSG
    printf("Connecting to the server %s at port %" PRIu16 " ...\n", server, port);
    #endif

    result = teosockConnectTimeout(con->fd, server, port, 5000);

    if (result == TEOSOCK_CONNECT_HOST_NOT_FOUND) {
        printf("HOST NOT FOUND --> h_errno = %" PRId32 "\n", h_errno);
        teosockClose(con->fd);
        con->fd = 0;
        con->status = CON_STATUS_HOST_ERROR;
        send_l0_event(con, EV_L_CONNECTED, &con->status, sizeof(con->status));
        return con;
    }
    else if (result == TEOSOCK_CONNECT_FAILED) {
        int error = errno;
        printf("Client-connect() error: %" PRId32 ", %s\n", error, strerror(error));
        teosockClose(con->fd);
        con->fd = 0;
        con->status = CON_STATUS_CONNECTION_ERROR;
        send_l0_event(con, EV_L_CONNECTED, &con->status, sizeof(con->status));
        return con;
    }
    else {
        #ifdef CONNECT_MSG
        printf("Connection established ...\n");
        #endif
    }

    // Set TCP_NODELAY option
    teosockSetTcpNodelay(con->fd);

    // Send connected event
    send_l0_event(con, EV_L_CONNECTED, &con->status, sizeof(con->status));

    return con;
}

/**
 * Create TCP client and connect to server without event callback
 *
 * @param server Server IP or name
 * @param port Server port
 *
 * @return Pointer to teoLNullConnectData. Null if no memory error
 * @retval teoLNullConnectData::status== 1 - Success connection
 * @retval teoLNullConnectData::status==-1 - Create socket error
 * @retval teoLNullConnectData::status==-2 - HOST NOT FOUND error
 * @retval teoLNullConnectData::status==-3 - Client-connect() error
 */
teoLNullConnectData* teoLNullConnect(const char *server, uint16_t port) {

    return teoLNullConnectE(server, port, NULL, NULL);
}

/**
 * Disconnect from server and free teoLNullConnectData
 *
 * @param con Pointer to teoLNullConnectData
 */
void teoLNullDisconnect(teoLNullConnectData *con) {

    if(con != NULL) {
        if(con->fd > 0) teosockClose(con->fd);
        if(con->read_buffer != NULL) free(con->read_buffer);
        free(con);
    }
}

/**
 * Disconnect from server and free teoLNullConnectData
 *
 * @param con Pointer to teoLNullConnectData
 */
void teoLNullShutdown(teoLNullConnectData *con) {

    if (con != NULL && con->fd > 0) {
        teosockShutdown(con->fd, TEOSOCK_SHUTDOWN_RDWR);
    }
}

#undef DEBUG_MSG
