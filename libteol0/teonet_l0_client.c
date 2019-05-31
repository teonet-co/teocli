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
#include <stdarg.h>

#if defined(TEONET_OS_LINUX) || defined(TEONET_OS_MACOS) || defined(TEONET_OS_IOS)
#include <netdb.h>
#include <unistd.h>
#endif

#include "teonet_socket.h"
#include "teonet_time.h"

//#define TRUDP_PROTOCOL 
#define DEBUG 0

static const int BUFFER_SIZE = 2048;
//char *buffer;
char *remote_address;
int remote_port_i;
/**
 * Show debug message
 *
 * @param fmt
 * @param ...
 */
static void debug(const void *tru, int mode, char *fmt, ...)
{
    static unsigned long idx = 0;
    va_list ap;
    if(DEBUG) {
        fflush(stdout);
        fprintf(stderr, "%lu %.3f debug: ", ++idx, trudpGetTimestamp() / 1000.0);
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
    }
}

// Uncomment next line to show debug message
//#define CONNECT_MSG
//#define DEBUG_MSG

// Internal functions
static ssize_t
teoLNullPacketSplit(teoLNullConnectData *con, void* data, size_t data_len, ssize_t received);

#if defined(HAVE_MINGW) || defined(_WIN32)
void TEOCLI_API WinSleep(uint32_t dwMilliseconds) {Sleep(dwMilliseconds);}
#endif

/**
 * Initialize L0 client library.
 *
 * Startup windows socket library.
 * Calls once per application to initialize this client library.
 */
void
teoLNullInit()
{
    teosockInit();
}


/**
 * Cleanup L0 client library.
 *
 * Cleanup windows socket library.
 * Calls once per application to cleanup this client library.
 */
void
teoLNullCleanup()
{
    teosockCleanup();
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
teoLNullConnectData*
teoLNullConnect(const char *server, uint16_t port)
{
    return teoLNullConnectE(server, port, NULL, NULL);
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
teoLNullConnectData*
teoLNullConnectE(const char *server, uint16_t port, teoLNullEventsCb event_cb, void *user_data)
{
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
 * Disconnect from server and free teoLNullConnectData
 *
 * @param con Pointer to teoLNullConnectData
 */
void
teoLNullDisconnect(teoLNullConnectData *con)
{
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
void
teoLNullShutdown(teoLNullConnectData *con)
{
    if (con != NULL && con->fd > 0) {
        teosockShutdown(con->fd, TEOSOCK_SHUTDOWN_RDWR);
    }
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
ssize_t
teoLNullLogin(void *connection, const char* host_name, PROTOCOL proto)
{
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
    if (proto == TCP) {
        teoLNullConnectData *con = (teoLNullConnectData *)connection;
        snd = teosockSend(con->fd, buf, pkg_length);
    } else if (proto == TR_UDP) {
        trudpChannelData *con = (trudpChannelData *)connection;
        snd = trudpChannelSendData(con, buf, pkg_length);
    }

    // Free buffer
    #if defined(_WIN32)
    free(buf);
    #endif

    return snd;
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
ssize_t
teoLNullSend(teoLNullConnectData *con, uint8_t cmd, const char *peer_name,
        void *data, size_t data_length)
{
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
 * Send ECHO command to L0 server
 *
 * Create L0 clients packet and send it to L0 server
 *
 * @param con Pointer to teoLNullConnectData
 teoLNullSendEcho* @param peer_name Peer name to send to
 * @param msg Message
 *
 * @return Length of send data or -1 at error
 */
ssize_t
teoLNullSendEcho(teoLNullConnectData *con, const char *peer_name, const char *msg)
{
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
int64_t
teoLNullProccessEchoAnswer(const char *msg)
{
    // Get time from answers data
    size_t time_ptr = strlen(msg) + 1;

    const int64_t* time_pointer = (const int64_t*)(msg + time_ptr);
    int64_t time_value = *time_pointer;

    // Calculate trip time
    int64_t trip_time = teotimeGetTimePassed(time_value);

    return trip_time;
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
ssize_t
teoLNullRecv(teoLNullConnectData *con)
{
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
ssize_t
teoLNullRecvCheck(teoLNullConnectData *con, char * buf, ssize_t rc)
{
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
ssize_t
teoLNullRecvTimeout(teoLNullConnectData *con, uint32_t timeout)
{
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
 * Wait socket data during timeout and call callback if data received
 *
 * @param con Pointer to teoLNullConnectData
 * @param timeout Timeout of wait socket read event in ms
 *
 * @return 0 - if disconnected or 1 other way
 */
int
teoLNullReadEventLoop(teoLNullConnectData *con, int timeout)
{
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
 * Create L0 clients initialization packet
 *
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param host_name Name of this L0 client
 *
 * @return Length of created packet or zero if buffer to less
 */
size_t
teoLNullPacketCreateLogin(void* buffer, size_t buffer_length, const char* host_name)
{
    return teoLNullPacketCreate(buffer, buffer_length, 0, "", host_name,
            strlen(host_name) + 1);
}


/**
 * Create package for Echo command
 * @param buf Buffer to create packet in
 * @param buf_len Buffer length
 * @param peer_name Peer name to send to
 * @param msg Echo message
 * @return
 */
size_t
teoLNullPacketCreateEcho(void *buf, size_t buf_len, const char *peer_name, const char *msg)
{
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
size_t
teoLNullPacketCreate(void* buffer, size_t buffer_length, uint8_t command,
        const char *peer, const void *data, size_t data_length)
{
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
ssize_t
teoLNullPacketSend(teoLNullConnectData *con, void *pkg, size_t pkg_length)
{
    if (con != NULL) {
        return teosockSend(con->fd, pkg, pkg_length);
    } else {
        return -1;
    }
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
static ssize_t
teoLNullPacketSplit(teoLNullConnectData *kld, void* data, size_t data_len, ssize_t received)
{
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
 * TR-UDP event callback
 *
 * @param tcd_pointer
 * @param event
 * @param data
 * @param data_length
 * @param user_data
 */
void
trudpEventCback(void *tcd_pointer, int event, void *data, size_t data_length,
        void *user_data)
{

    trudpChannelData *tcd = (trudpChannelData *)tcd_pointer;
    void *tru = user_data;

    switch(event) {

        // CONNECTED event
        // @param data NULL
        // @param user_data NULL
        case CONNECTED: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "Connect channel %s\n", key);

        } break;

        // DISCONNECTED event
        // @param tcd Pointer to trudpData
        // @param data Last packet received
        // @param user_data NULL
        case DISCONNECTED: {

            char *key = trudpChannelMakeKey(tcd);
            if(data_length == sizeof(uint32_t)) {
                uint32_t last_received = *(uint32_t*)data;
                debug(tru, DEBUG,
                      "Disconnect channel %s, last received: %.6f sec\n",
                      key, last_received / 1000000.0);
                trudpChannelDestroy(tcd);
            }
            else debug(tru, DEBUG,  "Disconnected channel %s\n", key);

            tcd->connected_f = 0;

        } break;

        // GOT_RESET event
        // @param data NULL
        // @param user_data NULL
        case GOT_RESET: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "got TRU_RESET packet from channel %s\n", key);
            
            tcd->connected_f = 0;

        } break;

        // SEND_RESET event
        // @param data Pointer to uint32_t id or NULL (data_size == 0)
        // @param user_data NULL
        case SEND_RESET: {

            char *key = trudpChannelMakeKey(tcd);

            if(!data) debug(tru, DEBUG,  "Send reset: to channel %s\n", key);
            else {

                uint32_t id = (data_length == sizeof(uint32_t)) ? *(uint32_t*)data:0;

                if(!id)
                  debug(tru, DEBUG,
                    "Send reset: "
                    "Not expected packet with id = 0 received from channel %s\n",
                    key);
                else
                  debug(tru, DEBUG,
                    "Send reset: "
                    "High send packet number (%d) at channel %s\n",
                    id, key);
                }

        } break;

        // GOT_ACK_RESET event: got ACK to reset command
        // @param data NULL
        // @param user_data NULL
        case GOT_ACK_RESET: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "Got ACK to RESET packet at channel %s\n", key);

        } break;

        // GOT_ACK_PING event: got ACK to ping command
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_ACK_PING: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,
              "got ACK to PING packet at channel %s, data: %s, %.3f(%.3f) ms\n",
              key, (char*)data,
              (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

        } break;

        // GOT_PING event: got PING packet, data
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_PING: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,
              "got PING packet at channel %s, data: %s\n",
              key, (char*)data);

        } break;

        // Got ACK event
        // @param data Pointer to ACK packet
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_ACK: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "got ACK id=%u at channel %s, %.3f(%.3f) ms\n",
                  trudpPacketGetId(data/*trudpPacketGetPacket(data)*/),
                  key, (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

            #if USE_LIBEV
            // trudp_start_send_queue_cb(&psd, 0);
            #endif

        } break;

        // Got DATA event
        // @param data Pointer to data
        // @param data_length Length of data
        // @param user_data NULL Pointer to teoLNullConnectData
        case GOT_DATA: {

            teoLNullConnectData *con = user_data;

            uint32_t id = trudpPacketGetId(trudpPacketGetPacket(data));
            size_t length = trudpPacketGetPacketLength(trudpPacketGetPacket(data));

            ssize_t rc = teoLNullRecvCheck(con, data, data_length);
            if(!(rc > 0)) {
                debug(tru, DEBUG, "got %d byte data to buffer\n", data_length);
                break;
            }
            
            data_length = rc;
            data = con->read_buffer;
            
            teoLNullCPacket *cp = trudpPacketGetData(trudpPacketGetPacket(data));
            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,
                "got %d byte data at channel %s [%.3f(%.3f) ms], id=%u, "
                "peer: %s, cmd: %d, data length: %d, data: %s\n",
                length,
//                trudpPacketGetPacketLength(trudpPacketGetPacket(data)),
                key,
                (double)tcd->triptime / 1000.0,
                (double)tcd->triptimeMiddle / 1000.0,
                id,
//                trudpPacketGetId(trudpPacketGetPacket(data)),
                cp->peer_name,
                cp->cmd,
                cp->data_length,
                cp->peer_name + cp->peer_name_length);

            // Process ECHO command
            if(cp->cmd == CMD_L_ECHO) {
                char *data = cp->peer_name + cp->peer_name_length;

//                char buf[BUFFER_SIZE];
//                size_t pkg_length = teoLNullPacketCreate(buf, BUFFER_SIZE, CMD_L_ECHO_ANSWER, cp->peer_name, data, cp->data_length);

                cp->cmd = CMD_L_ECHO_ANSWER;
                cp->header_checksum = get_byte_checksum(cp, sizeof(teoLNullCPacket) - sizeof(cp->header_checksum));
                trudpChannelSendData(tcd, cp, data_length);
            }
            // Send other commands to L0 event loop
            else send_l0_event_udp(tcd, EV_L_RECEIVED, cp, sizeof(teoLNullCPacket) + 
                    cp->data_length + cp->peer_name_length, NULL);


//            if(!o.show_statistic && !o.show_send_queue && !o.show_snake) {
//                if(o.debug) {
//                    printf("#%u at %.3f, cannel %s [%.3f(%.3f) ms] ",
//                           tcd->receiveExpectedId,
//                           (double)trudpGetTimestamp() / 1000.0,
//                           key,
//                           (double)tcd->triptime / 1000.0,
//                           (double)tcd->triptimeMiddle / 1000.0);
//
//                    printf("%s\n",(char*)data);
//                }
//            }
//            else {
//                // Show statistic window
//                //showStatistic(TD(tcd));
//            }
//            debug(tru, DEBUG,  "\n");

        } break;

        // Process received data
        // @param tcd Pointer to trudpData
        // @param data Pointer to receive buffer
        // @param data_length Receive buffer length
        // @param user_data NULL
        case PROCESS_RECEIVE: {

            trudpData *td = (trudpData *)tcd;
            trudpProcessReceived(td, data, data_length);

        } break;

        // Process send data
        // @param data Pointer to send data
        // @param data_length Length of send
        // @param user_data NULL
        case PROCESS_SEND: {

            //if(isWritable(TD(tcd)->fd, timeout) > 0) {
            // Send to UDP
            trudpUdpSendto(TD(tcd)->fd, data, data_length,
                    (__CONST_SOCKADDR_ARG) &tcd->remaddr, sizeof(tcd->remaddr));
            //}

            // Debug message
            if(/*o.debug*/ 1) {

                int port,type;
                uint32_t id = trudpPacketGetId(data);
                char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
                if(!(type = trudpPacketGetType(data))) {
                    teoLNullCPacket *cp = trudpPacketGetData(data);
                    debug(tru, DEBUG, "send %d bytes, id=%u, to %s:%d, %.3f(%.3f) ms, peer: %s, cmd: %d, data: %s\n",
                        (int)data_length,
                        id,
                        addr,
                        port,
                        tcd->triptime / 1000.0,
                        tcd->triptimeMiddle / 1000.0,
                        cp->peer_name,
                        cp->cmd,
                        (cp->data_length) ? cp->peer_name + cp->peer_name_length : "empty data");
                }
                else {
                    debug(tru, DEBUG,  "send %d bytes %s id=%u, to %s:%d\n",
                        (int)data_length,
                        type == 1 ? "ACK" :
                        type == 2 ? "RESET" :
                        type == 3 ? "ACK to RESET" :
                        type == 4 ? "PING" : "ACK to PING"
                        , id, addr, port);
                }
            }

            #if USE_LIBEV
//            trudpSendQueueCbStart(&psd, 0);
            #endif

        } break;

        default: break;
    }
}


/**
 * The TR-UDP cat network loop with select function
 *
 * @param td Pointer to trudpData
 * @param delay Default read data timeout
 */
void
trudpNetworkSelectLoop(trudpData *td, int timeout)
{

    int rv = 1;
    fd_set rfds, wfds;
    struct timeval tv;
    uint64_t ts = teoGetTimestampFull();


//    while(rv > 0) {
    // Watch server_socket to see when it has input.
    FD_ZERO(&wfds);
    FD_ZERO(&rfds);
    FD_SET(td->fd, &rfds);


    // Process write queue
    if (trudpGetWriteQueueSize(td)) {
        FD_SET(td->fd, &wfds);
    }


    uint32_t timeout_sq = trudpGetSendQueueTimeout(td, ts);


    // Wait up to ~50 ms. */
    uint32_t t = timeout_sq < timeout ? timeout_sq : timeout;
    usecToTv(&tv, t);


    rv = select((int)td->fd + 1, &rfds, &wfds, NULL, &tv);


    if (rv == -1) {
        fprintf(stderr, "select() handle error\n");
        return;
    } else if(!rv) { // Idle or Timeout event
        // Process send queue
        if(timeout_sq != UINT32_MAX) {
            int rv = trudpProcessSendQueue(td, 0);
            printf("process send queue ... %d\n", rv);
        }
    } else {  // There is a data in fd
        // Process read fd
        if(FD_ISSET(td->fd, &rfds)) {
            struct sockaddr_in remaddr; // remote address
            socklen_t addr_len = sizeof(remaddr);
            char *buffer = malloc(BUFFER_SIZE);
            ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, BUFFER_SIZE,
                    (__SOCKADDR_ARG)&remaddr, &addr_len);

            // Process received packet
            if(recvlen > 0) {

               size_t data_length;
                trudpChannelData *tcd = trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);

                trudpChannelProcessReceivedPacket(tcd, buffer, recvlen, &data_length);

            }

            free(buffer);
        }

        // Process write fd
        if(FD_ISSET(td->fd, &wfds)) {
            // Process write queue
            while(trudpProcessWriteQueue(td));
            //trudpProcessWriteQueue(td);
        }
    }
}

/**
 * Connect (login) to peer
 *
 * @param td
 * @return
 */
trudpChannelData *
trudpLNullLogin(trudpData *td, const char *host_name)
{
    trudpChannelData *tcd = NULL;

    const size_t buf_len = teoLNullBufferSize(1, strlen(host_name) + 1);

    // Buffer
    #if defined(_WIN32) || defined(_WIN64)
    char *buf = malloc(buf_len);
    #else
    char buf[buf_len];
    #endif

    size_t pkg_length = teoLNullPacketCreateLogin(buf, buf_len, host_name);
    if(!pkg_length) return NULL;

    tcd = trudpChannelNew(td, remote_address, remote_port_i, 0);
    trudpChannelSendData(tcd, buf, pkg_length);
    fprintf(stderr, "Connecting to %s:%d:%u\n", remote_address, remote_port_i, 0);
    //connected_flag = 1;

    tcd->fd = 1;
    //send_l0_event(tcd, EV_L_CONNECTED, &tcd->fd, sizeof(tcd->fd), NULL);
    //if ((snd = teoLNullPacketSend((int)con->fd, buf, pkg_length)) >= 0) {};

    // Free buffer
    #if defined(_WIN32) || defined(_WIN64)
    free(buf);
    #endif

//    // Start connection and Send "connect" packet
//    char *connect = "Connect with TR-UDP!";
//    size_t connect_length = strlen(connect) + 1;
//    tcd = trudp_ChannelNew(td, remote_address, remote_port_i, 0);
//    trudp_ChannelSendData(tcd, connect, connect_length);
//    fprintf(stderr, "Connecting to %s:%u:%u\n", remote_address, remote_port_i, 0);
//    connected_flag = 1;

    return tcd;
}


teoLNullConnectData *
trudpLNullConnect(teoLNullEventsCb event_cb, void *user_data)
{
    int result;
    teoLNullConnectData *con = malloc(sizeof(teoLNullConnectData));
    if(con == NULL) return con;

    con->last_packet_ptr = 0;
    con->read_buffer = NULL;
    con->read_buffer_ptr = 0;
    con->read_buffer_size = 0;
    con->event_cb = event_cb;
    con->user_data = user_data;
    
    con->fd = 0;
    
    return con;
}


void
trudpLNullFree(teoLNullConnectData *con)
{
    if(con) {
        free(con);
    }
}


ssize_t
trudpLNullSendEcho(trudpChannelData *tcd, const char *peer_name, const char *msg)
{
    char buf[L0_BUFFER_SIZE];
    size_t pkg_length = teoLNullPacketCreateEcho(buf, L0_BUFFER_SIZE, peer_name, msg);
    trudpChannelSendData(tcd, buf, pkg_length);
}


trudpData *
trudp_init(struct app_parameters *ap,  teoLNullConnectData *con)
{
    // Connect to L0 TR-UDP server
    // Bind UDP port and get FD (start listening at port)

    int port = 9090; //atoi(o_local_port); // Local port
    int fd = trudpUdpBindRaw(&port, 1);
    if(fd <= 0) {
        (void)fprintf(stderr, "Can't bind UDP port ...\n");
        exit(1);
    } else {
        printf("Start listening at UDP port %d\n", port);
    }

    trudpData *td = NULL;

    if(fd > 0) {
                // Initialize TR-UDP
        remote_port_i = ap->tcp_port;
        remote_address = (char*)ap->tcp_server;
        td = trudpInit(fd, port, trudpEventCback, con);

        printf("TR-UDP port created, fd = %d\n", td->fd);
    }

    return td;
}
/**
 * Set socket or SD to non blocking mode
 *
 * @param sd Socket descriptor
 */
inline void set_nonblock(int sd) __attribute_deprecated__;
inline void set_nonblock(int sd) { teosockSetBlockingMode(sd, TEOSOCK_NON_BLOCKING_MODE); }

/**
 * Set TCP NODELAY option
 *
 * @param sd TCP socket descriptor
 *
 * @return Result of setting. Success if >= 0.
 */
int set_tcp_nodelay(int sd) __attribute_deprecated__;
inline int set_tcp_nodelay(int sd) { return teosockSetTcpNodelay(sd); }


ssize_t
l0_send_msg(void *connection, uint8_t cmd, const char *peer_name,
    void *data, size_t data_length, PROTOCOL proto)
{
    if (proto == TCP) {
        teoLNullConnectData *con = (teoLNullConnectData *)connection; 
        return teoLNullSend(con, cmd, peer_name, data, data_length);
    } else if (proto == TR_UDP) {
        char buf[BUFFER_SIZE];
        size_t pkg_length = teoLNullPacketCreate(buf, BUFFER_SIZE, cmd, peer_name, data, data_length);
        trudpChannelData *con = (trudpChannelData *)connection;

        return trudpChannelSendData(con, buf, pkg_length);
    }

    return -1;
}

ssize_t
l0_send_echo(void *connection, const char *peer_name, const char *msg, PROTOCOL proto)
{
    if (proto == TCP) {
        teoLNullConnectData *con = (teoLNullConnectData *)connection;
        return teoLNullSendEcho(con, peer_name, msg);
    } else if (proto == TR_UDP) {
        trudpChannelData *con = (trudpChannelData *)connection;
        return trudpLNullSendEcho(con, peer_name, msg);
    }

    return -1;
}

/*******************
 * TRUDP INTERFACE *
 *******************/

#define PREPARE_IMPL_TRUDP(c) \
  trudp_impl_t* impl = (trudp_impl_t*)c->impl_;

int
trudp_connection_status(connection_interface_t *ci)
{
    PREPARE_IMPL_TRUDP(ci)
    return impl->tcd->connected_f;
}

ssize_t
trudp_send_echo(struct connection_interface_t *ci, const char *peer_name,
        const char *msg)
{
    PREPARE_IMPL_TRUDP(ci)
    trudpLNullSendEcho(impl->tcd, peer_name, msg);
}

int
trudp_read_event_loop(struct connection_interface_t *ci, int timeout)
{
    PREPARE_IMPL_TRUDP(ci)
    trudpNetworkSelectLoop(impl->tcd->td, timeout);
    return 1;//just for seems interface
}
size_t
trudp_keep_connection(struct connection_interface_t *ci)
{
    PREPARE_IMPL_TRUDP(ci)
    return trudpProcessKeepConnection(impl->tcd->td);
}

void
trudp_free_connection(connection_interface_t *ci)
{
    PREPARE_IMPL_TRUDP(ci)

    teoLNullConnectData *con = (teoLNullConnectData *)(impl->td)->user_data;
    teoLNullDisconnect(con);
    trudpChannelDestroy(impl->tcd);
    trudpDestroy(impl->td);
    free(impl);
}

void 
trudp_channel_free(connection_interface_t *ci, void *params)
{
    struct app_parameters *l_params = (struct app_parameters *)params;
    PREPARE_IMPL_TRUDP(ci)
    trudpChannelDestroy(impl->tcd);
    impl->tcd = trudpLNullLogin(impl->td, l_params->host_name);
}

void
trudp_ci_init(connection_interface_t *ci, teoLNullEventsCb event_cb, void *params)
{
    ci->impl_ = malloc(sizeof(trudp_impl_t));
    trudp_impl_t *impl = (trudp_impl_t *)ci->impl_;

    struct app_parameters *l_params = (struct app_parameters *)params;
    teoLNullConnectData* con = trudpLNullConnect(event_cb, params);
    impl->td = trudp_init(l_params, con);

    impl->tcd = trudpLNullLogin(impl->td, l_params->host_name);

    ci->send_echo = &trudp_send_echo;
    ci->get_connection_status = &trudp_connection_status;
    ci->read_event_loop = &trudp_read_event_loop;
    ci->keep_connection = &trudp_keep_connection;
    ci->free_connection = &trudp_free_connection;
    ci->channel_free    = &trudp_channel_free;
}



/*****************
 * TCP INTERFACE *
 *****************/
#define PREPARE_IMPL_TCP(c) \
  tcp_impl_t* impl = (tcp_impl_t*)c->impl_;

int
tcp_connection_status(connection_interface_t *ci)
{
    PREPARE_IMPL_TCP(ci)
    return impl->con->status;
}

ssize_t
tcp_send_echo(struct connection_interface_t *ci, const char *peer_name,
        const char *msg)
{
    PREPARE_IMPL_TCP(ci)
    teoLNullSendEcho(impl->con, peer_name, msg);
}

int
tcp_read_event_loop(struct connection_interface_t *ci, int timeout)
{
    PREPARE_IMPL_TCP(ci)
    return teoLNullReadEventLoop(impl->con, timeout);
}

void
tcp_channel_free(struct connection_interface_t *ci)
{
}

size_t
tcp_keep_connection(struct connection_interface_t *ci)
{
    return 1;
}

void
tcp_free_connection(connection_interface_t *ci)
{
    PREPARE_IMPL_TCP(ci)
    teoLNullDisconnect(impl->con);
    free(impl);
}

void
tcp_ci_init(connection_interface_t *ci, teoLNullEventsCb event_cb, void *params)
{
    ci->impl_ = malloc(sizeof(tcp_impl_t));
    tcp_impl_t *impl = (tcp_impl_t *)ci->impl_;
    struct app_parameters *l_params = (struct app_parameters *)params;
    impl->con = teoLNullConnectE(l_params->tcp_server, l_params->tcp_port,
                    event_cb, params);

    ci->send_echo = &tcp_send_echo;
    ci->get_connection_status = &tcp_connection_status;
    ci->read_event_loop = &tcp_read_event_loop;
    ci->keep_connection = &tcp_keep_connection;
    ci->free_connection = &tcp_free_connection;
    ci->channel_free    = &tcp_channel_free;
}


void
ci_init(connection_interface_t *ci, teoLNullEventsCb event_cb, void *params)
{
    struct app_parameters *l_params = (struct app_parameters *)params;
    if (l_params->proto == TCP) {
        tcp_ci_init(ci, event_cb, params);
    } else if (l_params->proto == TR_UDP) {
        trudp_ci_init(ci, event_cb, params);
    } else {
        fprintf(stderr, "%s", "Undefined protocol!\n"); 
    }
}

#undef DEBUG_MSG
