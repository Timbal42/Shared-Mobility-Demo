/*
 * MIT License
 *
 * Copyright (c) 2020 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file ifx/scp03_crypto.h
 * \brief Cryptography abstraction for Global Platform SCP03
 */
#ifndef _IFX_SCP03_CRYPTO_H_
#define _IFX_SCP03_CRYPTO_H_

#include "ifx/apdu.h"
#include <stddef.h>
#include <stdint.h>

#define SCP03_CHALLENGE_LEN 8
#define SCP03_CRYPTOGRAM_LEN 8
#define SCP03_CMAC_FULL_LENGTH 16

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint8_t* enc;
    size_t enc_len;
    uint8_t* mac;
    size_t mac_len;
    uint8_t* dek;
    size_t dek_len;
} Scp03StaticKeys;

typedef struct
{
    uint8_t* enc;
    size_t enc_len;
    uint8_t* mac;
    size_t mac_len;
    uint8_t* rmac;
    size_t rmac_len;
} Scp03SessionKeys;

int scp03_derive(uint8_t input_key[32], uint8_t scp03_derive, uint8_t context[16], uint8_t* derived_key, size_t derived_key_len);
int scp03_generate_host_challenge(uint8_t host_challenge[SCP03_CHALLENGE_LEN]);
int scp03_generate_session_keys(uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], Scp03StaticKeys* static_keys, Scp03SessionKeys* session_keys);
int scp03_generate_host_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CRYPTOGRAM_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t host_cryptogram[SCP03_CRYPTOGRAM_LEN]);
int scp03_generate_card_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t card_cryptogram[SCP03_CRYPTOGRAM_LEN]);
int scp03_verify_card_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t card_cryptogram[SCP03_CRYPTOGRAM_LEN]);
int scp03_wrap(Scp03SessionKeys* session_keys, uint8_t cmac_chaining[SCP03_CMAC_FULL_LENGTH], APDU* apdu, APDU* wrapped, uint8_t next_cmac_chaining[SCP03_CMAC_FULL_LENGTH]);
int scp03_unwrap(Scp03SessionKeys* session_keys, uint8_t cmac_chaining[SCP03_CMAC_FULL_LENGTH], APDUResponse* wrapped, APDUResponse* apdu);
int scp03_encrypt(Scp03SessionKeys* session_keys, uint32_t encryption_counter, APDU* apdu, APDU* encrypted);
int scp03_decrypt(Scp03SessionKeys* session_keys, uint32_t encryption_counter, APDUResponse* encrypted, APDUResponse* apdu);

#ifdef __cplusplus
}
#endif
#endif
