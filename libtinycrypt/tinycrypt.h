#pragma once
#include <stdlib.h>
#include <stdint.h>
#ifdef __cplusplus
#include "tiny-AES-c/aes.hpp"
#else
#include "tiny-AES-c/aes.h"
#endif /* __cplusplus */
#include "tiny-ECDH-c/ecdh.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void randomize_bytes(volatile uint8_t* bytes, size_t size);
void zero_bytes(volatile uint8_t* bytes, size_t size);
void xor_bytes(volatile uint8_t* dest, const uint8_t* source, size_t size);

// HINT    ECC_PUB_KEY_SIZE = 2*ECC_PRV_KEY_SIZE
/// public part of ECDH key
typedef struct {
  uint8_t data[ECC_PUB_KEY_SIZE];
} ECDHPubkey;

/// private part of ECDH key
typedef struct {
  uint8_t data[ECC_PRV_KEY_SIZE];
} ECDHPvtkey;

///< shared part of ECDH key
typedef struct {
  uint8_t data[ECC_PRV_KEY_SIZE];
} ECDHShrkey;

///< AES key
typedef struct {
  uint8_t data[AES_KEYLEN];
} AES128_1_KEY;

///< AES encryption block
typedef struct {
  uint8_t data[AES_BLOCKLEN];
} AES128_1_BLOCK;

typedef struct {
  ECDHPvtkey pvtkeylocal;
  ECDHPubkey pubkeylocal;

  ECDHPubkey pubkeyremote;
  ECDHPvtkey sharedkey;

  AES128_1_KEY sessionkey;
  AES128_1_BLOCK sessionsalt;
} PeerKeyset;

/// randomize pvtkeylocal & sessionsalt, compute pubkeylocal, zero pubkeyremote,
/// sharedkey, sessionkey
void initPeerKeys(PeerKeyset* keys);
///< copy remote key, session salt in PeerKeyset, compute sharedkey, sessionkey
///< returns error message
const char* initApplyRemoteKey(PeerKeyset* keys, const ECDHPubkey* remote,
                               const AES128_1_BLOCK* sessionsalt);

void HMAC_AES128_1(const AES128_1_KEY* key, uint8_t* message,
                   size_t message_len);

void XCrypt_AES128_1(const AES128_1_KEY* key, uint32_t nonce, uint8_t* message,
                     size_t message_len);

void PBKDF2_AES128_1(const AES128_1_KEY* key, const AES128_1_BLOCK* salt,
                     int n_rounds, uint8_t* derived_key, size_t dk_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
