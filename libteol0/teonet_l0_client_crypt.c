#include "teonet_l0_client_crypt.h"
#include "teonet_l0_client.h"
#include "teobase/logging.h"
#include <assert.h>

extern int teocliOpt_DBG_packetFlow;

typedef struct KeyExchangePayload_ECDH_AES_128_V1 {
    //! common.protocolId, must be ENC_PROTO_ECDH_AES_128_V1
    KeyExchangePayload_Common common;

    // Protocol-dependent encryption parameters
    ECDHPubkey pubkey;   ///< public key
    AES128_1_BLOCK salt; ///< common salt
} KeyExchangePayload_ECDH_AES_128_V1;

size_t teoLNullKEXBufferSize(teoLNullEncryptionProtocol enc_proto) {
    static_assert(3 == sizeof(KeyExchangePayload_Common),
                  "KeyExchangePayload_Common memory layout must be 1+2 bytes");

    switch (enc_proto) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        return sizeof(KeyExchangePayload_ECDH_AES_128_V1);
    }

    default: {
        return 0;
    }
    }
}

size_t teoLNullKEXCreate(teoLNullEncryptionContext *ctx, uint8_t *buffer,
                         size_t buffer_length) {
    switch (ctx->enc_proto) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        const size_t payload_len = teoLNullKEXBufferSize(ctx->enc_proto);
        if (payload_len != buffer_length) {
            LTRACK_E("TeonetClient", "Buffer size mismatch in KEXCreate");
            abort();
        }

        KeyExchangePayload_ECDH_AES_128_V1 *kex =
            (KeyExchangePayload_ECDH_AES_128_V1 *)buffer;

        kex->common.nul_byte = 0;
        kex->common.protocolId = ctx->enc_proto;
        kex->pubkey = ctx->keys.pubkeylocal;
        kex->salt = ctx->keys.sessionsalt;
        return payload_len;
    }

    default: {
        return 0;
    }
    }
}

KeyExchangePayload_Common *teoLNullKEXGetFromPayload(uint8_t *buffer,
                                                     size_t buffer_length) {
    size_t min_len = sizeof(KeyExchangePayload_Common);
    // Check for len and initial NUL
    if (buffer_length < min_len || buffer[0] != 0) { return NULL; }

    return (KeyExchangePayload_Common *)buffer;
}

bool teoLNullKEXValidate(teoLNullEncryptionContext *ctx,
                         KeyExchangePayload_Common *buffer,
                         size_t buffer_length) {
    switch (buffer->protocolId) {
    case ENC_PROTO_DISABLED: {
        LTRACK_E("TeonetClient", "KEX_PACKET broken ENC_PROTO_DISABLED");
        return false;
    }

    case ENC_PROTO_ECDH_AES_128_V1: {
        const size_t kex_len = teoLNullKEXBufferSize(buffer->protocolId);
        if (kex_len != buffer_length) {
            LTRACK_E("TeonetClient",
                     "KEX_PACKET broken ECDH_AES_128_V1 size %u mismatch "
                     "buffer %u bytes",
                     (uint32_t)kex_len, (uint32_t)buffer_length);
            return false;
        }

        if (ctx && ctx->enc_proto != buffer->protocolId) {
            LTRACK_E("TeonetClient",
                     "KEX_PACKET broken ECDH_AES_128_V1 proto mismatch "
                     "ctx %s(%d)",
                     STRING_teoLNullEncryptionProtocol(ctx->enc_proto),
                     (int)ctx->enc_proto);
            return false;
        }
        return true;
    }

    default: {
        LTRACK_E("TeonetClient", "KEX_PACKET broken: Unknown proto %s(%d)",
                 STRING_teoLNullEncryptionProtocol(buffer->protocolId),
                 (int)buffer->protocolId);
        return false;
    }
    }
}

size_t teoLNullEncryptionContextSize(teoLNullEncryptionProtocol enc_proto) {
    switch (enc_proto) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        return sizeof(teoLNullEncryptionContext);
    }

    default: {
        return 0;
    }
    }
}

size_t teoLNullEncryptionContextCreate(teoLNullEncryptionProtocol enc_proto,
                                       uint8_t *buffer, size_t buffer_length) {
    switch (enc_proto) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        if (buffer_length != sizeof(teoLNullEncryptionContext)) {
            LTRACK_E("TeonetClient",
                     "Buffer size mismatch in EncryptioContextCreate");
            abort();
        }

        teoLNullEncryptionContext *ctx = (teoLNullEncryptionContext *)buffer;
        ctx->enc_proto = enc_proto;
        ctx->receiveNonce = 1;
        ctx->sendNonce = 1;
        initPeerKeys(&ctx->keys);

        return sizeof(teoLNullEncryptionContext);
    }

    default: {
        return 0;
    }
    }
}

bool teoLNullEncryptionContextApplyKEX(teoLNullEncryptionContext *ctx,
                                       KeyExchangePayload_Common *buffer,
                                       size_t buffer_length) {
    if (!teoLNullKEXValidate(ctx, buffer, buffer_length)) {
        LTRACK_E("TeonetClient", "KEX_PACKET broken validation");
        return false;
    }

    switch (buffer->protocolId) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        KeyExchangePayload_ECDH_AES_128_V1 *kex =
            (KeyExchangePayload_ECDH_AES_128_V1 *)buffer;

        const char *err =
            initApplyRemoteKey(&ctx->keys, &kex->pubkey, &kex->salt);
        if (err != NULL) {
            LTRACK_E("TeonetClient",
                     "KEX_PACKET ECDH_AES_128_V1 failed apply: %s", err);
            return false;
        }
        ctx->state = SESCRYPT_ESTABLISHED;
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "KEX_PACKET ECDH_AES_128_V1");

        return true;
    }

    default: {
        // Already checked in teoLNullKEXValidate
        return false;
    }
    }
}

const uint32_t PACKET_ENCRYPTED_FLAG = 0x80;

bool teoLNullPacketIsEncrypted(teoLNullCPacket *packet) {
    return PACKET_ENCRYPTED_FLAG ==
           (packet->reserved_1 & PACKET_ENCRYPTED_FLAG);
}

inline void _packetSetIsEncrypted(teoLNullCPacket *packet, bool is_encrypted) {
    if (is_encrypted) {
        packet->reserved_1 |= PACKET_ENCRYPTED_FLAG;
    } else {
        packet->reserved_1 &= ~PACKET_ENCRYPTED_FLAG;
    }
}

void teoLNullPacketEncrypt(teoLNullEncryptionContext *ctx, teoLNullCPacket *packet) {
    if (ctx == NULL) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip encryption - NO CTX");
        return;
    }

    if (ctx->state != SESCRYPT_ESTABLISHED) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - CTX_STATE %s (%d)\n",
                STRING_teoLNullEncryptionProtocol(ctx->state), (int)ctx->state);
        return;
    }

    switch (ctx->enc_proto) {
    case ENC_PROTO_DISABLED: {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - ENC_PROTO_DISABLED\n");
    } break;

    case ENC_PROTO_ECDH_AES_128_V1: {
        if (packet->data_length) {
            XCrypt_AES128_1(&ctx->keys.sessionkey, ctx->sendNonce,
                            teoLNullPacketGetPayload(packet),
                            packet->data_length);

            _packetSetIsEncrypted(packet, true);
            ctx->sendNonce++;
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Encrypted - ENC_PROTO_ECDH_AES_128_V1");
        } else {
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Skip - NO_DATA_TO_ENCRYPT\n");
        }
    } break;

    default: {
        // Invalid/unknown encryption
        LTRACK("TeonetClient", "Unexpected teoLNullEncryptionProtocol (%d)",
               (int)ctx->enc_proto);
        abort();
    } break;
    }
}

bool teoLNullPacketDecrypt(teoLNullEncryptionContext *ctx, teoLNullCPacket *packet) {
    // HINT: check is_encrypted flag first
    const bool encrypted = teoLNullPacketIsEncrypted(packet);
    if (!encrypted) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient", "Skip - NOT_ENCRYPTED\n");
        // unencrypted packets always return success
        return true;
    }

    if (ctx == NULL) {
        // TODO : separate log control
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient", "Skip - NO CTX");
        return false;
    }

    if (ctx->state != SESCRYPT_ESTABLISHED) {
        // TODO : separate log control
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - CTX_STATE %s (%d)\n",
                STRING_teoLNullEncryptionProtocol(ctx->state), (int)ctx->state);
        return false;
    }

    // encrypted packet
    switch (ctx->enc_proto) {
    case ENC_PROTO_ECDH_AES_128_V1: {
        // decrypt packet payload
        if (packet->data_length) {
            XCrypt_AES128_1(&ctx->keys.sessionkey, ctx->receiveNonce,
                            teoLNullPacketGetPayload(packet),
                            packet->data_length);
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Decrypted - ENC_PROTO_ECDH_AES_128_V1");
            // Not encrypted anymore, clear is_encrypted flag
            _packetSetIsEncrypted(packet, false);
            // Count encrypted
            ctx->receiveNonce++;
        } else {
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Skip - NO_DATA_TO_DECRYPT");
        }
    } break;

    case ENC_PROTO_DISABLED: {
        // TODO : separate log control
        // encryption disabled
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient",
                  "Skip - ENC_PROTO_DISABLED");
        return false;
    } break;

    default: {
        // Invalid/unknown encryption
        LTRACK_E("TeonetClient", "Unexpected teoLNullEncryptionProtocol = (%d)",
                 (int)ctx->enc_proto);
        abort();
    } break;
    }

    return true;
}

const char *STRING_teoLNullEncryptionProtocol(teoLNullEncryptionProtocol v) {
    switch (v) {
    case ENC_PROTO_DISABLED: return "ENC_PROTO_DISABLED";
    case ENC_PROTO_ECDH_AES_128_V1: return "ENC_PROTO_ECDH_AES_128_V1";
    default: break;
    }

    return "INVALID teoLNullEncryptionProtocol";
}

const char *
STRING_teoLNullEncryptedSessionState(teoLNullEncryptedSessionState v) {
    switch (v) {
    case SESCRYPT_PENDING: return "SESCRYPT_PENDING";
    case SESCRYPT_ESTABLISHED: return "SESCRYPT_ESTABLISHED";
    default: break;
    }

    return "INVALID teoLNullEncryptedSessionState";
}
