/**
 * File:   teonet_lo_client.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on October 12, 2015, 12:32 PM
 */

#include "teobase/platform.h"

#if defined(TEONET_COMPILER_MSVC)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "teonet_l0_client.h"
#include "teonet_l0_client_crypt.h"

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(TEONET_OS_WINDOWS)
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#if defined(TEONET_OS_LINUX) || defined(TEONET_OS_MACOS) ||                    \
    defined(TEONET_OS_IOS) || defined(TEONET_OS_ANDROID)
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include "teobase/logging.h"
#include "teobase/socket.h"
#include "teobase/time.h"

#include "teoccl/memory.h"

// Uncomment next line to show debug message
#define DEBUG 0
// Application constants
#define BUFFER_SIZE 4096

#define SEND_MESSAGE_AFTER 1000000

// Global teocli options
extern bool teocliOpt_DBG_packetFlow;
extern bool teocliOpt_DBG_selectLoop;
extern bool teocliOpt_DBG_sentPackets;
extern bool teocliOpt_PacketDataChecksumInR2;
extern int32_t teocliOpt_MaximumReceiveInSelect;
extern int32_t teocliOpt_ConnectTimeoutMs;
extern teoLNullEncryptionProtocol teocliOpt_EncryptionProtocol;

// Internal functions
static ssize_t teoLNullPacketSplit(teoLNullConnectData *con, void *data,
                                   size_t data_len, ssize_t received);
static void trudpEventCback(void *tcd_pointer, int event, void *data,
                            size_t data_length, void *user_data);
static teoLNullConnectData *
_teoLNullConnectionInitiate(teoLNullConnectData *con,
                            teoLNullEncryptionProtocol enc_proto);
static void teoLNullPacketUpdateHeaderChecksum(teoLNullCPacket *packet);

#if defined(HAVE_MINGW) || defined(_WIN32)
void TEOCLI_API WinSleep(uint32_t dwMilliseconds) { Sleep(dwMilliseconds); }
#endif

typedef struct teoPipeSendData {
    size_t packet_length;
    teoLNullCPacket *packet;
    bool with_encryption;
} teoPipeSendData;

static void send_l0_event(teoLNullConnectData *con, teoLNullEvents event,
                          void *data, size_t data_length) {
    if (con->event_cb != NULL) {
        con->event_cb(con, event, data, data_length, con->user_data);
    }
}

static inline char *teoLNullPacketGetPeerName(teoLNullCPacket *packet) {
    return (char *)packet->peer_name;
}

static inline uint8_t *teoLNullPacketGetData(teoLNullCPacket *packet) {
    return (uint8_t *)(packet->peer_name + packet->peer_name_length);
}

static inline uint8_t *teoLNullPacketGetFullPayload(teoLNullCPacket *packet) {
    return (uint8_t *)(packet->peer_name);
}

static inline size_t teoLNullPacketGetFullPayloadSize(teoLNullCPacket *packet) {
    return (size_t)packet->peer_name_length + packet->data_length;
}

/**
 * Initialize L0 client library.
 *
 * Startup windows socket library.
 * Calls once per application to initialize this client library.
 */
void teoLNullInit() { teosockInit(); }

/**
 * Cleanup L0 client library.
 *
 * Cleanup windows socket library.
 * Calls once per application to cleanup this client library.
 */
void teoLNullCleanup() { teosockCleanup(); }

/**
 * Ensures @a packet checksums, encrypts packet inplace (if applicable)
 * If ctx is NULL or ctx->state != SESCRYPT_ESTABLISHED then encryption isn't
 * performed, but checksums calculated anyway
 *
 * @param ctx encryption context
 * @param with_encryption flag allowing encryption
 * @param packet LNull packet to encrypt and seal
 */
void teoLNullPacketSeal(teoLNullEncryptionContext *ctx, bool with_encryption,
                        teoLNullCPacket *packet) {
    if (!with_encryption) {
        return;
    }

    teoLNullPacketEncrypt(ctx, packet);
    teoLNullPacketUpdateChecksums(packet);
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
size_t teoLNullPacketCreate(void *buffer, size_t buffer_length, uint8_t command,
                            const char *peer, const uint8_t *data,
                            size_t data_length) {
    size_t peer_name_length = strlen(peer) + 1;

    // Check buffer length
    if (buffer_length < teoLNullBufferSize(peer_name_length, data_length)) {
        LTRACK_E("TeonetClient", "Insufficient buffer size");
        abort();
    }

    teoLNullCPacket *pkg = (teoLNullCPacket *)buffer;
    memset(buffer, 0, sizeof(teoLNullCPacket));

    pkg->cmd = command;
    pkg->data_length = (uint16_t)data_length;
    pkg->peer_name_length = (uint8_t)peer_name_length;

    char *packet_peer_name = teoLNullPacketGetPeerName(pkg);
    uint8_t *packet_data = teoLNullPacketGetData(pkg);

    memcpy(packet_peer_name, peer, pkg->peer_name_length);
    memcpy(packet_data, data, pkg->data_length);

    if (teocliOpt_PacketDataChecksumInR2) {
        uint8_t extra_checksum = get_byte_checksum(data, data_length);

        // Additional check is not performed when checksum field value is zero.
        // Value 1 is used for both 0 and 1 checksum to avoid skipping check
        // on packets with data checksum equal to zero.
        if (extra_checksum == 0) {
            ++extra_checksum;
        }

        // The problem is only reproduced with command 150.
        if (command == 150) {
            char packet_id[5];

            // Get string identifier of the command data
            // (this is specific for command 150).
            if (data_length > 12) {
                memcpy(packet_id, data + 8, 4);
                packet_id[4] = 0;
            } else {
                packet_id[0] = 0;
            }

            LTRACK_I("TeonetClient",
                     "Scheduling to send META packet %u bytes with checksum "
                     "%#04x and "
                     "packet id %s.",
                     (uint32_t)data_length, extra_checksum, packet_id);
        }

        pkg->reserved_2 = extra_checksum;
    }

    teoLNullPacketUpdateChecksums(pkg);

    return teoLNullBufferSize(pkg->peer_name_length, pkg->data_length);
}

static ssize_t _teosockSend(teoLNullConnectData *con, bool with_encryption,
                            teoLNullCPacket *packet, size_t length) {
    if (con->tcp_f) {
        // for TCP connection packet sent immediately, so we should seal it right now
        teoLNullPacketSeal(con->client_crypt, with_encryption, packet);
        return teosockSend(con->fd, (const char *)packet, length);
    } else {
        teoPipeSendData pipe_send_data;
        memset(&pipe_send_data, 0, sizeof(pipe_send_data));

        // for UDP connection packet will be sent later, and we should seal it
        // just before sending to network
        pipe_send_data.with_encryption = with_encryption;
        pipe_send_data.packet_length = length;
        pipe_send_data.packet = (teoLNullCPacket *)ccl_malloc(length);
        memcpy(pipe_send_data.packet, packet, length);

// Write to pipe
#if defined(_WIN32)
        ssize_t write_result =
            _write(con->pipefd[1], &pipe_send_data, sizeof(pipe_send_data));
        SetEvent(con->handles[1]);
#else
        ssize_t write_result =
            write(con->pipefd[1], &pipe_send_data, sizeof(pipe_send_data));
#endif

        if (write_result == -1) {
            LTRACK_E("TeonetClient",
                     "Failed to write message to the pipe: write error.");
            abort();
        }

        if ((size_t)write_result != sizeof(pipe_send_data)) {
            LTRACK_E("TeonetClient", "Failed to write message to the pipe: "
                                     "message written partially.");
            abort();
        }

        return length;
    }
}

/**
 * Send packet to L0 server/client
 *
 * @param con Pointer to teoLNullConnectData
 * @param data Package to send
 * @param data_length Package length
 *
 * @return Length of send data or -1 at error
 */
ssize_t teoLNullPacketSend(teoLNullConnectData *con, bool with_encryption,
                           teoLNullCPacket *packet, size_t packet_length) {
    if (con != NULL) {
        return _teosockSend(con, with_encryption, packet, packet_length);
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
ssize_t teoLNullSend(teoLNullConnectData *con, uint8_t cmd,
                     const char *peer_name, const void *data,
                     size_t data_length) {
    CLTRACK(teocliOpt_DBG_sentPackets, "TeonetClient",
            "Sending reliable data %u bytes.", (uint32_t)data_length);

    if (data == NULL) { data_length = 0; }

    const size_t peer_length = strlen(peer_name) + 1;
    const size_t buf_length = teoLNullBufferSize(peer_length, data_length);
    teoLNullCPacket *buf = (teoLNullCPacket *)ccl_malloc(buf_length);

    size_t pkg_length = teoLNullPacketCreate(buf, buf_length,
                                             cmd, peer_name, data, data_length);
    ssize_t snd = _teosockSend(con, true, buf, pkg_length);

    free(buf);

    return snd;
}

ssize_t teoLNullSendUnreliable(teoLNullConnectData *con, uint8_t cmd,
                               const char *peer_name, const void *data,
                               size_t data_length) {
    CLTRACK(teocliOpt_DBG_sentPackets, "TeonetClient",
            "Sending unreliable data %u bytes.", (uint32_t)data_length);

    if (data == NULL) { data_length = 0; }

    const size_t peer_length = strlen(peer_name) + 1;
    const size_t buf_length = teoLNullBufferSize(peer_length, data_length);
    teoLNullCPacket *buf = (teoLNullCPacket *)ccl_malloc(buf_length);

    size_t pkg_length = teoLNullPacketCreate(buf, buf_length, cmd,
                                             peer_name,
                                             data, data_length);

    // Unreliable packets couldn't be encrypted/decrypted due to it's
    // unreliability - we can't correctly count them and seed encryption algo
    // with identifier
    ssize_t snd = 0;
    if (con->tcp_f) {
        snd = _teosockSend(con, false, buf, pkg_length);
    } else {
        snd = trudpUdpSendto(con->td->fd, buf, pkg_length,
                             (__CONST_SOCKADDR_ARG)&con->tcd->remaddr,
                             sizeof(con->tcd->remaddr));
    }
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
size_t teoLNullPacketCreateEcho(void *buf, size_t buf_len,
                                const char *peer_name, const char *msg) {
    int64_t current_time_ms = teotimeGetCurrentTimeMs();

    const unsigned int time_length = sizeof(current_time_ms);

    const size_t msg_len = strlen(msg) + 1;
    const size_t msg_buf_len = msg_len + time_length;
    void *msg_buf = ccl_malloc(msg_buf_len);

    // Fill message buffer
    memcpy(msg_buf, msg, msg_len);
    memcpy((char *)msg_buf + msg_len, &current_time_ms, time_length);
    size_t package_len = teoLNullPacketCreate(buf, buf_len, CMD_L_ECHO,
                                              peer_name, msg_buf, msg_buf_len);

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
    size_t pkg_length =
        teoLNullPacketCreateEcho(buf, L0_BUFFER_SIZE, peer_name, msg);

    // Send message with time
    ssize_t snd = _teosockSend(con, false, (teoLNullCPacket *)buf, pkg_length);

    return snd;
}

/**
 * Process ECHO_ANSWER request.(Get time from answers data and calculate trip
 * time)
 *
 * @param msg Echo answers command data
 * @return Trip time in ms
 */
int64_t teoLNullProccessEchoAnswer(const char *msg) {
    size_t time_ptr = strlen(msg) + 1;

    const int64_t *time_pointer = (const int64_t *)(msg + time_ptr);
    int64_t time_value = *time_pointer;

    int64_t trip_time_ms = teotimeGetTimePassedMs(time_value);

    return trip_time_ms;
}

/**
 * Calculate and set teoLNullCPacket header checksum.
 *
 * @param packet Pointer to packet
 */
static void teoLNullPacketUpdateHeaderChecksum(teoLNullCPacket* packet) {
    uint8_t* packet_buffer = (uint8_t*)packet;
    size_t header_size_without_checksum =
        sizeof(teoLNullCPacket) - sizeof(packet->header_checksum);

    packet->header_checksum =
        get_byte_checksum(packet_buffer, header_size_without_checksum);
}

/**
 * Calculate and set teoLNullCPacket checksums.
 *
 * @param packet Pointer to packet
 */
void teoLNullPacketUpdateChecksums(teoLNullCPacket *packet) {
    uint8_t *packet_full_pauload = teoLNullPacketGetFullPayload(packet);
    size_t packet_payload_size = teoLNullPacketGetFullPayloadSize(packet);

    packet->checksum =
        get_byte_checksum(packet_full_pauload, packet_payload_size);

    teoLNullPacketUpdateHeaderChecksum(packet);
}

/**
 * Check teoLNullCPacket checksums.
 *
 * @param packet Pointer to packet
 *
 * @return true if packet checksums are valid or false otherwise.
 */
static bool teoLNullPacketChecksumCheck(teoLNullCPacket *packet) {
    uint8_t *packet_buffer = (uint8_t *)packet;
    size_t header_size_without_checksum =
        sizeof(teoLNullCPacket) - sizeof(packet->header_checksum);

    uint8_t header_checksum =
        get_byte_checksum(packet_buffer, header_size_without_checksum);

    if (packet->header_checksum != header_checksum) { return false; }

    uint8_t *packet_full_pauload = teoLNullPacketGetFullPayload(packet);
    size_t packet_payload_size = teoLNullPacketGetFullPayloadSize(packet);

    uint8_t checksum =
        get_byte_checksum(packet_full_pauload, packet_payload_size);

    if (packet->checksum != checksum) { return false; }

    return true;
}

static inline bool _applyKEXAnswer(teoLNullConnectData *con,
                                   KeyExchangePayload_Common *kex,
                                   size_t kex_length) {
    const bool was_connected = (con->status == CON_STATUS_CONNECTED);
    if (con->client_crypt == NULL ||
        con->client_crypt->enc_proto == ENC_PROTO_DISABLED) {
        LTRACK_E("TeonetClient", "Unexpected KEX answer");
        return false;
    }

    // check if can be applied
    if (!teoLNullKEXValidate(con->client_crypt, kex, kex_length)) {
        LTRACK_E("TeonetClient",
                 "Invalid KEX, failed validation; disconnecting");
        return false;
    }

    // check if applied successfully
    if (!teoLNullEncryptionContextApplyKEX(con->client_crypt, kex,
                                           kex_length)) {
        LTRACK_E("TeonetClient", "Invalid KEX, can't apply; disconnecting");
        return false;
    }

    if (was_connected) {
        LTRACK("TeonetClient", "Re-confirmed KEX answer");
    } else {
        LTRACK_I("TeonetClient", "Applied KEX answer");
    }
    return true;
}

/**
 * Checks and applies key exchange packet from server
 * Disconnects in case of errors
 *
 * @return on success returns true, otherwise disconnects and returns false
 */
static inline bool _teoLNullProccessKEXAnswer(teoLNullConnectData *con,
                                              KeyExchangePayload_Common *kex, size_t kex_length) {
    // const bool was_established = con->client_crypt->state ==
    // SESCRYPT_ESTABLISHED;
    const bool was_connected = (con->status == CON_STATUS_CONNECTED);
    if (_applyKEXAnswer(con, kex, kex_length)) {
        if (!was_connected) {
            con->status = CON_STATUS_CONNECTED;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
        }
        return true;
    }

    con->status = CON_STATUS_ENCRYPTION_ERROR;

    if (was_connected) {
        send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
    } else {
        send_l0_event(con, EV_L_CONNECTED, &con->status, sizeof(con->status));
    }

    return false;
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
static ssize_t teoLNullPacketSplit(teoLNullConnectData *kld, void *data,
                                   size_t data_len, ssize_t received) {
    ssize_t retval = -1;

    CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
            "L0 Client: Got %" PRId32 " bytes of packet...\n", (int)received);

    // Check end of previous buffer
    if (kld->last_packet_offset > 0) {

        kld->read_buffer_offset =
            kld->read_buffer_offset - kld->last_packet_offset;

        if (kld->read_buffer_offset > 0) {

            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "L0 Client: Use %" PRId32
                    " bytes from previously received data...\n",
                    (int)(kld->read_buffer_offset));

            memmove(kld->read_buffer,
                    (char *)kld->read_buffer + kld->last_packet_offset,
                    kld->read_buffer_offset);
        }

        kld->last_packet_offset = 0;
    }

    // Increase buffer size
    if ((size_t)received > kld->read_buffer_size - kld->read_buffer_offset) {

        kld->read_buffer_size += data_len;
        if (kld->read_buffer != NULL) {
            kld->read_buffer = ccl_realloc(kld->read_buffer, kld->read_buffer_size);
        } else {
            kld->read_buffer = ccl_malloc(kld->read_buffer_size);
        }

        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "L0 Client: Increase read buffer to new size: %" PRId32
                " bytes ...\n",
                (int)kld->read_buffer_size);
    }

    // Add received data to the read buffer
    if (received > 0) {
        memmove((char *)kld->read_buffer + kld->read_buffer_offset, data,
                received);
        kld->read_buffer_offset += received;
    }

    teoLNullCPacket *packet = (teoLNullCPacket *)kld->read_buffer;
    ssize_t len;

    // \todo Check packet

    // Process read buffer
    if (kld->read_buffer_offset > sizeof(teoLNullCPacket) &&
        kld->read_buffer_offset >=
            (size_t)(len = teoLNullBufferSize(packet->peer_name_length,
                                              packet->data_length))) {

        if (teoLNullPacketChecksumCheck(packet)) {
            // Packet has received - return packet size
            retval = len;
            kld->last_packet_offset += len;

            teoLNullPacketDecrypt(kld->client_crypt, packet);

            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "L0 Server: Identify packet %" PRId32 " bytes length ...\n",
                    (int)retval);

        } else { // Wrong checksum, wrong packet - drop this packet and return
                 // -2
            kld->read_buffer_offset = 0;
            kld->last_packet_offset = 0;
            retval = -2;

            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "L0 Client: Wrong packet %" PRId32
                    " bytes length; dropped ...\n",
                    (int)len);
        }
    } else {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "L0 Client: Wait next part of packet, now it has %" PRId32
                " bytes ...\n",
                (int)kld->read_buffer_offset);
    }

    return retval;
}

/**
 * Check that buffer is valid teoLNullCPacket.
 *
 * @param data Received data buffer
 * @param data_len Received data buffer length
 *
 * @return true if packet is valid or false otherwise.
 */
static inline bool teoLNullPacketCheck(uint8_t *data, size_t data_len) {
    if (data_len < sizeof(teoLNullCPacket)) { return false; }

    teoLNullCPacket *packet = (teoLNullCPacket *)data;
    size_t len =
        teoLNullBufferSize(packet->peer_name_length, packet->data_length);

    if (data_len != len) { return false; }

    return teoLNullPacketChecksumCheck(packet);
}

/**
 * Check and cast buffer to teoLNullCPacket.
 *
 * @param data Received data buffer
 * @param data_len Received data buffer length
 *
 * @return casted pointer to teoLNullCPacket if buffer contains valid packet and
 * NULL otherwise
 */
teoLNullCPacket *teoLNullPacketGetFromBuffer(uint8_t *data, size_t data_len) {
    if (teoLNullPacketCheck(data, data_len)) {
        return (teoLNullCPacket *)data;
    }
    return NULL;
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
    if (rc != 0) { rc = teoLNullRecvCheck(con, buf, rc); }

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
ssize_t teoLNullRecvCheck(teoLNullConnectData *con, char *buf, ssize_t rc) {
    rc = teoLNullPacketSplit(con, buf, L0_BUFFER_SIZE, rc != -1 ? rc : 0);
    if (rc <= 0) {
        return rc; // No packet to check
    }

    teoLNullCPacket *cp = (teoLNullCPacket *)con->read_buffer;
    if (cp->cmd == CMD_L_INIT) {
        KeyExchangePayload_Common *kex =
            (KeyExchangePayload_Common *)teoLNullPacketGetPayload(cp);
        size_t kex_length = cp->data_length;
        if (_teoLNullProccessKEXAnswer(con, kex, kex_length)) {
            return -2; // Skip current packet
        }
        return -2; // Skip current packet
        // return 0; // Disconnect
    }

    if (cp->cmd == CMD_L_ECHO && con->fd) {
        // Send echo answer to echo command
        char *data = cp->peer_name + cp->peer_name_length;
        teoLNullSend(con, CMD_L_ECHO_ANSWER, cp->peer_name, data,
                        cp->data_length);
        return -1; // break current iteration
    }
    return rc;  // Pass as-is
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
    while ((rc = teoLNullRecv(con)) == -1 && t < timeout) {
        teoLNullSleep(wait);
        t += wait;
    }

    return rc;
}

/**
 * Create L0 clients initialization packet
 *
 * @param ctx Encryption context
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 * @param host_name Name of this L0 client
 *
 * @return Length of created packet or zero if buffer too short
 */
size_t teoLNullPacketCreateLogin(void *buffer,
                                 size_t buffer_length, const char *host_name) {
    return teoLNullPacketCreate(buffer, buffer_length, 0, "",
                                (const uint8_t*)host_name, strlen(host_name) + 1);
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
ssize_t teoLNullLogin(teoLNullConnectData *con, const char *host_name) {
    const size_t buf_len = teoLNullBufferSize(1, strlen(host_name) + 1);
    teoLNullCPacket *buf = (teoLNullCPacket *)ccl_malloc(buf_len);

    size_t pkg_length = teoLNullPacketCreateLogin(buf, buf_len, host_name);
    ssize_t snd = _teosockSend(con, true, buf, pkg_length);

    free(buf);
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
uint8_t get_byte_checksum(const uint8_t *data, size_t data_length) {
    const uint8_t *position;
    uint8_t checksum = 0;

    for (size_t i = 0; i < data_length; ++i) {
        position = data + i;
        checksum += *position;
    }

    return checksum;
}

/**
 * Get L0 packet payload pointer
 *
 * @param packet Pointer to teoLNullCPacket
 *
 * @return Pointer to payload bytes
 */
uint8_t *teoLNullPacketGetPayload(teoLNullCPacket *packet) {
    return (uint8_t *)(packet->peer_name) + packet->peer_name_length;
}

#if defined(_WIN32)
#define SELECT_RESULT_TIMEOUT WAIT_TIMEOUT
#define SELECT_RESULT_ERROR WAIT_FAILED
#else
#define SELECT_RESULT_TIMEOUT 0
#define SELECT_RESULT_ERROR -1
#endif

/**
 * The TR-UDP cat network loop with select function
 *
 * @param td Pointer to trudpData
 * @param delay Default read data timeout
 */
static teosockSelectResult trudpNetworkSelectLoop(teoLNullConnectData *con,
                                                  int timeout) {
    trudpData *td = con->td;

    teosockSelectResult retval;

    CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient", "Entered select loop.");

#if !defined(_WIN32)
    // Watch server_socket to see when it has input.
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(td->fd, &rfds);
    FD_SET(con->pipefd[0], &rfds);
#endif

    uint64_t ts = teoGetTimestampFull();
    uint32_t timeout_sq = trudpGetSendQueueTimeout(td, ts);

    // Wait up to ~50 ms. */
    uint32_t t = timeout_sq < timeout ? timeout_sq : timeout;

#if defined(_WIN32)
    DWORD select_result =
        WaitForMultipleObjects(2, con->handles, FALSE, t / 1000);
#else
    int nfds = td->fd > con->pipefd[0] ? td->fd : con->pipefd[0];

    struct timeval tv;
    usecToTv(&tv, t);

    int select_result = select(nfds + 1, &rfds, NULL, NULL, &tv);
#endif

    // Error
    if (select_result == SELECT_RESULT_ERROR) {
#if defined(_WIN32)
        LTRACK_E("TeonetClient", "WaitForMultipleObjects() returned error.");
#else
        int select_errno = errno;

        if (select_errno == EINTR) {
            CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                    "Exiting select loop by interrupt signal.");
        } else {
            const size_t buffer_size = 512;
            char error_text_buffer[buffer_size];

            int strerror_result =
                strerror_r(select_errno, error_text_buffer, buffer_size);

            if (strerror_result == 0) {
                LTRACK_E("TeonetClient", "Select failed with error: %s (%d).",
                         error_text_buffer, select_errno);
            } else {
                LTRACK_E("TeonetClient", "Select failed with unknown error (%d).",
                         select_errno);
            }
        }
#endif
        retval = TEOSOCK_SELECT_ERROR;
    } else if (select_result ==
               SELECT_RESULT_TIMEOUT) { // Idle or Timeout event
        // Process send queue

        // \TODO: need information
        retval = TEOSOCK_SELECT_TIMEOUT;

        CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                "Exiting select by timeout.");
    } else { // There is a data in fd
             // Process read fd
#if defined(_WIN32)
        if (select_result == WAIT_OBJECT_0) {
#else
        if (FD_ISSET(td->fd, &rfds)) {
#endif

            char buffer[BUFFER_SIZE];
            struct sockaddr_in remaddr; // remote address
            socklen_t addr_len = sizeof(remaddr);

            for (int receive_counter = 0;
                 receive_counter < teocliOpt_MaximumReceiveInSelect;
                 ++receive_counter) {
                ssize_t recvlen =
                    trudpUdpRecvfrom(td->fd, buffer, BUFFER_SIZE,
                                     (__SOCKADDR_ARG)&remaddr, &addr_len);
                // Process received packet
                if (recvlen > 0) {
                    CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                            "Received %u bytes from socket.",
                            (uint32_t)recvlen);

                    size_t data_length;
                    trudpChannelData *tcd =
                        trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);
                    trudpChannelProcessReceivedPacket(tcd, buffer, recvlen,
                                                      &data_length);
                } else {
                    if (recvlen == -1) {
#if defined(_WIN32)
                        int recv_errno = WSAGetLastError();
                        if (recv_errno != WSAEWOULDBLOCK) {
#else
                        int recv_errno = errno;
// EWOULDBLOCK may be not defined or may have same value as EAGAIN.
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
                        if (recv_errno != EAGAIN && recv_errno != EWOULDBLOCK) {
#else
                        if (recv_errno != EAGAIN) {
#endif
#endif
                            // TODO: Use thread safe error formatting function.
                            // TODO: On Windows use correct error formatting function.
                            LTRACK_E(
                                "TeonetClient",
                                "trudpUdpRecvfrom failed with error %" PRId32
                                ": %s",
                                recv_errno, strerror(recv_errno));
                        }
                    }
#if defined(_WIN32)
                    CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                            "Resetting socket receive state.");

                    WSANETWORKEVENTS network_events;
                    memset(&network_events, 0, sizeof(network_events));

                    WSAEnumNetworkEvents(con->fd, con->handles[0],
                                         &network_events);
#endif
                    // No more messages to receive. Leaving receive loop.
                    break;
                }
            }
        }
// Process Pipe (thread safe write)
#if defined(_WIN32)
        if (select_result == WAIT_OBJECT_0 + 1) {
#else
        if (FD_ISSET(con->pipefd[0], &rfds)) {
#endif
            CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                    "Checking sent data on pipe.");

            teoPipeSendData pipe_send_data;
            memset(&pipe_send_data, 0, sizeof(pipe_send_data));

#if defined(_WIN32)
            struct _stat status;
            memset(&status, 0, sizeof(status));

            int fstat_result = _fstat(con->pipefd[0], &status);

            int read_result = -1;
            if (fstat_result == 0 && status.st_size > 0) {
                read_result = _read(con->pipefd[0], &pipe_send_data,
                                    sizeof(pipe_send_data));
            }
#else
            ssize_t read_result =
                read(con->pipefd[0], &pipe_send_data, sizeof(pipe_send_data));
#endif

            if (read_result != -1 && read_result != 0) {
                if ((size_t)read_result != sizeof(pipe_send_data)) {
                    LTRACK_E("TeonetClient", "Failed to read message from the "
                                             "pipe: message read partially.");
                    abort();
                }

                CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                        "Received message from pipe.");

                teoLNullPacketSeal(con->client_crypt,
                                   pipe_send_data.with_encryption,
                                   pipe_send_data.packet);

                uint8_t *ptr = (uint8_t *)pipe_send_data.packet;
                size_t length = pipe_send_data.packet_length;
                for (;;) {
                    size_t len = length > 512 ? 512 : length;
                    trudpChannelSendData(con->tcd, ptr, len);
                    length -= len;
                    if (!length) break;
                    ptr += len;
                }
                free(pipe_send_data.packet);

#if defined(_WIN32)
                SetEvent(con->handles[1]);
#endif
            } else {
#if defined(_WIN32)
                ResetEvent(con->handles[1]);
#endif
            }
        }

        retval = TEOSOCK_SELECT_READY;
    }

    if (select_result != SELECT_RESULT_ERROR && timeout_sq != UINT32_MAX) {
        CLTRACK(DEBUG || teocliOpt_DBG_selectLoop, "TeonetClient",
                "Processing send queue.");
        int process_queue_result = trudpProcessSendQueue(td, 0);
        CLTRACK(DEBUG || teocliOpt_DBG_selectLoop, "TeonetClient",
                "Send queue processing finished with result %d.",
                process_queue_result);
    } else {
        CLTRACK(teocliOpt_DBG_selectLoop, "TeonetClient",
                "Skipping processing send queue.");
    }

    return retval;
}

/**
 * Wait socket data during timeout and call callback if data received
 *
 * @param con Pointer to teoLNullConnectData
 * @param timeout Timeout of wait socket read event in ms
 *
 * @return 0 - if disconnected or 1 other way
 */
bool teoLNullReadEventLoop(teoLNullConnectData *con, int timeout) {
    bool can_continue = true;
    int rv;

    if (con->tcp_f) {
        rv = teosockSelect(con->fd, TEOSOCK_SELECT_MODE_READ, timeout);
    } else {
        rv = trudpNetworkSelectLoop(con, timeout * 1000);
    }

    if (rv == TEOSOCK_SELECT_ERROR) {
        int error = errno;
        if (error != EINTR) {
            LTRACK_E("TeonetClient",
                   "select(fd = %" PRId32 ") handle error %" PRId32 ": %s",
                   (int)con->fd, error, strerror(error));
        }
    } else if (rv == TEOSOCK_SELECT_TIMEOUT) { // Idle or Timeout event
        send_l0_event(con, EV_L_IDLE, NULL, 0);
        if (!con->tcp_f) { trudpProcessKeepConnection(con->td); }
    } else { // There is a data in sd. We should send TCP-data to event-loop,
             // UDP-data has been send in trudp-eventloop
        if (con->tcp_f) {

            ssize_t rc;
            while ((rc = teoLNullRecv(con)) != -1) {
                if (rc > 0) {
                    send_l0_event(con, EV_L_RECEIVED, con->read_buffer, rc);
                } else if (rc == 0) {
                    LTRACK_I("TeonetClient",
                             "send_l0_event EV_L_DISCONNECTED in "
                             "teoLNullReadEventLoop with 0 data");

                    send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
                    con->status = CON_STATUS_NOT_CONNECTED;
                    can_continue = false;
                    break;
                }
            }
        }
    }

    if (!con->tcp_f) {
        if (con->udp_reset_f) {
            LTRACK_I("TeonetClient", "send_l0_event EV_L_DISCONNECTED in "
                                     "teoLNullReadEventLoop with udp reset");

            send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
            con->status = CON_STATUS_NOT_CONNECTED;
            can_continue = false;
            con->udp_reset_f = 0;

            // teoLNullDisconnect(con);  // This crashes app because we still
            // need con after this function. We probably have to set
            // con->tcd->conneced_f to false instead
            con->tcd->connected_f = 0;

        } else {
            if (con->status < 0) {
                can_continue = false;
            }
        }
    }
    send_l0_event(con, EV_L_TICK, NULL, 0);

    return can_continue;
}

static inline bool
_setupEncryptionContext(teoLNullConnectData *con,
                        teoLNullEncryptionProtocol enc_proto) {
    if (enc_proto == ENC_PROTO_DISABLED) {
        // TODO: Check if allowed
        return true;
    }

    size_t crypt_size = teoLNullEncryptionContextSize(enc_proto);
    if (crypt_size == 0) { return false; }

    con->client_crypt = (teoLNullEncryptionContext *)ccl_malloc(crypt_size);
    size_t result = teoLNullEncryptionContextCreate(
        enc_proto, (uint8_t *)con->client_crypt, crypt_size);
    if (result == 0) { return false; }

    return true;
}

static inline uint8_t *_createKEXPayload(teoLNullConnectData *con,
                                         teoLNullEncryptionProtocol enc_proto,
                                         size_t *payload_len) {
    size_t kex_len = teoLNullKEXBufferSize(enc_proto);
    if (kex_len == 0) {
        //
        return NULL;
    }

    uint8_t *kex_buf = (uint8_t *)ccl_malloc(kex_len);
    size_t result = teoLNullKEXCreate(con->client_crypt, kex_buf, kex_len);
    if (result == 0) {
        free(kex_buf);
        return NULL;
    }

    (*payload_len) = result;
    return kex_buf;
}

/**
 * Performs connection handshake if required
 * TCP connection without encryption considered established instantly
 * For TRUDP connection without encryption - sends dummy empty packet and awaits
 * for ACK or DATA in response.
 * If connecion uses encryption - sends key exchange packet to server and awaits
 * for remote keys in answer, both - for TCP and UDP
 *
 * @return resulting connection
 */
static inline teoLNullConnectData *
_teoLNullConnectionInitiate(teoLNullConnectData *con,
                            teoLNullEncryptionProtocol enc_proto) {
    if (enc_proto == ENC_PROTO_DISABLED) {
        if (con->tcp_f) {
            // Plain old TCP connection ready to use
            LTRACK_I("TeonetClient", "Connection established ...");
            con->status = CON_STATUS_CONNECTED;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;

        } else {
            CLTRACK_I(teocliOpt_DBG_packetFlow, "TeonetClient",
                      "Sent dummy initial TRUDP packet ...");
            // Send empty data packet to ensure server reachable
            trudpChannelSendData(con->tcd, NULL, 0);
        }

    } else {
        // Create compatible with enc_proto encryption context
        if (!_setupEncryptionContext(con, enc_proto)) {
            const char *proto_name =
                STRING_teoLNullEncryptionProtocol(enc_proto);
            LTRACK_E("TeonetClient",
                     "Can't create encryption context for %s (%d)", proto_name,
                     (int)enc_proto);

            con->status = CON_STATUS_ENCRYPTION_ERROR;
            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        // Prepare and send key exchange packet to establish encryption

        // Create key exchange payload
        size_t kex_len = 0;
        uint8_t *kex_buf = _createKEXPayload(con, enc_proto, &kex_len);
        if (kex_buf == NULL) {
            const char *proto_name =
                STRING_teoLNullEncryptionProtocol(enc_proto);
            LTRACK_E("TeonetClient", "Cant create KEX payload for %s (%d)",
                     proto_name, (int)enc_proto);

            con->status = CON_STATUS_ENCRYPTION_ERROR;
            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        // Wrap it via teoLNullCPacket
        const size_t packet_buf_len = teoLNullBufferSize(1, kex_len);
        teoLNullCPacket *packet = (teoLNullCPacket *)ccl_malloc(packet_buf_len);

        const size_t packet_len =
            teoLNullPacketCreate(packet, packet_buf_len,
                                 CMD_L_INIT, "", kex_buf, kex_len);

        // Send packet, unencrypted
        ssize_t send_result = _teosockSend(con, false, packet, packet_len);

        free(packet);
        free(kex_buf);

        if (send_result <= 0) {
            LTRACK_E("TeonetClient", "Failed to send KEX, with result %d",
                     (int)send_result);

            con->status = CON_STATUS_ENCRYPTION_ERROR;
            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }
    }

    int64_t connect_start_time_ms = teotimeGetCurrentTimeMs();
    while (con->status == CON_STATUS_NOT_CONNECTED) {
        bool can_continue = teoLNullReadEventLoop(con, 100);
        if (!can_continue) {
            CLTRACK_I(teocliOpt_DBG_packetFlow, "TeonetClient",
                      "connection event loop stopped");
            break;
        }
        const int64_t connection_time_ms =
            teotimeGetTimePassedMs(connect_start_time_ms);
        if (connection_time_ms > teocliOpt_ConnectTimeoutMs) {

            CLTRACK_I(teocliOpt_DBG_packetFlow, "TeonetClient",
                      "connection timed out");
            con->status = CON_STATUS_CONNECTION_ERROR;
            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }
    }

    if (con->status != CON_STATUS_CONNECTED) {
        CLTRACK_I(teocliOpt_DBG_packetFlow, "TeonetClient",
                  "Connection canceled by status %s (%d)",
                  STRING_teoLNullConnectionStatus(con->status),
                  (int)con->status);
        teosockClose(con->fd);
        con->fd = -1;
        return con;
    }

    // Success !!!
    return con;
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
 * @retval teoLNullConnectData::status== 1 - Success connection
 * @retval teoLNullConnectData::status==-1 - Create socket error
 * @retval teoLNullConnectData::status==-2 - HOST NOT FOUND error
 * @retval teoLNullConnectData::status==-3 - Client-connect() error
 * @retval teoLNullConnectData::status==-4 - Pipe creation error
 */
teoLNullConnectData *teoLNullConnectE(const char *server, int16_t port,
                                      teoLNullEventsCb event_cb,
                                      void *user_data,
                                      PROTOCOL connection_flag) {
    teoLNullConnectData *con =
        (teoLNullConnectData *)ccl_malloc(sizeof(teoLNullConnectData));
    if (con == NULL) {
        LTRACK_E("TeonetClient", "Failed to allocate teoLNullConnectData");
        abort();
    }

    con->last_packet_offset = 0;
    con->read_buffer = NULL;
    con->read_buffer_offset = 0;
    con->read_buffer_size = 0;
    con->client_crypt = NULL;
    con->event_cb = event_cb;
    con->user_data = user_data;
    con->udp_reset_f = 0;
    con->td = NULL;
    con->tcp_f = connection_flag;
    con->tcd = NULL;
    con->pipefd[0] = -1;
    con->pipefd[1] = -1;
    con->status = CON_STATUS_NOT_CONNECTED;

#if defined(_WIN32)
    con->handles[0] = NULL;
    con->handles[1] = NULL;
#endif

    // Connect to TCP
    if (con->tcp_f) {
        con->fd = teosockCreateTcp();

        if (con->fd == TEOSOCK_INVALID_SOCKET) {
            LTRACK_E("TeonetClient", "Can't create socket");
            con->fd = -1;
            con->status = CON_STATUS_SOCKET_ERROR;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Successfully created socket, connecting to %s at port %" PRIu16
                " ...",
                server, port);

        int result =
            teosockConnectTimeout(con->fd, server, port, teocliOpt_ConnectTimeoutMs);

        if (result == TEOSOCK_CONNECT_HOST_NOT_FOUND) {
            LTRACK_E("TeonetClient", "HOST NOT FOUND --> h_errno = %" PRId32,
                     h_errno);
            teosockClose(con->fd);
            con->fd = -1;
            con->status = CON_STATUS_HOST_ERROR;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        if (result == TEOSOCK_CONNECT_FAILED) {
            int error = errno;
            LTRACK_E("TeonetClient", "Client-connect() error: %" PRId32 ", %s",
                     error, strerror(error));
            teosockClose(con->fd);
            con->fd = -1;
            con->status = CON_STATUS_CONNECTION_ERROR;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        // Set TCP_NODELAY option
        teosockSetTcpNodelay(con->fd);

    } else {
        // Connect to UDP
        int port_local = 0;
        con->fd = trudpUdpBindRaw(&port_local, 1);
        if (con->fd < 0) {
            LTRACK_E("TeonetClient", "Failed to bind UDP socket.");
            con->status = CON_STATUS_SOCKET_ERROR;
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

        con->td = trudpInit(con->fd, port, trudpEventCback, con);
        con->tcd = trudpChannelNew(con->td, (char *)server, port, 0);
        LTRACK_I("TeonetClient", "TR-UDP port = %d created, fd = %d",
                 port_local, (int)con->fd);

// Pipe create
#if defined(_WIN32)
        int pipe_result = _pipe(con->pipefd, 1024 * 10, _O_BINARY);
#else
        int pipe_result = pipe(con->pipefd);
#endif
        if (pipe_result == -1) {
            con->status = CON_STATUS_PIPE_ERROR;
            con->pipefd[0] = -1;
            con->pipefd[1] = -1;
            LTRACK_E("TeonetClient",
                     "Failed to create pipe for sending commands.");

            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }

#if defined(_WIN32)
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient", "Creating events.");
        con->handles[0] = WSACreateEvent();
        con->handles[1] = CreateEventA(NULL, TRUE, FALSE, NULL);

        int event_select_result = 0;
        if (con->handles[0] != NULL && con->handles[1] != NULL) {
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Binding socket to event.");
            event_select_result =
                WSAEventSelect(con->fd, con->handles[0], FD_READ | FD_CLOSE);
            if (event_select_result != 0) {
                int error_code = WSAGetLastError();
                LTRACK_E("TeonetClient", "Failed to bind event, error code %d.",
                         error_code);

                if (error_code == WSAENETDOWN) {
                    LTRACK_E("TeonetClient", "Error: WSAENETDOWN.");
                } else if (error_code == WSAEINVAL) {
                    LTRACK_E("TeonetClient", "Error: WSAEINVAL.");
                } else if (error_code == WSAEINPROGRESS) {
                    LTRACK_E("TeonetClient", "Error: WSAEINPROGRESS.");
                } else if (error_code == WSAENOTSOCK) {
                    LTRACK_E("TeonetClient", "Error: WSAENOTSOCK.");
                } else {
                    LTRACK_E("TeonetClient", "Error: unknown.");
                }
            }
        }

        if (con->handles[0] == NULL || con->handles[1] == NULL ||
            event_select_result != 0) {
            LTRACK_E("TeonetClient",
                     "Failed to create events for sending commands.");

            if (con->handles[0] != NULL) {
                CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                        "Closing write handle.");
                WSACloseEvent(con->handles[0]);
                con->handles[0] = NULL;
            }

            if (con->handles[1] != NULL) {
                CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                        "Closing read handle.");
                CloseHandle(con->handles[1]);
                con->handles[1] = NULL;
            }

            _close(con->pipefd[0]);
            _close(con->pipefd[1]);

            con->pipefd[0] = -1;
            con->pipefd[1] = -1;

            con->status = CON_STATUS_PIPE_ERROR;
            teosockClose(con->fd);
            con->fd = -1;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
            return con;
        }
#endif

        con->status = CON_STATUS_NOT_CONNECTED;
    }

    return _teoLNullConnectionInitiate(con, teocliOpt_EncryptionProtocol);
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
teoLNullConnectData *teoLNullConnect(const char *server, int16_t port,
                                     PROTOCOL connection_flag) {
    return teoLNullConnectE(server, port, NULL, NULL, connection_flag);
}

/**
 * Disconnect from server and free teoLNullConnectData
 *
 * @param con Pointer to teoLNullConnectData
 */
void teoLNullDisconnect(teoLNullConnectData *con) {
    if (con != NULL) {
        if (con->fd > 0) { teosockClose(con->fd); }

        if (con->read_buffer != NULL) { free(con->read_buffer); }

        if (con->client_crypt != NULL) { free(con->client_crypt); }

        if (!con->tcp_f) {

            trudpChannelDestroyAll(con->td);
            trudpDestroy(con->td);
        }

        if (con->pipefd[0] != -1) {
#if defined(_WIN32)
            _close(con->pipefd[0]);
#else
            close(con->pipefd[0]);
#endif
        }

        if (con->pipefd[1] != -1) {
#if defined(_WIN32)
            _close(con->pipefd[1]);
#else
            close(con->pipefd[1]);
#endif
        }

#if defined(_WIN32)
        if (con->handles[0] != NULL) {
            WSACloseEvent(con->handles[0]);
            con->handles[0] = NULL;
        }

        if (con->handles[1] != NULL) {
            CloseHandle(con->handles[1]);
            con->handles[1] = NULL;
        }
#endif

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
        if (con->tcp_f) {
            teosockShutdown(con->fd, TEOSOCK_SHUTDOWN_RDWR);
        } else {
            con->udp_reset_f = 1;
        }
    }
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
static void trudpEventCback(void *tcd_pointer, int event, void *data,
                            size_t data_length, void *user_data) {
    trudpChannelData *tcd = (trudpChannelData *)tcd_pointer;
    CLTRACK(DEBUG, "TeonetClient", "chan %s event %s data_length %u",
            tcd->channel_key, STRING_trudpEvent(event), (uint32_t)data_length);

    switch (event) {

    // CONNECTED event
    // @param data NULL
    // @param user_data NULL
    case CONNECTED: {
        CLTRACK(DEBUG, "TeonetClient", "Connect channel %s\n", tcd->channel_key);
        // tcd->connected_f = 1; would be set in trudpGetChannelCreate anyway
        teoLNullConnectData *con = (teoLNullConnectData *)user_data;
        if (con->client_crypt &&
            con->client_crypt->enc_proto != ENC_PROTO_DISABLED) {
            // Must wait for KEX answer
            // event would be sent in KEX handler

        } else if (!tcd->connected_f) {
            con->status = CON_STATUS_CONNECTED;
            log_info("TeonetClient",
                     "send_l0_event EV_L_CONNECTED in trudpEventCback on "
                     "CONNECTED(for unconnected)");
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
        }
    } break;

    // DISCONNECTED event
    // @param tcd Pointer to trudpData
    // @param data Last packet received
    // @param user_data NULL
    case DISCONNECTED: {
        teoLNullConnectData *con = (teoLNullConnectData*)user_data;

        // Disconnect notification with interval elapsed causes
        // all channels of the same connection to be destroyed,
        // which in turn emits disconnect notifications for each of them
        // to avoid infinite recursion we do not destroy DISCONNECTED event
        // without time in data
        if (data_length == sizeof(uint32_t) && data) {
            uint32_t last_received = *(uint32_t *)data;
            CLTRACK(DEBUG, "TeonetClient",
                    "Disconnect channel %s, last received: %.6f sec",
                    tcd->channel_key, last_received / 1000000.0);

            LTRACK_I("TeonetClient",
                     "send_l0_event EV_L_DISCONNECTED on DISCONNECTED");

            send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
            tcd->connected_f = 0;
            con->status = CON_STATUS_NOT_CONNECTED;
            trudpChannelDestroyAll(con->td);
        } else {
            CLTRACK(DEBUG, "TeonetClient",
                    "Disconnected channel %s, data=%p, data_len=%u\n",
                    tcd->channel_key, data, (uint32_t)data_length);
        }

        tcd->connected_f = 0;
    } break;

    // GOT_RESET event
    // @param data NULL
    // @param user_data NULL
    case GOT_RESET: {
        CLTRACK(DEBUG, "TeonetClient", "got TRU_RESET packet from channel %s",
                tcd->channel_key);
        teoLNullConnectData *con = (teoLNullConnectData*)user_data;
        if (tcd->connected_f) {
            LTRACK_I("TeonetClient",
                     "send_l0_event EV_L_DISCONNECTED on GOT_RESET");

            send_l0_event(con, EV_L_DISCONNECTED, NULL, 0);
            con->status = CON_STATUS_NOT_CONNECTED;
            tcd->connected_f = 0;
        }
    } break;

    // SEND_RESET event
    // @param data Pointer to uint32_t id or NULL (data_size == 0)
    // @param user_data NULL
    case SEND_RESET: {
        if (!data) {
            CLTRACK(DEBUG, "TeonetClient", "Send reset: to channel %s",
                    tcd->channel_key);
        } else {
            uint32_t id =
                (data_length == sizeof(uint32_t)) ? *(uint32_t *)data : 0;

            if (id == 0) {
                LTRACK_E("TeonetClient",
                         "Send reset: Not expected packet with id = 0 "
                         "received from channel %s",
                         tcd->channel_key);
            } else {
                CLTRACK(
                    DEBUG, "TeonetClient",
                    "Send reset: High send packet number (%u) at channel %s",
                    id, tcd->channel_key);
            }
        }
    } break;

    // GOT_ACK_RESET event: got ACK to reset command
    // @param data NULL
    // @param user_data NULL
    case GOT_ACK_RESET: {
        CLTRACK(DEBUG, "TeonetClient", "Got ACK to RESET packet at channel %s",
                tcd->channel_key);
    } break;

    // GOT_ACK_PING event: got ACK to ping command
    // @param data Pointer to ping data (usually it is a string)
    // @param user_data NULL
    case GOT_ACK_PING: {
        CLTRACK(DEBUG, "TeonetClient",
                "got ACK to PING packet at channel %s, %.3f(%.3f) ms",
                tcd->channel_key, (tcd->triptime) / 1000.0,
                (tcd->triptimeMiddle) / 1000.0);
    } break;

    // GOT_PING event: got PING packet, data
    // @param data Pointer to ping data (usually it is a string)
    // @param user_data NULL
    case GOT_PING: {
        CLTRACK(DEBUG, "TeonetClient", "got PING packet at channel %s",
                tcd->channel_key);
    } break;

    // Got ACK event
    // @param data Pointer to ACK packet
    // @param data_length Length of data
    // @param user_data NULL
    case GOT_ACK: {
        teoLNullConnectData *con = (teoLNullConnectData *)user_data;
        trudpPacket * packet = (trudpPacket *)data;
        uint32_t id = trudpPacketGetId(packet);

        if (con->client_crypt &&
            con->client_crypt->enc_proto != ENC_PROTO_DISABLED) {
            // Must wait for KEX answer
            // event would be sent in KEX handler

        } else if (id == 0 && !tcd->connected_f) {
            trudpSendEvent(con->tcd, CONNECTED, NULL, 0, NULL);
            con->status = CON_STATUS_CONNECTED;
            send_l0_event(con, EV_L_CONNECTED, &con->status,
                          sizeof(con->status));
        }
        CLTRACK(DEBUG, "TeonetClient",
                "got ACK id=%u at channel %s, %.3f(%.3f) ms", id,
                tcd->channel_key, (tcd->triptime) / 1000.0,
                (tcd->triptimeMiddle) / 1000.0);
    } break;

    // Got DATA event
    // @param data Pointer to data
    // @param data_length Length of data
    // @param user_data NULL Pointer to teoLNullConnectData
    case GOT_DATA: {
        teoLNullConnectData *con = (teoLNullConnectData*)user_data;
        trudpPacket * packet = (trudpPacket *)data;

        uint32_t id = trudpPacketGetId(packet);
        size_t block_len = trudpPacketGetDataLength(packet);
        void* block = trudpPacketGetData(packet);
        ssize_t ready_bytes_count = teoLNullRecvCheck(con, block, block_len);
        if (ready_bytes_count <= 0) {
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Got block id=%u chan=%s of %u bytes", id, tcd->channel_key,
                    (uint32_t)block_len);
            break;
        }
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Got block id=%u chan=%s of %u bytes, assembled L0 packet of "
                "%d bytes",
                id, tcd->channel_key, (uint32_t)block_len,
                (int32_t)ready_bytes_count);

        teoLNullCPacket *cp = (teoLNullCPacket *)con->read_buffer;
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "trip(mid)=[%.3f(%.3f) ms] peer=%s, cmd=%d, payload=%u",
                (double)tcd->triptime / 1000.0,
                (double)tcd->triptimeMiddle / 1000.0, cp->peer_name,
                (int32_t)cp->cmd, (uint32_t)cp->data_length);

        // Process commands
        if (cp->cmd == CMD_L_ECHO) {
            cp->cmd = CMD_L_ECHO_ANSWER;
            teoLNullPacketUpdateHeaderChecksum(cp);
            trudpChannelSendData(tcd, cp, ready_bytes_count);
        } else { // Send other commands to L0 event loop
            send_l0_event(con, EV_L_RECEIVED, cp, ready_bytes_count);
        }
    } break;

    // Got DATA event
    // @param data Pointer to data
    // @param data_length Length of data
    // @param user_data NULL Pointer to teoLNullConnectData
    case GOT_DATA_NO_TRUDP: {
        teoLNullConnectData *con = (teoLNullConnectData*)user_data;

        if (!teoLNullPacketCheck(data, data_length)) {
            CLTRACK(DEBUG, "TeonetClient",
                    "got invalid non TR-UDP data packet with %u bytes of data",
                    (uint32_t)data_length);
            break;
        }

        CLTRACK(DEBUG, "TeonetClient",
                "got valid non TR-UDP data packet with %u bytes of data",
                (uint32_t)data_length);
        send_l0_event(con, EV_L_RECEIVED, data, data_length);
    }

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
        // Send to UDP
        trudpUdpSendto(TD(tcd)->fd, data, data_length,
                       (__CONST_SOCKADDR_ARG)&tcd->remaddr,
                       sizeof(tcd->remaddr));

        if (DEBUG) {
            trudpPacket* packet = (trudpPacket*)data;
            uint32_t id = trudpPacketGetId(packet);
            trudpPacketType type = trudpPacketGetType(packet);
            if (type == TRU_DATA) {
                CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                        "send %u bytes data id=%u, to %s",
                        (uint32_t)data_length, id, tcd->channel_key);

            } else {
                CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                        "send %u bytes type=%s(%d) id=%u, to %s",
                        (uint32_t)data_length, STRING_trudpPacketType(type),
                        (int)type, id, tcd->channel_key);
            }
        }
    } break;

    default: break;
    }
}

const char *STRING_teoLNullConnectionStatus(teoLNullConnectionStatus v) {
    switch (v) {
    case CON_STATUS_CONNECTED: return "CON_STATUS_CONNECTED";
    case CON_STATUS_NOT_CONNECTED: return "CON_STATUS_NOT_CONNECTED";
    case CON_STATUS_SOCKET_ERROR: return "CON_STATUS_SOCKET_ERROR";
    case CON_STATUS_HOST_ERROR: return "CON_STATUS_HOST_ERROR";
    case CON_STATUS_CONNECTION_ERROR: return "CON_STATUS_CONNECTION_ERROR";
    case CON_STATUS_PIPE_ERROR: return "CON_STATUS_PIPE_ERROR";
    case CON_STATUS_ENCRYPTION_ERROR: return "CON_STATUS_ENCRYPTION_ERROR";
    default: break;
    }

    return "INVALID teoLNullConnectionStatus";
}

const char *STRING_teoLNullEvents(teoLNullEvents v) {
    switch (v) {
    case EV_L_CONNECTED: return "EV_L_CONNECTED";
    case EV_L_DISCONNECTED: return "EV_L_DISCONNECTED";
    case EV_L_RECEIVED: return "EV_L_RECEIVED";
    case EV_L_TICK: return "EV_L_TICK";
    case EV_L_IDLE: return "EV_L_IDLE";
    default: break;
    }

    return "INVALID teoLNullEvents";
};
