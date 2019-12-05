#pragma once

#ifndef TEONET_L0_CLIENT_OPTIONS_H
#define TEONET_L0_CLIENT_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef _WINDLL
#define TEOCLI_API __declspec(dllexport)
#else
#define TEOCLI_API
#endif

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
 * Enable extra debug logs in packet sending functions.
 *
 * @param enable - boolean, if true - enables dozens of debugging messages
 * when sending packets.
 */
TEOCLI_API void teoLNUllSetOption_DBG_sentPackets(bool enable);

/**
 * Set custom connection timeout,
 *
 * @param timeoutMs should be non-negative integer, specifying desirable timeout
 * value in milliseconds. Default connection timeout 5000ms. If @a timeoutMs is
 * zero or less then timeout set to default 5000ms instead.
 */
TEOCLI_API void teoLNUllSetOption_ConnectTimeoutMs(int64_t timeout_ms);

/**
 * Set maximum messages that can be received in one select loop.
 *
 * @param maximum_messages should be positive integer, specifying desirable
 * maximum amount of messages that can be received in one select loop
 * iteration. Default value is 1. If @a maximum_messages is zero or less
 * then timeout set to default 1 instead.
 */
TEOCLI_API void teoLNUllSetOption_MaximumReceiveInSelect(int32_t maximum_messages);

#ifdef __cplusplus
}
#endif

#endif /* TEONET_L0_CLIENT_OPTIONS_H */
