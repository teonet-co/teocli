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

extern bool teocliOpt_DBG_sentPackets;
bool teocliOpt_DBG_sentPackets = false;

void teoLNUllSetOption_DBG_sentPackets(bool enable) {
    teocliOpt_DBG_sentPackets = enable;
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
teoLNullEncryptionProtocol teocliOpt_EncryptionProtocol = ENC_PROTO_ECDH_AES_128_V1;

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

    LTRACK("TeonetClient", "Set ConnectTimeoutMs = %ld ms",
           teocliOpt_ConnectTimeoutMs);
}
