#include "teonet_l0_client_crypt.h"
#include "teonet_l0_client.h"
#include "teobase/logging.h"

extern int teocliOpt_DBG_packetFlow;

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

void teoLNullPacketDecrypt(teoLNullEncryptionContext *ctx, teoLNullCPacket *packet) {
    if (!ctx) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient", "Skip - NO CTX");
        return;
    }

    if (ctx->state != SESCRYPT_ESTABLISHED) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - CTX_STATE %s (%d)\n",
                STRING_teoLNullEncryptionProtocol(ctx->state), (int)ctx->state);
        return;
    }

    // HINT: check is_encrypted flag
    if ((packet->reserved_1 & 0x80) == 0x80) {
        CLTRACK(teocliOpt_DBG_packetFlow, "TeonetClient",
                "Skip - NOT_ENCRYPTED\n");
        return;
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
        LTRACK("TeonetClient", "Skip - ENC_PROTO_DISABLED");
    } break;

    default: {
        // Invalid/unknown encryption
        LTRACK("TeonetClient", "Unexpected teoLNullEncryptionProtocol = (%d)\n",
               ctx->enc_proto);
        abort();
    } break;
    }
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
