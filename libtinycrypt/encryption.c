#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "encryption.h"

void randomize_bytes(volatile uint8_t* bytes, size_t size) {
  // random source via traits/strategy
  const unsigned D = RAND_MAX / 256;
  for (size_t it = 0; it < size; ++it) {
    bytes[it] = rand() / D;
  }
}

void zero_bytes(volatile uint8_t* bytes, size_t size) {
  for (size_t it = 0; it < size; ++it) {
    bytes[it] = 0;
  }
}

void xor_bytes(volatile uint8_t* dest, const uint8_t* source, size_t size) {
  for (size_t it = 0; it < size; ++it) {
    dest[it] ^= source[it];
  }
}

void initPeerKeys(PeerKeyset* keys) {
  // compute public key based on private one and curve parameters
  for (;;) {
    randomize_bytes(keys->pvtkeylocal.data, sizeof(keys->pvtkeylocal.data));
    int ok = ecdh_generate_keys(keys->pubkeylocal.data, keys->pvtkeylocal.data);
    if (ok) {
      break;
    }
  }

  zero_bytes(keys->pubkeyremote.data, sizeof(keys->pubkeyremote.data));
  zero_bytes(keys->sharedkey.data, sizeof(keys->sharedkey.data));

  zero_bytes(keys->sessionkey.data, sizeof(keys->sessionkey.data));
  randomize_bytes(keys->sessionsalt.data, sizeof(keys->sessionsalt.data));
}

const char* initApplyRemoteKey(PeerKeyset* keys, const ECDHPubkey* remote,
                               const AES128_1_BLOCK* sessionsalt) {
  static_assert(sizeof(keys->pubkeyremote.data) == sizeof(remote->data),
                "must be equivalent");
  memcpy(keys->pubkeyremote.data, remote->data,
         sizeof(keys->pubkeyremote.data));

  int ok = ecdh_shared_secret(keys->pvtkeylocal.data, keys->pubkeyremote.data,
                              keys->sharedkey.data);
  if (!ok) {
    zero_bytes(keys->pubkeyremote.data, sizeof(keys->pubkeyremote.data));
    zero_bytes(keys->sharedkey.data, sizeof(keys->sharedkey.data));
    return "remote public key doesn't lie on the curve";
  }

  static_assert(sizeof(keys->sessionsalt.data) == sizeof(sessionsalt->data),
                "must be equivalent");
  memcpy(keys->sessionsalt.data, sessionsalt->data,
         sizeof(keys->sessionsalt.data));

  AES128_1_KEY clamped_key;
  zero_bytes(clamped_key.data, sizeof(clamped_key.data));

  if (sizeof(clamped_key.data) < sizeof(keys->sharedkey.data)) {
    memcpy(clamped_key.data, keys->sharedkey.data, sizeof(clamped_key.data));
  } else {
    memcpy(clamped_key.data, keys->sharedkey.data,
           sizeof(keys->sharedkey.data));
  }

  PBKDF2_AES128_1(&clamped_key, &keys->sessionsalt, 30, keys->sessionkey.data,
                  sizeof(keys->sessionkey.data));
  return NULL;
};

void HMAC_AES128_1(const AES128_1_KEY* key, uint8_t* message,
                   size_t message_len) {
  AES128_1_BLOCK iv;

  static_assert(sizeof(key->data) == AES_KEYLEN, "Must be equivalent");
  static_assert(sizeof(iv.data) == AES_BLOCKLEN, "Must be equivalent");

  zero_bytes(iv.data, sizeof(iv.data));

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key->data, iv.data);
  AES_CTR_xcrypt_buffer(&ctx, message, message_len);
}

void XCrypt_AES128_1(const AES128_1_KEY* key, uint32_t nonce, uint8_t* message,
                     size_t message_len) {
  static_assert(sizeof(key->data) == AES_KEYLEN, "Must be equivalent");
  // HINT hardcoded init vector
  static uint8_t hardIv[] = {
      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
      0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
  };
  static_assert(sizeof(hardIv) == AES_BLOCKLEN, "Must be equivalent");

  AES128_1_BLOCK iv;
  static_assert(sizeof(iv.data) == AES_BLOCKLEN, "Must be equivalent");
  memcpy(iv.data, hardIv, sizeof(iv.data));

  static_assert(sizeof(iv.data) > sizeof(nonce), "counter must fit in IV");
  const size_t ofs = sizeof(iv.data) - sizeof(nonce);
  xor_bytes(iv.data + ofs, (const uint8_t*)(&nonce), sizeof(nonce));

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key->data, iv.data);
  AES_CTR_xcrypt_buffer(&ctx, message, message_len);
}

// TODO add different algos
void PBKDF2_AES128_1(const AES128_1_KEY* key, const AES128_1_BLOCK* salt,
                     int n_rounds, uint8_t* derived_key, size_t dk_len) {
  static_assert(sizeof(key->data) == AES_KEYLEN, "Must be equivalent");
  static_assert(sizeof(salt->data) == AES_BLOCKLEN, "Must be equivalent");

  AES128_1_BLOCK U_n;
  uint64_t curRound = 1;
  static_assert(sizeof(U_n.data) > sizeof(curRound), "counter must fit in IV");
  const size_t ofs = sizeof(U_n.data) - sizeof(curRound);
  for (size_t writePos = 0; writePos < dk_len;) {
    // T_i = F(key, salt, n_rounds, i)
    // F(P, S, c, i) = U_1 ^ U_2 ^ ... ^ U_n_rounds
    // U_0 = salt || INT (i)
    // U_n = PRF (P, U_(n-1))

    // create U_0
    memcpy(U_n.data, salt->data, sizeof(salt->data));
    xor_bytes(U_n.data + ofs, (const uint8_t*)(&curRound), sizeof(curRound));

    AES128_1_BLOCK T_i;
    zero_bytes(T_i.data, sizeof(T_i.data));
    for (int N = 1; N <= n_rounds; ++N) {
      HMAC_AES128_1(key, U_n.data, sizeof(U_n.data));
      xor_bytes(T_i.data, U_n.data, sizeof(U_n.data));
    }

    size_t writeLen = dk_len - writePos;
    if (writeLen > sizeof(T_i.data)) {
      writeLen = sizeof(T_i.data);
    }
    memcpy(derived_key + writePos, T_i.data, writeLen);
    writePos += writeLen;
    curRound++;
  }
}
