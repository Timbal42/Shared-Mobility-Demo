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
 * \file aes-openssl.c
 * \brief AES interface implementation based on OpenSSL
 */
#include "ifx/aes.h"
#include "openssl/cmac.h"
#include "openssl/evp.h"
#include "openssl/kdf.h"
#include "openssl/rand.h"

int aes_random_bytes(uint8_t* buffer, size_t num)
{
    //return !RAND_priv_bytes(buffer, (int)num);
    return !RAND_bytes(buffer, (int)num);
    
}

int aes_cmac_update(void* context, uint8_t* message, size_t message_len)
{
    CMAC_CTX* ctx = (CMAC_CTX*)context;
    return !CMAC_Update(ctx, message, message_len);
}

int aes_cmac_generate(void* context, uint8_t* cmac, size_t* cmac_len)
{
    CMAC_CTX* ctx = (CMAC_CTX*)context;
    int status = CMAC_Final(ctx, cmac, cmac_len);
    CMAC_CTX_free(ctx);
    return !status;
}

int aes_cmac_128(uint8_t key[AES_KEYLEN_128], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len)
{
    void* context;
    int status = aes_cmac_128_init(key, &context);
    if(!status)
        status = aes_cmac_update(context, message, message_len);
    if(!status)
        status = aes_cmac_generate(context, cmac, cmac_len);
    return status;
}

int aes_cmac_128_init(uint8_t key[AES_KEYLEN_128], void** context)
{
    CMAC_CTX *ctx = CMAC_CTX_new();
    int status = CMAC_Init(ctx, key, AES_KEYLEN_128, EVP_aes_128_cbc(), NULL);
    *context = ctx;
    return !status;
}

int aes_cmac_192(uint8_t key[AES_KEYLEN_192], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len)
{
    void* context;
    int status = aes_cmac_192_init(key, &context);
    if(!status)
        status = aes_cmac_update(context, message, message_len);
    if(!status)
        status = aes_cmac_generate(context, cmac, cmac_len);
    return status;
}

int aes_cmac_192_init(uint8_t key[AES_KEYLEN_192], void** context)
{
    CMAC_CTX *ctx = CMAC_CTX_new();
    int status = CMAC_Init(ctx, key, AES_KEYLEN_192, EVP_aes_192_cbc(), NULL);
    *context = ctx;
    return !status;
}

int aes_cmac_256(uint8_t key[AES_KEYLEN_256], uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len)
{
    void* context;
    int status = aes_cmac_256_init(key, &context);
    if(!status)
        status = aes_cmac_update(context, message, message_len);
    if(!status)
        status = aes_cmac_generate(context, cmac, cmac_len);
    return status;
}

int aes_cmac_256_init(uint8_t key[AES_KEYLEN_256], void** context)
{
    CMAC_CTX *ctx = CMAC_CTX_new();
    int status = CMAC_Init(ctx, key, AES_KEYLEN_256, EVP_aes_256_cbc(), NULL);
    *context = ctx;
    return !status;
}

int aes_cbc_encrypt_128_init(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_encrypt_192_init(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_192_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_encrypt_256_init(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_encrypt_update_and_finalize(void* context, uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len)
{
    EVP_CIPHER_CTX* ctx = (EVP_CIPHER_CTX*)context;
    int len;
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    EVP_EncryptUpdate(ctx, cipher, &len, message, (int)message_len);
    *cipher_len = (size_t)len;
    EVP_EncryptFinal_ex(ctx, cipher + len, &len);
    *cipher_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

int aes_cbc_encrypt_128(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_encrypt_128_init(key, icv, (void*)&ctx);
    aes_cbc_encrypt_update_and_finalize(ctx, message, message_len, cipher, cipher_len);
    return 0;
}

int aes_cbc_encrypt_192(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_encrypt_192_init(key, icv, (void*)&ctx);
    aes_cbc_encrypt_update_and_finalize(ctx, message, message_len, cipher, cipher_len);
    return 0;
}

int aes_cbc_encrypt_256(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], uint8_t* message, size_t message_len, uint8_t* cipher, size_t* cipher_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_encrypt_256_init(key, icv, (void*)&ctx);
    aes_cbc_encrypt_update_and_finalize(ctx, message, message_len, cipher, cipher_len);
    return 0;
}

int aes_cbc_decrypt_128_init(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_decrypt_192_init(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_192_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_decrypt_256_init(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], void** context)
{
    EVP_CIPHER_CTX* ctx;
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, icv);
    *context = ctx;
    return 0;
}

int aes_cbc_decrypt_update_and_finalize(void* context, uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len)
{
    EVP_CIPHER_CTX* ctx = (EVP_CIPHER_CTX*)context;
    int len;
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    EVP_DecryptUpdate(ctx, message, &len, cipher, (int)cipher_len);
    *message_len = (size_t)len;

    EVP_DecryptFinal_ex(ctx, message + len, &len);
    *message_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

int aes_cbc_decrypt_128(uint8_t key[AES_KEYLEN_128], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_decrypt_128_init(key, icv, (void*)&ctx);
    aes_cbc_decrypt_update_and_finalize(ctx, cipher, cipher_len, message, message_len);

    return 0;
}

int aes_cbc_decrypt_192(uint8_t key[AES_KEYLEN_192], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_decrypt_192_init(key, icv, (void*)&ctx);
    aes_cbc_decrypt_update_and_finalize(ctx, cipher, cipher_len, message, message_len);

    return 0;
}

int aes_cbc_decrypt_256(uint8_t key[AES_KEYLEN_256], uint8_t icv[AES_ICV_LEN], uint8_t* cipher, size_t cipher_len, uint8_t* message, size_t* message_len)
{
    EVP_CIPHER_CTX* ctx;
    aes_cbc_decrypt_256_init(key, icv, (void*)&ctx);
    aes_cbc_decrypt_update_and_finalize(ctx, cipher, cipher_len, message, message_len);

    return 0;
}
