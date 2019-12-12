#pragma once

#ifndef TEONET_L0_CLIENT_CRYPT_H
#define TEONET_L0_CLIENT_CRYPT_H

#include "libtinycrypt/tinycrypt.h"
#include <stdbool.h>
#include <stdint.h>

#include "teocli_api.h"

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

#pragma pack(push)
#pragma pack(1)
typedef struct KeyExchangePayload_Common {
    //! in ANY key exchange struct first byte must be zero
    uint8_t nul_byte;
    //! encryption protocol id, teoLNullEncryptionProtocol enum
    uint16_t protocolId;
} KeyExchangePayload_Common;
#pragma pack(pop)

/**
 * Estimate buffer size sufficient to hold key exchange payload for @a enc_proto
 *
 * @param enc_proto desired encryption protocol
 *
 * @return buffer size in bytes or zero in case of error
 */
TEOCLI_API size_t teoLNullKEXBufferSize(teoLNullEncryptionProtocol enc_proto);

/**
 * Create key exchange payload to be sent to L0 server
 * requires @a ctx to be populated recently
 *
 * @param ctx encryption context to derive KEX from
 * @param buffer Buffer to create payload in
 * @param buffer_length Buffer length
 *
 * @return Length of created payload or zero if failed
 */
TEOCLI_API size_t teoLNullKEXCreate(teoLNullEncryptionContext *ctx,
                                    uint8_t *buffer, size_t buffer_length);

/**
 * Check if buffer supposed to be KEX payload
 *
 * @param buffer bytes to check
 * @param buffer_length buffer length
 *
 * @return pointer to KeyExchangePayload_Common if succeed
 */
TEOCLI_API KeyExchangePayload_Common *
teoLNullKEXGetFromPayload(uint8_t *buffer, size_t buffer_length);

/**
 * Check if KEX payload valid and can be applied to @a ctx
 *
 * @param ctx context that must be compatible
 * @param buffer payload, must be already checked via teoLNullKEXGetFromPayload
 * @param buffer_length length of @a buffer in bytes
 *
 * @return true if valid
 */
TEOCLI_API bool teoLNullKEXValidate(teoLNullEncryptionContext *ctx,
                                    KeyExchangePayload_Common *buffer,
                                    size_t buffer_length);

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
 * Apply valid KEX payload to given context
 *
 * @param ctx recipient context
 * @param buffer previously validated KEX, MUST be compatibe with @a ctx
 * @param buffer_length @a buffer length in bytes
 *
 * @return true on success
 */
TEOCLI_API bool
teoLNullEncryptionContextApplyKEX(teoLNullEncryptionContext *ctx,
                                  KeyExchangePayload_Common *buffer,
                                  size_t buffer_length);

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
