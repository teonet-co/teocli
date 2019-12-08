#include "teonet_l0_client_crypt.h"
#include "teonet_l0_client.h"
#include "teobase/logging.h"

extern int teocliOpt_DBG_packetFlow;

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
            return 0;
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

void teoLNullPacketEncrypt(teoLNullEncryptionContext *ctx, teoLNullCPacket *packet) {
    if (!ctx) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip encryption - NO CTX");
        return;
    }

    if (ctx->state != SESCRYPT_ESTABLISHED) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - CTX_STATE %s (%d)\n",
                STRING_teoLNullEncryptionProtocol(ctx->state), ctx->state);
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

            packet->reserved_1 |= 0x80; // HINT: set is_encrypted flag
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
        LTRACK("TeonetClient", "Unexpected teoLNullEncryptionProtocol = (%d)\n",
               ctx->enc_proto);
        abort();
    } break;
    }
}

bool teoLNullPacketIsEncrypted(teoLNullCPacket *packet) {
    const bool encrypted = (packet->reserved_1 & 0x80) == 0x80;
    return encrypted;
}

bool teoLNullPacketDecrypt(teoLNullEncryptionContext *ctx, teoLNullCPacket *packet) {
    // HINT: check is_encrypted flag first
    const bool encrypted = (packet->reserved_1 & 0x80) == 0x80;
    if (!encrypted) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient", "Skip - NOT_ENCRYPTED\n");
        // unencrypted packets always return success
        return true;
    }

    if (!ctx) {
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient", "Skip - NO CTX");
        return false;
    }

    if (ctx->state != SESCRYPT_ESTABLISHED) {
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
            packet->reserved_1 &= ~0x80;
            // Count encrypted
            ctx->receiveNonce++;
        } else {
            CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                    "Skip - NO_DATA_TO_DECRYPT");
        }
    } break;

    case ENC_PROTO_DISABLED: {
        // encryption disabled
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient",
                  "Skip - ENC_PROTO_DISABLED");
        return false;
    } break;

    default: {
        // Invalid/unknown encryption
        CLTRACK_E(teocliOpt_DBG_packetFlow, "TeonetClient",
                  "Unexpected teoLNullEncryptionProtocol = (%d)\n",
                  ctx->enc_proto);
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
