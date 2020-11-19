#include "teonet_l0_client_options.h"

#include <stdbool.h>
#include <stdint.h>

#include "teobase/logging.h"
#include "teonet_l0_client_crypt.h"

extern bool teocliOpt_DBG_packetFlow;
bool teocliOpt_DBG_packetFlow = false;

void teoLNUllSetOption_DBG_packetFlow(bool enable) {
    teocliOpt_DBG_packetFlow = enable;
}

extern bool teocliOpt_DBG_selectLoop;
bool teocliOpt_DBG_selectLoop = false;

void teoLNUllSetOption_DBG_selectLoop(bool enable) {
    teocliOpt_DBG_selectLoop = enable;
}

extern bool teocliOpt_DBG_logUnknownErrors;
bool teocliOpt_DBG_logUnknownErrors = false;

void teoLNUllSetOption_DBG_logUnknownErrors(bool enable) {
    teocliOpt_DBG_logUnknownErrors = enable;
}

extern bool teocliOpt_DBG_sentPackets;
bool teocliOpt_DBG_sentPackets = false;

void teoLNUllSetOption_DBG_sentPackets(bool enable) {
    teocliOpt_DBG_sentPackets = enable;
}

extern bool teocliOpt_PacketDataChecksumInR2;
bool teocliOpt_PacketDataChecksumInR2 = false;

void teoLNUllSetOption_PacketDataChecksumInR2(bool enable) {
    teocliOpt_PacketDataChecksumInR2 = enable;
}

extern int32_t teocliOpt_MaximumReceiveInSelect;
int32_t teocliOpt_MaximumReceiveInSelect = 1;

void teoLNUllSetOption_MaximumReceiveInSelect(int32_t maximum_messages) {
    if (maximum_messages < 1) {
        teocliOpt_MaximumReceiveInSelect = 1;
    } else {
        teocliOpt_MaximumReceiveInSelect = maximum_messages;
    }

    LTRACK("TeonetClient", "Set MaximumReceiveInSelect = %d ms",
           teocliOpt_MaximumReceiveInSelect);
}

extern teoLNullEncryptionProtocol teocliOpt_EncryptionProtocol;
teoLNullEncryptionProtocol teocliOpt_EncryptionProtocol =
    ENC_PROTO_ECDH_AES_128_V1;

void teoLNUllSetOption_EncryptionProtocol(int protocol) {
    switch (protocol) {
    case ENC_PROTO_DISABLED: // fallthrough
    case ENC_PROTO_ECDH_AES_128_V1:
        teocliOpt_EncryptionProtocol = (teoLNullEncryptionProtocol)protocol;
        break;

    default:
        teocliOpt_EncryptionProtocol =
            (teoLNullEncryptionProtocol)ENC_PROTO_ECDH_AES_128_V1;
        break;
    }
}

enum {
    DEFAULT_CONNECT_TIMEOUT_MS = 5000,
};

extern int32_t teocliOpt_ConnectTimeoutMs;
int32_t teocliOpt_ConnectTimeoutMs = DEFAULT_CONNECT_TIMEOUT_MS;

void teoLNUllSetOption_ConnectTimeoutMs(int32_t timeout_ms) {
    teocliOpt_ConnectTimeoutMs =
        (timeout_ms > 0) ? timeout_ms : DEFAULT_CONNECT_TIMEOUT_MS;

    LTRACK("TeonetClient", "Set ConnectTimeoutMs = %d ms",
           teocliOpt_ConnectTimeoutMs);
}

extern teocliDataSentCallback_t teocliOpt_STAT_dataSentCallback;
teocliDataSentCallback_t teocliOpt_STAT_dataSentCallback = NULL;

// Set callback function that get called when data sent to udp socket.
void teocliSetOption_STAT_bytesSentCallback(teocliDataSentCallback_t callback) {
    teocliOpt_STAT_dataSentCallback = callback;
}

extern teocliDataReceivedCallback_t teocliOpt_STAT_dataReceivedCallback;
teocliDataReceivedCallback_t teocliOpt_STAT_dataReceivedCallback = NULL;

// Set callback function that get called when data received from udp socket.
void teocliSetOption_STAT_bytesReceivedCallback(
    teocliDataReceivedCallback_t callback) {
    teocliOpt_STAT_dataReceivedCallback = callback;
}
