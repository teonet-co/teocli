#include "teonet_l0_client_options.h"
#include "teobase/logging.h"

extern bool teocliOpt_DBG_packetFlow;
bool teocliOpt_DBG_packetFlow = false;
TEOCLI_API void teoLNUllSetOption_DBG_packetFlow(bool enable) {
    teocliOpt_DBG_packetFlow = enable;
}

extern int64_t teocliOpt_ConnectTimeoutMs;
static const int64_t DEFAULT_CONNECT_TIMEOUTMS = 5000;
int64_t teocliOpt_ConnectTimeoutMs = DEFAULT_CONNECT_TIMEOUTMS;

TEOCLI_API void teoLNUllSetOption_ConnectTimeoutMs(int64_t timeout_ms) {
    teocliOpt_ConnectTimeoutMs =
        (timeout_ms > 0) ? timeout_ms : DEFAULT_CONNECT_TIMEOUTMS;
    LTRACK("TeonetClient", "Set ConnectTimeoutMs=%ld ms",
           teocliOpt_ConnectTimeoutMs);
    return;
}