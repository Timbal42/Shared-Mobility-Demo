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
 * \file nist_kdf.c
 * \brief Key derivation function (KDF) according to NIST SP 800-108 in counter mode with various pseudo-random functions (PRF)
 */

#include "ifx/nist_kdf.h"
#include "ifx/aes.h"
#include <stdlib.h>
#include <string.h>

int nist_kdf_counter(nist_kdf_prf_t prf, uint8_t* input_key, uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
    	uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len)
{
    // TODO r
    uint16_t L = (uint16_t)derived_key_len * 8;

    size_t input_data_len = data_before_counter_len + data_after_counter_len + counter_len;
    uint8_t* input_data = (uint8_t*)malloc(input_data_len);
    memcpy(input_data, data_before_counter, data_before_counter_len);
    memcpy(input_data + data_before_counter_len + counter_len, data_after_counter, data_after_counter_len);

    // Derive
    uint8_t block[16];
    size_t block_len;
    uint16_t h = 16; // PRF output len in bytes
    uint16_t n = L / (h * 8); // iterations
    if((derived_key_len % 16))
        n++;
    for(size_t i = 0; i < n; i++)
    {
        memset(block, 0, 16);
        // Set counter value in data
        for(uint8_t j = 0; j < counter_len; j++)
        {
            uint8_t byte_pos = counter_len - j - 1; // position of byte in counter
            size_t counter = i + 1;
            counter >>= byte_pos * 8; // shift current byte to the right
            uint8_t counter_byte = (uint8_t)(counter & 0xFF); // select only rightmost byte
            input_data[data_before_counter_len + j] = counter_byte; // set byte
        }

        // Derive
        int status = prf(input_key, input_data, input_data_len, block, &block_len);
        if(status)
            return status;

        // Append
        size_t append_pos = i * h;
        size_t append_len = h;
        if(append_pos + append_len > derived_key_len)
            append_len = derived_key_len - append_pos;

        memcpy(derived_key + append_pos, block, append_len);
    }

    free(input_data);
    return 0;
}

int nist_kdf_counter_aes128(uint8_t input_key[16], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
    	uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len)
{
    return nist_kdf_counter(&aes_cmac_128, input_key, counter_len,
        data_before_counter, data_before_counter_len,
        data_after_counter, data_after_counter_len,
        derived_key, derived_key_len);
}

int nist_kdf_counter_aes192(uint8_t input_key[24], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
    	uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len)
{
    return nist_kdf_counter(&aes_cmac_192, input_key, counter_len,
        data_before_counter, data_before_counter_len,
        data_after_counter, data_after_counter_len,
        derived_key, derived_key_len);
}

int nist_kdf_counter_aes256(uint8_t input_key[32], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
    	uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len)
{
    return nist_kdf_counter(&aes_cmac_256, input_key, counter_len,
        data_before_counter, data_before_counter_len,
        data_after_counter, data_after_counter_len,
        derived_key, derived_key_len);
}
