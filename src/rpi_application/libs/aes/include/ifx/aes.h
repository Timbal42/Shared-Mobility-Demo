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
 * \file ifx/aes.h
 * \brief AES interface
 */
#ifndef _IFX_AES_H_
#define _IFX_AES_H_

#include <stddef.h>
#include <stdint.h>

#define AES_KEYLEN_128 16
#define AES_KEYLEN_192 24
#define AES_KEYLEN_256 32

#define AES_ICV_LEN 16

#ifdef __cplusplus
extern "C"
{
#endif
int aes_random_bytes(uint8_t* buffer, size_t num);

int aes_cmac_update(void* context, uint8_t* message, size_t messange_len);
int aes_cmac_generate(void* context, uint8_t* cmac, size_t* cmac_len);

int aes_cmac_128(uint8_t key[AES_KEYLEN_128], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len);
int aes_cmac_128_init(uint8_t key[AES_KEYLEN_128], void** context);

int aes_cmac_192(uint8_t key[AES_KEYLEN_192], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len);
int aes_cmac_192_init(uint8_t key[AES_KEYLEN_192], void** context);

int aes_cmac_256(uint8_t key[AES_KEYLEN_256], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len);
int aes_cmac_256_init(uint8_t key[AES_KEYLEN_256], void** context);

int aes_cbc_encrypt_128(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len);
int aes_cbc_encrypt_192(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len);
int aes_cbc_encrypt_256(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len);

int aes_cbc_decrypt_128(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len);
int aes_cbc_decrypt_192(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len);
int aes_cbc_decrypt_256(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len);

#ifdef __cplusplus
}
#endif

#endif
