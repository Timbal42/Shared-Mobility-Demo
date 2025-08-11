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
 * \file t1prime-datastructures-tests.cpp
 * \brief Tests for en-/decoding of Global Platform T=1' datastructures
 */
#include "catch2/catch.hpp"
#include <stdlib.h>
#include <string.h>
#include "ifx/error.h"
#include "ifx/t1prime.h"
#include "t1prime.h"
#include "t1prime/datastructures.h"

/**
 * \brief Check if two bytearrays are equal
 *
 * \param first First bytearray for comparison
 * \param first_length Number of bytes in \p first
 * \param second Second byte arrayfor comparison
 * \param second_length Number of bytes in \p second
 * \return true if arrays are equal
 */
static bool bytearray_equal(uint8_t *first, size_t first_length, uint8_t *second, size_t second_length)
{
    return (first_length == second_length) && (memcmp(first, second, first_length) == 0);
}

TEST_CASE("Encode Block without information", "[Block, t1prime_block_encode]")
{
    Block block = {0x21, 0x80, 0x00, NULL};

    uint8_t *encoded;
    size_t encoded_len;
    int status = t1prime_block_encode(&block, &encoded, &encoded_len);
    REQUIRE(status == T1PRIME_BLOCK_ENCODE_SUCCESS);
    uint8_t expected[] = {0x21, 0x80, 0x00, 0x00, 0x63, 0xda};
    size_t expected_len = sizeof(expected);
    REQUIRE(bytearray_equal(encoded, encoded_len, expected, expected_len));

    free(encoded);
}

TEST_CASE("Encode Block with information", "[Block, t1prime_block_encode]")
{
    uint8_t ifs[] = {0x01, 0x02};
    Block block = {0x21, 0xc1, 0x02, ifs};

    uint8_t *encoded;
    size_t encoded_len;
    int status = t1prime_block_encode(&block, &encoded, &encoded_len);
    REQUIRE(status == T1PRIME_BLOCK_ENCODE_SUCCESS);
    uint8_t expected[] = {0x21, 0xc1, 0x00, 0x02, 0x01, 0x02, 0xb9, 0x85};
    size_t expected_len = sizeof(expected);
    REQUIRE(bytearray_equal(encoded, encoded_len, expected, expected_len));

    free(encoded);
}

TEST_CASE("Decode Block without information", "[Block, t1prime_block_decode]")
{
    uint8_t encoded[] = {0x21, 0x80, 0x00, 0x00, 0x63, 0xda};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(status == T1PRIME_BLOCK_DECODE_SUCCESS);
    REQUIRE(block.nad == 0x21);
    REQUIRE(block.pcb == 0x80);
    REQUIRE(block.information_size == 0x00);
    REQUIRE(block.information == NULL);

    t1prime_block_destroy(&block);
}

TEST_CASE("Decode Block with information", "[Block, t1prime_block_decode]")
{
    uint8_t encoded[] = {0x21, 0xc1, 0x00, 0x02, 0x01, 0x02, 0xb9, 0x85};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(status == T1PRIME_BLOCK_DECODE_SUCCESS);
    REQUIRE(block.nad == 0x21);
    REQUIRE(block.pcb == 0xc1);
    REQUIRE(block.information_size == 0x02);
    uint8_t ifs[] = {0x01, 0x02};
    REQUIRE(bytearray_equal(block.information, block.information_size, ifs, sizeof(ifs)));

    t1prime_block_destroy(&block);
}

TEST_CASE("Decode invalid Block with too little data", "[Block, t1prime_block_decode, Error]")
{
    uint8_t encoded[] = {0x21, 0xc1, 0x00};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_BLOCK_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode invalid Block with information size indicating more data than actually present", "[Block, t1prime_block_decode, Error]")
{
    uint8_t encoded[] = {0x21, 0xc1, 0x00, 0x01, 0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_BLOCK_DECODE);
    REQUIRE(ifx_error_get_reason(status) == INFORMATIONSIZE_MISMATCH);
}

TEST_CASE("Decode invalid Block with information size indicating less data than actually present", "[Block, t1prime_block_decode, Error]")
{
    uint8_t encoded[] = {0x21, 0xc1, 0x00, 0x01, 0x01, 0x02, 0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_BLOCK_DECODE);
    REQUIRE(ifx_error_get_reason(status) == INFORMATIONSIZE_MISMATCH);
}

TEST_CASE("Decode Block with invalid CRC", "[Block, t1prime_block_decode, Error]")
{
    uint8_t encoded[] = {0x21, 0xc1, 0x00, 0x00, 0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    Block block;
    int status = t1prime_block_decode(&block, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_BLOCK_DECODE);
    REQUIRE(ifx_error_get_reason(status) == INVALID_CRC);
}

TEST_CASE("Validate matching CRC for block without information", "[Block, t1prime_validate_crc]")
{
    Block block = {0x21, 0x80, 0x00, NULL};
    uint16_t expected = 0x63da;
    REQUIRE(t1prime_validate_crc(&block, expected) == T1PRIME_VALIDATE_CRC_SUCCESS);
}

TEST_CASE("Validate matching CRC for block with information", "[Block, t1prime_validate_crc]")
{
    uint8_t information[] = {0x01, 0x02};
    size_t information_size = sizeof(information);
    Block block = {0x21, 0xc1, information_size, information};
    uint16_t expected = 0xb985;
    REQUIRE(t1prime_validate_crc(&block, expected) == T1PRIME_VALIDATE_CRC_SUCCESS);
}

TEST_CASE("Validate CRC mismatch", "[Block, t1prime_validate_crc]")
{
    Block block = {0x21, 0x80, 0x00, NULL};
    int status = t1prime_validate_crc(&block, 0x0000);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_VALIDATE_CRC);
    REQUIRE(ifx_error_get_reason(status) == INVALID_CRC);
}

TEST_CASE("Decode CIP version information", "[CIP, t1prime_cip_decode]")
{
    uint8_t encoded[] = {
        0x01,
        0x03, 0x00, 0x01, 0x02,
        0x01,
        0x0C, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(status == T1PRIME_CIP_DECODE_SUCCESS);

    REQUIRE(cip.version == 0x01);

    t1prime_cip_destroy(&cip);
}

TEST_CASE("Decode CIP with 3 byte IIN according to ISO7812-1", "[CIP, t1prime_cip_decode]")
{
    uint8_t encoded[] = {
        0x01,
        0x03, 0x00, 0x01, 0x02,
        0x01,
        0x0C, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(status == T1PRIME_CIP_DECODE_SUCCESS);

    uint8_t expected_iin[] = {0x00, 0x01, 0x02};
    size_t expected_iin_len = sizeof(expected_iin);
    REQUIRE(bytearray_equal(cip.iin, cip.iin_len, expected_iin, expected_iin_len));

    t1prime_cip_destroy(&cip);
}

TEST_CASE("Decode CIP with 4 byte IIN according to ISO7812-1", "[CIP, t1prime_cip_decode]")
{
    uint8_t encoded[] = {
        0x01,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x01,
        0x0C, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(status == T1PRIME_CIP_DECODE_SUCCESS);

    uint8_t expected_iin[] = {0x00, 0x01, 0x02, 0x03};
    size_t expected_iin_len = sizeof(expected_iin);
    REQUIRE(bytearray_equal(cip.iin, cip.iin_len, expected_iin, expected_iin_len));

    t1prime_cip_destroy(&cip);
}

TEST_CASE("Decode CIP with historical bytes", "[CIP, t1prime_cip_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x02,
        0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x02, 0x00, 0x01};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(status == T1PRIME_CIP_DECODE_SUCCESS);
    REQUIRE(cip.version == 0x01);

    uint8_t expected_iin[] = {0x00, 0x01, 0x02, 0x03};
    size_t expected_iin_len = sizeof(expected_iin);
    REQUIRE(cip.iin_len == expected_iin_len);
    REQUIRE(bytearray_equal(cip.iin, cip.iin_len, expected_iin, expected_iin_len));

    REQUIRE(cip.plid == 0x02);

    uint8_t expected_plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    size_t expected_plp_len = sizeof(expected_plp);
    REQUIRE(cip.plp_len == expected_plp_len);
    REQUIRE(bytearray_equal(cip.plp, cip.plp_len, expected_plp, expected_plp_len));

    uint8_t expected_dllp[] = {0x00, 0x01, 0x02, 0x03};
    size_t expected_dllp_len = sizeof(expected_dllp);
    REQUIRE(cip.dllp_len == expected_dllp_len);
    REQUIRE(bytearray_equal(cip.dllp, cip.dllp_len, expected_dllp, expected_dllp_len));

    uint8_t expected_hb[] = {0x00, 0x01};
    size_t expected_hb_len = sizeof(expected_hb);
    REQUIRE(cip.hb_len == expected_hb_len);
    REQUIRE(bytearray_equal(cip.hb, cip.hb_len, expected_hb, expected_hb_len));

    t1prime_cip_destroy(&cip);
}

TEST_CASE("Decode CIP with without historical bytes", "[CIP, t1prime_cip_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x02,
        0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x04, 0x00, 0x01, 0x02, 0x03,
        0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(status == T1PRIME_CIP_DECODE_SUCCESS);
    REQUIRE(cip.version == 0x01);

    uint8_t expected_iin[] = {0x00, 0x01, 0x02, 0x03};
    size_t expected_iin_len = sizeof(expected_iin);
    REQUIRE(cip.iin_len == expected_iin_len);
    REQUIRE(bytearray_equal(cip.iin, cip.iin_len, expected_iin, expected_iin_len));

    REQUIRE(cip.plid == 0x02);

    uint8_t expected_plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    size_t expected_plp_len = sizeof(expected_plp);
    REQUIRE(cip.plp_len == expected_plp_len);
    REQUIRE(bytearray_equal(cip.plp, cip.plp_len, expected_plp, expected_plp_len));

    uint8_t expected_dllp[] = {0x00, 0x01, 0x02, 0x03};
    size_t expected_dllp_len = sizeof(expected_dllp);
    REQUIRE(cip.dllp_len == expected_dllp_len);
    REQUIRE(bytearray_equal(cip.dllp, cip.dllp_len, expected_dllp, expected_dllp_len));

    REQUIRE(cip.hb_len == 0);
    REQUIRE(cip.hb == NULL);

    t1prime_cip_destroy(&cip);
}

TEST_CASE("Decode invalid CIP with too little data overall", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x00, 0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode invalid CIP with too much data indicated by iin_len", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x01, 0x01, 0x01, 0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode invalid CIP with too much data indicated by plp_len", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x01, 0x00, 0x01, 0x01, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode invalid CIP with too much data indicated by dllp_len", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x01, 0x00, 0x01, 0x00, 0x01, 0x00};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode invalid CIP with too much data indicated by hb_len", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x01};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == INVALID_LENGTH);
}

TEST_CASE("Decode invalid CIP with too little data indicated by hb_len", "[CIP, t1prime_cip_decode, Error]")
{
    uint8_t encoded[] = {0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
    size_t encoded_len = sizeof(encoded);

    CIP cip;
    int status = t1prime_cip_decode(&cip, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == INVALID_LENGTH);
}

TEST_CASE("Validate invalid CIP with too little data in iin", "[CIP, t1prime_cip_validate, Error]")
{
    uint8_t iin[] = {0x00, 0x01};
    uint8_t plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t dllp[] = {0x00, 0x01, 0x02, 0x03};
    CIP cip = {0x01, sizeof(iin), iin, 0x02, sizeof(plp), plp, sizeof(dllp), dllp, 0, NULL};

    int status = t1prime_cip_validate(&cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_VALIDATE);
    REQUIRE(ifx_error_get_reason(status) == INVALID_LENGTH);
}

TEST_CASE("Validate invalid CIP with too much data in iin", "[CIP, t1prime_cip_validate, Error]")
{
    uint8_t iin[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    uint8_t plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t dllp[] = {0x00, 0x01, 0x02, 0x03};
    CIP cip = {0x01, sizeof(iin), iin, 0x02, sizeof(plp), plp, sizeof(dllp), dllp, 0, NULL};

    int status = t1prime_cip_validate(&cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_VALIDATE);
    REQUIRE(ifx_error_get_reason(status) == INVALID_LENGTH);
}

#ifdef INTERFACE_I2C
TEST_CASE("Validate invalid CIP with too little data in I2C plp", "[CIP, t1prime_cip_validate, Error]")
{
    uint8_t iin[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t dllp[] = {0x00, 0x01, 0x02, 0x03};
    CIP cip = {0x01, sizeof(iin), iin, PLID_I2C, sizeof(plp), plp, sizeof(dllp), dllp, 0, NULL};

    int status = t1prime_cip_validate(&cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_VALIDATE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

#else
TEST_CASE("Validate invalid CIP with too little data in SPI plp", "[CIP, t1prime_cip_validate, Error]")
{
    uint8_t iin[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a};
    uint8_t dllp[] = {0x00, 0x01, 0x02, 0x03};
    CIP cip = {0x01, sizeof(iin), iin, PLID_SPI, sizeof(plp), plp, sizeof(dllp), dllp, 0, NULL};

    int status = t1prime_cip_validate(&cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_VALIDATE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}
#endif

TEST_CASE("Validate invalid CIP with too little data in dllp", "[CIP, t1prime_cip_validate, Error]")
{
    uint8_t iin[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t plp[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t dllp[] = {0x00, 0x01, 0x02};
    CIP cip = {0x01, sizeof(iin), iin, 0x02, sizeof(plp), plp, sizeof(dllp), dllp, 0, NULL};

    int status = t1prime_cip_validate(&cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_CIP_VALIDATE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

TEST_CASE("Decode DLLP without rfu bytes", "[DLLP, t1prime_dllp_decode")
{
    uint8_t encoded[] = {
        0x01, 0x02,
        0x03, 0x04};
    size_t encoded_len = sizeof(encoded);

    DLLP dllp;
    int status = t1prime_dllp_decode(&dllp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(dllp.bwt == 0x0102);
    REQUIRE(dllp.ifsc == 0x0304);

    t1prime_dllp_destroy(&dllp);
}

TEST_CASE("Decode DLLP with rfu bytes", "[DLLP, t1prime_dllp_decode")
{
    uint8_t encoded[] = {
        0x01, 0x02,
        0x03, 0x04,
        0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    DLLP dllp;
    int status = t1prime_dllp_decode(&dllp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(dllp.bwt == 0x0102);
    REQUIRE(dllp.ifsc == 0x0304);

    t1prime_dllp_destroy(&dllp);
}

TEST_CASE("Decode invalid DLLP with too little data", "[DLLP, t1prime_dllp_decode, Error]")
{
    uint8_t encoded[] = {0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    DLLP dllp;
    int status = t1prime_dllp_decode(&dllp, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_DLLP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

#ifdef INTERFACE_I2C
TEST_CASE("Decode I2C PLP without rfu bytes", "[I2CPLP, t1prime_i2c_plp_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x02,
        0x03, 0x04,
        0x05,
        0x06,
        0x07, 0x08};
    size_t encoded_len = sizeof(encoded);

    I2CPLP plp;
    int status = t1prime_i2c_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(plp.configuration == 0x01);
    REQUIRE(plp.pwt == 0x02);
    REQUIRE(plp.mcf == 0x0304);
    REQUIRE(plp.pst == 0x05);
    REQUIRE(plp.mpot == 0x06);
    REQUIRE(plp.rwgt == 0x0708);

    t1prime_i2c_plp_destroy(&plp);
}

TEST_CASE("Decode I2C PLP with rfu bytes", "[I2CPLP, t1prime_i2c_plp_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x02,
        0x03, 0x04,
        0x05,
        0x06,
        0x07, 0x08,
        0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    I2CPLP plp;
    int status = t1prime_i2c_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(plp.configuration == 0x01);
    REQUIRE(plp.pwt == 0x02);
    REQUIRE(plp.mcf == 0x0304);
    REQUIRE(plp.pst == 0x05);
    REQUIRE(plp.mpot == 0x06);
    REQUIRE(plp.rwgt == 0x0708);

    t1prime_i2c_plp_destroy(&plp);
}

TEST_CASE("Decode invalid I2C PLP with too little data", "[I2CPLP, t1prime_i2c_plp_decode, Error]")
{
    uint8_t encoded[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    I2CPLP plp;
    int status = t1prime_i2c_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_PLP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

#else
TEST_CASE("Decode SPI PLP without rfu bytes", "[SPIPLP, t1prime_spi_plp_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x02,
        0x03, 0x04,
        0x05,
        0x06,
        0x07, 0x08,
        0x09, 0x0a,
        0x0b, 0x0c};
    size_t encoded_len = sizeof(encoded);

    SPIPLP plp;
    int status = t1prime_spi_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(plp.configuration == 0x01);
    REQUIRE(plp.pwt == 0x02);
    REQUIRE(plp.mcf == 0x0304);
    REQUIRE(plp.pst == 0x05);
    REQUIRE(plp.mpot == 0x06);
    REQUIRE(plp.segt == 0x0708);
    REQUIRE(plp.seal == 0x090a);
    REQUIRE(plp.wut == 0x0b0c);

    t1prime_spi_plp_destroy(&plp);
}

TEST_CASE("Decode SPI PLP with rfu bytes", "[SPIPLP, t1prime_spi_plp_decode")
{
    uint8_t encoded[] = {
        0x01,
        0x02,
        0x03, 0x04,
        0x05,
        0x06,
        0x07, 0x08,
        0x09, 0x0a,
        0x0b, 0x0c,
        0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    SPIPLP plp;
    int status = t1prime_spi_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(status == T1PRIME_DLLP_DECODE_SUCCESS);
    REQUIRE(plp.configuration == 0x01);
    REQUIRE(plp.pwt == 0x02);
    REQUIRE(plp.mcf == 0x0304);
    REQUIRE(plp.pst == 0x05);
    REQUIRE(plp.mpot == 0x06);
    REQUIRE(plp.segt == 0x0708);
    REQUIRE(plp.seal == 0x090a);
    REQUIRE(plp.wut == 0x0b0c);

    t1prime_spi_plp_destroy(&plp);
}

TEST_CASE("Decode invalid SPI PLP with too little data", "[SPIPLP, t1prime_spi_plp_decode, Error]")
{
    uint8_t encoded[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    SPIPLP plp;
    int status = t1prime_spi_plp_decode(&plp, encoded, encoded_len);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == T1PRIME_PLP_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}
#endif
