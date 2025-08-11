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
 * \file ifx/nist_kdf.h
 * \brief Key derivation function (KDF) according to NIST SP 800-108 in counter mode with various pseudo-random functions (PRF)
 */

#ifndef _IFX_NIST_KDF_H_
#define _IFX_NIST_KDF_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (*nist_kdf_prf_t)(uint8_t* key, uint8_t* message, size_t message_len, uint8_t* cmac, size_t* cmac_len);

int nist_kdf_counter(nist_kdf_prf_t prf, uint8_t input_key[16], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len);

int nist_kdf_counter_aes128(uint8_t input_key[16], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len);

int nist_kdf_counter_aes192(uint8_t input_key[24], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len);

int nist_kdf_counter_aes256(uint8_t input_key[32], uint8_t counter_len, uint8_t* data_before_counter, size_t data_before_counter_len,
uint8_t* data_after_counter, size_t data_after_counter_len, uint8_t* derived_key, size_t derived_key_len);

#ifdef __cplusplus
}
#endif
#endif
