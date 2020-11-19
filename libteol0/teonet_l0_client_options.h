#pragma once

#ifndef TEONET_L0_CLIENT_OPTIONS_H
#define TEONET_L0_CLIENT_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "teocli_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global options and controls

/**
 * Enable extra debug logs about packet processing stages
 *
 * @param enable - boolean, if true - enables dozens of debugging messages
 */
TEOCLI_API void teoLNUllSetOption_DBG_packetFlow(bool enable);

/**
 * Enable extra debug logs in packet processing loop.
 *
 * @param enable - boolean, if true - enables dozens of debugging messages
 * in select loop.
 */
TEOCLI_API void teoLNUllSetOption_DBG_selectLoop(bool enable);

/**
 * Enable logging of unknown errors in socket functions.
 *
 * @param enable - boolean, if true - enables logging of error codes and text
 * when unknown error occurs in socket function.
 */
TEOCLI_API void teoLNUllSetOption_DBG_logUnknownErrors(bool enable);

/**
 * Enable extra debug logs in packet sending functions.
 *
 * @param enable - boolean, if true - enables dozens of debugging messages
 * when sending packets.
 */
TEOCLI_API void teoLNUllSetOption_DBG_sentPackets(bool enable);

/**
 * Enable additional packet data checksum in teocli packets.
 *
 * @param enable - boolean, if true - additional data checksum is written
 * to reserved_2 field of teoLNullCPacket. Disabled by default.
 */
TEOCLI_API void teoLNUllSetOption_PacketDataChecksumInR2(bool enable);

/**
 * Set custom connection timeout,
 *
 * @param timeoutMs should be non-negative integer, specifying desirable timeout
 * value in milliseconds. Default connection timeout 5000ms. If @a timeoutMs is
 * zero or less then timeout set to default 5000ms instead.
 */
TEOCLI_API void teoLNUllSetOption_ConnectTimeoutMs(int32_t timeout_ms);

/**
 * Set maximum messages that can be received in one select loop.
 *
 * @param maximum_messages should be positive integer, specifying desirable
 * maximum amount of messages that can be received in one select loop
 * iteration. Default value is 1. If @a maximum_messages is zero or less
 * then amount set to default 1 instead.
 */
TEOCLI_API void teoLNUllSetOption_MaximumReceiveInSelect(int32_t maximum_messages);

/**
 * Set encryption protocol used by connections
 * by default used ENC_PROTO_ECDH_AES_128_V1
 * to disable application should explicitly set it to ENC_PROTO_DISABLED
 *
 * @param protocol one of teoLNullEncryptionProtocol values
*/
TEOCLI_API void teoLNUllSetOption_EncryptionProtocol(int protocol);

/**
 * Callback function type for @a teocliSetOption_STAT_bytesSentCallback.
 */
typedef void (*teocliDataSentCallback_t)(int bytes);

/**
 * Set callback function that get called when data sent to transport
 * layer (tcp or Trudp). Default value is NULL.
 *
 * @param callback callback function.
 */
TEOCLI_API void teocliSetOption_STAT_bytesSentCallback(
    teocliDataSentCallback_t callback);

/**
 * Callback function type for @a teocliSetOption_STAT_bytesReceivedCallback.
 */
typedef void (*teocliDataReceivedCallback_t)(int bytes);

/**
 * Set callback function that get called when data received from transport
 * layer (tcp or Trudp). Default value is NULL.
 *
 * @param callback callback function.
 */
TEOCLI_API void teocliSetOption_STAT_bytesReceivedCallback(
    teocliDataReceivedCallback_t callback);


#ifdef __cplusplus
}
#endif

#endif /* TEONET_L0_CLIENT_OPTIONS_H */
