#pragma once

#ifndef TEONET_L0_CLIENT_CRYPT_H
#define TEONET_L0_CLIENT_CRYPT_H

#include "libtinycrypt/tinycrypt.h"
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

/////////////////
// teonet client-to-l0-server encryption support
/////////////////

typedef enum teoLNullEncryptionProtocol {
    //! Run without encryption
    ENC_PROTO_DISABLED = 0,
    //! First implementation protocol
    ENC_PROTO_ECDH_AES_128_V1 = 1,
} teoLNullEncryptionProtocol;

typedef enum teoLNullEncryptedSessionState {
    //! Keys sent from our side, waiting for other side reply
    SESCRYPT_PENDING,
    //! Encrypted session established
    SESCRYPT_ESTABLISHED,
} teoLNullEncryptedSessionState;

typedef struct teoLNullEncryptionContext {
    //! Stages of session handshake
    teoLNullEncryptedSessionState state;
    //! Encryption protocol variant, for future extensions
    teoLNullEncryptionProtocol enc_proto;
    //! counters for CTR-mode encryption
    uint32_t receiveNonce, sendNonce;
    //! Encryption keys holder
    PeerKeyset keys;
} teoLNullEncryptionContext;

// forward declaration, complete type in libteol0/teonet_l0_client.h
typedef struct teoLNullCPacket teoLNullCPacket;

/**
 * Estimate buffer size sufficient to hold teoLNullEncryptionContext for @a
 * enc_proto
 *
 * @param enc_proto desired encryption protocol
 *
 * @return buffer size in bytes or zero in case of error
 */
TEOCLI_API size_t
teoLNullEncryptionContextSize(teoLNullEncryptionProtocol enc_proto);

/**
 * Create teoLNullEncryptionContext for @a enc_proto
 *
 * @param enc_proto desired encryption protocol
 * @param buffer Buffer to create packet in
 * @param buffer_length Buffer length
 *
 * @return Length of created teoLNullEncryptionContext or zero if failed
 */
TEOCLI_API size_t
teoLNullEncryptionContextCreate(teoLNullEncryptionProtocol enc_proto,
                                uint8_t *buffer, size_t buffer_length);

/**
 * Encrypt packet before sending. Encrypts inplace.
 *
 * @param ctx Encryption context, determines the way data be encrypted
 *  if ctx is NULL or session weren't established yet - no encryption performed
 * @param packet L0 packet to be encrypted
 */
TEOCLI_API void teoLNullPacketEncrypt(teoLNullEncryptionContext *ctx,
                                      teoLNullCPacket *packet);

/**
 * Decrypt received packet inplace.
 *
 * @param ctx Encryption context, determines the way data be decrypted
 *  if ctx is NULL or session weren't established yet - no encryption performed
 * @param packet L0 packet to be decrypted
 *
 * @return true if success, false if error
 */
TEOCLI_API bool teoLNullPacketDecrypt(teoLNullEncryptionContext *ctx,
                                      teoLNullCPacket *packet);

/**
 * Checks if packet encrypted
 *
 * @param packet L0 packet
 *
 * @return true if packet encrypted
 */
TEOCLI_API bool teoLNullPacketIsEncrypted(teoLNullCPacket *packet);

/**
 * enum teoLNullEncryptionProtocol printer
 */
TEOCLI_API const char *
STRING_teoLNullEncryptionProtocol(teoLNullEncryptionProtocol v);
/**
 * enum teoLNullEncryptedSessionState printer
 */
TEOCLI_API const char *
STRING_teoLNullEncryptedSessionState(teoLNullEncryptedSessionState v);

#ifdef __cplusplus
}
#endif

#endif /* TEONET_L0_CLIENT_CRYPT_H */
