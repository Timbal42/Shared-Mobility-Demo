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
 * \file scp03_crypto.c
 * \brief Cryptography for Global Platform SCP03
 */

#include "ifx/scp03_crypto.h"
#include "ifx/aes.h"
#include "ifx/nist_kdf.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*cmac_init_function_t)(uint8_t* key, void** context);

uint8_t zeros[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * \brief Checks if given APDU response status word indicates a successful value
 *
 * \param sw APDU response status word to be checked
 * \return bool \c true if status word indicates success
 * \todo Define in header to be reusable
 */
static bool _sw_success(uint16_t sw)
{
    // XXX: 0x9000 should be the only valid success status word but V2X also uses 0x9001 for FIPS non-approved mode
    return (sw == 0x9000) || (sw == 0x9001);
}

int scp03_derive(uint8_t input_key[32], uint8_t derivation_constant, uint8_t context[16], uint8_t* derived_key, size_t derived_key_len)
{
    // Prepare data
    uint16_t L = (uint16_t)derived_key_len * 8; // length of derived key in bits

    uint8_t data_before_counter[15];
    memset(data_before_counter, 0, 11);
    data_before_counter[11] = derivation_constant;
    data_before_counter[12] = 0x00;
    data_before_counter[13] = (uint8_t)((L & 0xFF00) >> 8);
    data_before_counter[14] = (uint8_t)(L & 0x00FF);

    // Derive
    return nist_kdf_counter_aes256(input_key, 1, data_before_counter, 15, context, 16, derived_key, derived_key_len);
}

int scp03_generate_host_challenge(uint8_t host_challenge[SCP03_CHALLENGE_LEN])
{
    return aes_random_bytes(host_challenge, SCP03_CHALLENGE_LEN);
}

int scp03_generate_session_keys(uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], Scp03StaticKeys* static_keys, Scp03SessionKeys* session_keys)
{
    uint8_t context[SCP03_CHALLENGE_LEN * 2];
    memcpy(context, host_challenge, SCP03_CHALLENGE_LEN);
    memcpy(context + SCP03_CHALLENGE_LEN, card_challenge, SCP03_CHALLENGE_LEN);

    session_keys->enc_len = 32;
    session_keys->enc = (uint8_t*)malloc(session_keys->enc_len);
    int status = scp03_derive(static_keys->enc, 0x04, context, session_keys->enc, session_keys->enc_len);
    if(status)
        return status;

    session_keys->mac_len = 32;
    session_keys->mac = (uint8_t*)malloc(session_keys->mac_len);
    status = scp03_derive(static_keys->mac, 0x06, context, session_keys->mac, session_keys->enc_len);
    if(status)
        return status;

    session_keys->rmac_len = 32;
    session_keys->rmac = (uint8_t*)malloc(session_keys->rmac_len);
    return scp03_derive(static_keys->mac, 0x07, context, session_keys->rmac, session_keys->enc_len);
}

int scp03_generate_cryptogram(Scp03SessionKeys* session_keys, uint8_t challenge1[SCP03_CHALLENGE_LEN], uint8_t challenge2[SCP03_CHALLENGE_LEN],
    uint8_t derivation_constant, uint8_t cryptogram[SCP03_CRYPTOGRAM_LEN])
{
    // context = [challenge1] .. [challenge2]
    uint8_t context[16];
    memcpy(context, challenge1, SCP03_CHALLENGE_LEN);
    memcpy(context + SCP03_CHALLENGE_LEN, challenge2, SCP03_CHALLENGE_LEN);

    // Derive
    return scp03_derive(session_keys->mac, derivation_constant, context, cryptogram, SCP03_CRYPTOGRAM_LEN);
}

int scp03_generate_host_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CRYPTOGRAM_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t host_cryptogram[SCP03_CRYPTOGRAM_LEN])
{
    return scp03_generate_cryptogram(session_keys, host_challenge, card_challenge, 0x01, host_cryptogram);
}

int scp03_generate_card_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t card_cryptogram[SCP03_CRYPTOGRAM_LEN])
{
    return scp03_generate_cryptogram(session_keys, host_challenge, card_challenge, 0x00, card_cryptogram);
}

int scp03_verify_card_cryptogram(Scp03SessionKeys* session_keys, uint8_t host_challenge[SCP03_CHALLENGE_LEN], uint8_t card_challenge[SCP03_CHALLENGE_LEN], uint8_t card_cryptogram[SCP03_CRYPTOGRAM_LEN])
{
    uint8_t calculated_cryptogram[SCP03_CRYPTOGRAM_LEN];
    int status = scp03_generate_card_cryptogram(session_keys, host_challenge, card_challenge, calculated_cryptogram);
    if(status)
        return status;

    for(size_t i = 0; i < SCP03_CRYPTOGRAM_LEN; i++)
    {
        if(card_cryptogram[i] != calculated_cryptogram[i])
            return -1;
    }
    return 0;
}

int scp03_get_cmac_init_from_keylen(size_t key_len, cmac_init_function_t* cmac_init)
{
    switch(key_len)
    {
        case AES_KEYLEN_128:
            *cmac_init = &aes_cmac_128_init;
            break;
        case AES_KEYLEN_192:
            *cmac_init = &aes_cmac_192_init;
            break;
        case AES_KEYLEN_256:
            *cmac_init = &aes_cmac_256_init;
            break;
        default:
            // Invalid key length
            return -1;
    }
    return 0;
}

void scp03_command_cmac_prepare(APDU* apdu, APDU* wrapped, uint8_t* cla)
{
    memcpy(wrapped, apdu, sizeof(APDU));

    // Update header
    if(wrapped->cla & 0x40) // Further Class Byte Coding (11.1.4.2)
    {
        *cla = wrapped->cla | 0x20;     // Set secure messaging
        wrapped->cla = *cla & 0xF0;  // Unset logical channel info;
    }
    else // First Class Byte Coding (11.1.4.1)
    {
        *cla = wrapped->cla | 0x04;     // set secure messaging
        wrapped->cla = *cla & 0xFC;  // unset logical channel info;
    }
    wrapped->lc += 8;
}

int scp03_command_cmac(uint8_t* mac, size_t mac_len, uint8_t cmac_chaining[SCP03_CMAC_FULL_LENGTH], APDU* wrapped, uint8_t cmac[SCP03_CMAC_FULL_LENGTH])
{
    // Choose CMAC algorithm based in key length
    cmac_init_function_t cmac_init;
    int status = scp03_get_cmac_init_from_keylen(mac_len, &cmac_init);
    if(status)
        return status;

    // Pass key to CMAC generator
    void* cmac_context;
    cmac_init(mac, &cmac_context);

    // Pass chaining value to CMAC generator
    aes_cmac_update(cmac_context, cmac_chaining, SCP03_CMAC_FULL_LENGTH);

    // Pass APDU with updated header to CMAC generator
    aes_cmac_update(cmac_context, (uint8_t*)&(wrapped->cla), 1);
    aes_cmac_update(cmac_context, (uint8_t*)&(wrapped->ins), 1);
    aes_cmac_update(cmac_context, (uint8_t*)&(wrapped->p1), 1);
    aes_cmac_update(cmac_context, (uint8_t*)&(wrapped->p2), 1);
    aes_cmac_update(cmac_context, (uint8_t*)&(wrapped->lc), 1);
    aes_cmac_update(cmac_context, wrapped->data, wrapped->lc - 8);

    // Generate CMAC
    size_t cmac_len;
    aes_cmac_generate(cmac_context, cmac, &cmac_len);
    return 0;
}

void scp03_command_cmac_finish(APDU* wrapped, uint8_t cla, uint8_t cmac[SCP03_CMAC_FULL_LENGTH], uint8_t next_cmac_chaining[SCP03_CMAC_FULL_LENGTH])
{
    // Reset logical channel info
    wrapped->cla = cla;

    // Copy result to output
    uint8_t* original_data = wrapped->data;
    size_t original_data_len = wrapped->lc - 8;
    wrapped->data = (uint8_t*)malloc(wrapped->lc);
    memcpy(wrapped->data, original_data, original_data_len);
    memcpy(wrapped->data + original_data_len, cmac, 8);

    // Set chaining value for next transaction
    memcpy(next_cmac_chaining, cmac, SCP03_CMAC_FULL_LENGTH);
}

int scp03_wrap(Scp03SessionKeys* session_keys, uint8_t cmac_chaining[SCP03_CMAC_FULL_LENGTH], APDU* apdu, APDU* wrapped, uint8_t next_cmac_chaining[SCP03_CMAC_FULL_LENGTH])
{
    uint8_t cla;
    scp03_command_cmac_prepare(apdu, wrapped, &cla);

    uint8_t cmac[SCP03_CMAC_FULL_LENGTH];
    int status = scp03_command_cmac(session_keys->mac, session_keys->mac_len, cmac_chaining, wrapped, cmac);
    if(status)
        return status;

    scp03_command_cmac_finish(wrapped, cla, cmac, next_cmac_chaining);
    return 0;
}

int scp03_unwrap(Scp03SessionKeys* session_keys, uint8_t cmac_chaining[SCP03_CMAC_FULL_LENGTH], APDUResponse* wrapped, APDUResponse* apdu)
{
    // Skip CMAC on error code
    if (!_sw_success(wrapped->sw) &&
       (wrapped->sw & 0xFF00) != 0x6200 &&
       (wrapped->sw & 0xFF00) != 0x6300)
    {
        apdu->sw = wrapped->sw;
        apdu->len = 0;
        apdu->data = NULL;
        return 0;
    }

    // Choose CMAC algorithm based in key length
    cmac_init_function_t cmac_init;
    int status = scp03_get_cmac_init_from_keylen(session_keys->rmac_len, &cmac_init);
    if(status)
        return status;

    // Pass key to CMAC generator
    void* cmac_context;
    cmac_init(session_keys->rmac, &cmac_context);

    // Pass chaining value to CMAC generator
    aes_cmac_update(cmac_context, cmac_chaining, SCP03_CMAC_FULL_LENGTH);

    // Pass response without R-CMAC to CMAC generator
    if(wrapped->len > 8)
    {
        aes_cmac_update(cmac_context, wrapped->data, wrapped->len - 8);
    }
    uint8_t sw_msb = (wrapped->sw >> 8) & 0xFF;
    uint8_t sw_lsb = (wrapped->sw) & 0xFF;
    aes_cmac_update(cmac_context, &sw_msb, 1);
    aes_cmac_update(cmac_context, &sw_lsb, 1);

    // Generate CMAC
    uint8_t cmac[SCP03_CMAC_FULL_LENGTH];
    size_t cmac_len;
    aes_cmac_generate(cmac_context, cmac, &cmac_len);

    uint8_t* received_cmac = wrapped->data + wrapped->len - 8;
    for(size_t i = 0; i < 8; i++)
    {
        if(received_cmac[i] != cmac[i])
        {
            // Invald R-MAC
            return -2;
        }
    }

    // Copy result to output
    apdu->sw = wrapped->sw;
    apdu->len = wrapped->len - 8;
    if(apdu->len > 0)
    {
        apdu->data = (uint8_t*)malloc(apdu->len);
        memcpy(apdu->data, wrapped->data, apdu->len);
    }

    return 0;
}

void scp03_pad_counter(uint32_t counter, uint8_t padded[16])
{
    memset(padded, 0, 13);
    padded[13] = (uint8_t)((counter >> 16) & 0xFF);
    padded[14] = (uint8_t)((counter >> 8) & 0xFF);
    padded[15] = (uint8_t)((counter) & 0xFF);
}

void scp03_pad(uint8_t* data, size_t data_len, uint8_t** padded, size_t* padded_len)
{
    // Data + 0x80
    *padded_len = data_len + 1;

    // Pad to full block
    size_t additional_bytes = (*padded_len) % 16;
    if(additional_bytes > 0)
    {
        *padded_len -= additional_bytes;
        *padded_len += 16;
    }

    *padded = (uint8_t*)malloc(*padded_len);
    memcpy(*padded, data, data_len);
    (*padded)[data_len] = 0x80;

    if(additional_bytes > 0)
    {
        size_t num_zeros = 16 - additional_bytes;
        memset((*padded) + data_len + 1, 0, num_zeros);
    }
}

int scp03_encrypt(Scp03SessionKeys* session_keys, uint32_t encryption_counter, APDU* apdu, APDU* encrypted)
{
    // Choose algorithm based in key length
    int (*encrypt)(uint8_t* key, uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len);
    switch(session_keys->enc_len)
    {
        case AES_KEYLEN_128:
            encrypt = &aes_cbc_encrypt_128;
            break;
        case AES_KEYLEN_192:
            encrypt = &aes_cbc_encrypt_192;
            break;
        case AES_KEYLEN_256:
            encrypt = &aes_cbc_encrypt_256;
            break;
        default:
            // Invalid key length
            return -1;
    }

    // Pad counter to full block
    uint8_t padded_counter[16];
    scp03_pad_counter(encryption_counter, padded_counter);

    // Generate ICV (encrypt counter)
    uint8_t icv[16];
    size_t icv_len;
    memset(icv, 0, 16);
    encrypt(session_keys->enc, zeros, padded_counter, 16, icv, &icv_len);

    // Copy header
    memcpy(encrypted, apdu, sizeof(APDU));

    // Pad data to full block
    uint8_t* padded_data;
    size_t padded_data_len;
    scp03_pad(apdu->data, apdu->lc, &padded_data, &padded_data_len);

    // Encrypt data
    encrypted->data = (uint8_t*)malloc(padded_data_len);
    encrypt(session_keys->enc, icv, padded_data, padded_data_len, encrypted->data, &(encrypted->lc));

    free(padded_data);
    return 0;
}

int scp03_decrypt(Scp03SessionKeys* session_keys, uint32_t encryption_counter, APDUResponse* encrypted, APDUResponse* apdu)
{
    // Choose algorithm based in key length
    int (*encrypt)(uint8_t* key, uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len);
    int (*decrypt)(uint8_t* key, uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len);
    switch(session_keys->enc_len)
    {
        case AES_KEYLEN_128:
            encrypt = &aes_cbc_encrypt_128;
            decrypt = &aes_cbc_decrypt_128;
            break;
        case AES_KEYLEN_192:
            encrypt = &aes_cbc_encrypt_192;
            decrypt = &aes_cbc_decrypt_192;
            break;
        case AES_KEYLEN_256:
            encrypt = &aes_cbc_encrypt_256;
            decrypt = &aes_cbc_decrypt_256;
            break;
        default:
            // Invalid key length
            return -1;
    }

    // Pad counter to full block and set leftmost byte to 0x80
    uint8_t padded_counter[16];
    scp03_pad_counter(encryption_counter, padded_counter);
    padded_counter[0] = 0x80;

    // Generate ICV (encrypt counter)
    uint8_t icv[16];
    size_t icv_len;
    memset(icv, 0, 16);
    encrypt(session_keys->enc, zeros, padded_counter, 16, icv, &icv_len);

    // Copy header
    memcpy(apdu, encrypted, sizeof(APDUResponse));

    // Decrypt data
    apdu->data = (uint8_t*)malloc(encrypted->len);
    decrypt(session_keys->enc, icv, encrypted->data, encrypted->len, apdu->data, &apdu->len);

    while(apdu->data[apdu->len - 1] == 0x00)
    {
        apdu->len--;
    }

    if(apdu->data[apdu->len - 1] != 0x80)
    {
        // Invalid padding
        return -2;
    }
    apdu->len--;

    return 0;
}
