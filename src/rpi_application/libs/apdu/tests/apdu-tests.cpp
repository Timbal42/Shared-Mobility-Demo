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
 * \file apdu-tests.cpp
 * \brief Tests for APDU de- / encoding
 */
#include "catch2/catch.hpp"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "ifx/apdu.h"

using namespace std;

/**
 * \test Tests encoding of ISO7816-3 case 1 APDU
 */
TEST_CASE("APDU encode case 1", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    APDU apdu = {cla, ins, p1, p2, 0, NULL, 0};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;
    int status = apdu_encode(&apdu, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 0 + 0 + 0;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + encoded_len) == vector<uint8_t>{cla, ins, p1, p2});

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 2S APDU
 */
TEST_CASE("APDU encode case 2S", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t le = 0x04;
    APDU apdu = {cla, ins, p1, p2, 0, NULL, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;

    // Verify return value
    int status = apdu_encode(&apdu, &encoded, &encoded_len);
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 0 + 0 + 1;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + encoded_len) == vector<uint8_t>{cla, ins, p1, p2, (uint8_t) (le & 0xff)});

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 2S APDU (special case LE = 0x100)
 */
TEST_CASE("APDU encode case 2S (LE = 0x100)", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t le = 0x100;
    APDU apdu = {cla, ins, p1, p2, 0, NULL, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;

    // Verify return value
    int status = apdu_encode(&apdu, &encoded, &encoded_len);
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 0 + 0 + 1;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + encoded_len) == vector<uint8_t>{cla, ins, p1, p2, 0x00});

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 2E APDU
 */
TEST_CASE("APDU encode case 2E", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t le1 = 0x01;
    uint8_t le2 = 0x02;
    size_t le = (le1 << 8) | le2;
    APDU apdu = {cla, ins, p1, p2, 0, NULL, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;

    // Verify return value
    int status = apdu_encode(&apdu, &encoded, &encoded_len);
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 0 + 0 + 3;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + encoded_len) == vector<uint8_t>{cla, ins, p1, p2, 0x00, le1, le2});

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 2E APDU (special case LE = 0x10000)
 */
TEST_CASE("APDU encode case 2E (LE = 0x10000)", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t le = 0x10000;
    APDU apdu = {cla, ins, p1, p2, 0, NULL, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;

    // Verify return value
    int status = apdu_encode(&apdu, &encoded, &encoded_len);
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 0 + 0 + 3;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + encoded_len) == vector<uint8_t>{cla, ins, p1, p2, 0x00, 0x00, 0x00});

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 3S APDU
 */
TEST_CASE("APDU encode case 3S", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t data[] = {0x05, 0x06, 0x07, 0x08};
    uint8_t lc = sizeof(data);
    uint8_t le = 0x00;
    APDU apdu = {cla, ins, p1, p2, lc, data, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;
    int status = apdu_encode(&apdu, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 1 + lc + 0;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(encoded[0] == cla);
    REQUIRE(encoded[1] == ins);
    REQUIRE(encoded[2] == p1);
    REQUIRE(encoded[3] == p2);
    REQUIRE(encoded[4] == lc);
    REQUIRE(vector<uint8_t>(encoded + 4 + 1, encoded + 4 + 1 + lc) == vector<uint8_t>(data, data + lc));

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 3E APDU
 */
TEST_CASE("APDU encode case 3E", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t lc1 = 0x01;
    uint8_t lc2 = 0x08;
    size_t lc = (lc1 << 8) | lc2;
    uint8_t *data = (uint8_t *)calloc(lc, sizeof(uint8_t));
    REQUIRE(data != NULL);
    APDU apdu = {cla, ins, p1, p2, lc, data, 0};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;

    // Verify return value
    int status = apdu_encode(&apdu, &encoded, &encoded_len);
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 3 + lc + 0;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(encoded[0] == cla);
    REQUIRE(encoded[1] == ins);
    REQUIRE(encoded[2] == p1);
    REQUIRE(encoded[3] == p2);
    REQUIRE(encoded[4] == 0x00);
    REQUIRE(encoded[5] == lc1);
    REQUIRE(encoded[6] == lc2);
    REQUIRE(vector<uint8_t>(encoded + 4 + 3, encoded + 4 + 3 + lc) == vector<uint8_t>(data, data + lc));

    // Cleanup
    apdu_destroy(&apdu);
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 4S APDU
 */
TEST_CASE("APDU encode case 4S", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t data[] = {0x05, 0x06, 0x07, 0x08};
    uint8_t lc = sizeof(data);
    uint8_t le = 0x09;
    APDU apdu = {cla, ins, p1, p2, lc, data, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;
    int status = apdu_encode(&apdu, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 1 + lc + 1;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(encoded[0] == cla);
    REQUIRE(encoded[1] == ins);
    REQUIRE(encoded[2] == p1);
    REQUIRE(encoded[3] == p2);
    REQUIRE(encoded[4] == lc);
    REQUIRE(vector<uint8_t>(encoded + 4 + 1, encoded + 4 + 1 + lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(encoded[9] == le);

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of ISO7816-3 case 4E APDU
 */
TEST_CASE("APDU encode case 4E", "[APDU, apdu_encode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t lc1 = 0x01;
    uint8_t lc2 = 0x08;
    size_t lc = (lc1 << 8) | lc2;
    uint8_t *data = (uint8_t *)calloc(lc, sizeof(uint8_t));
    REQUIRE(data != NULL);
    uint8_t le1 = 0x03;
    uint8_t le2 = 0x04;
    size_t le = (le1 << 8) | le2;
    APDU apdu = {cla, ins, p1, p2, lc, data, le};

    // Encode APDU
    uint8_t *encoded;
    size_t encoded_len;
    int status = apdu_encode(&apdu, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = 4 + 3 + lc + 2;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(encoded[0] == cla);
    REQUIRE(encoded[1] == ins);
    REQUIRE(encoded[2] == p1);
    REQUIRE(encoded[3] == p2);
    REQUIRE(encoded[4] == 0x00);
    REQUIRE(encoded[5] == lc1);
    REQUIRE(encoded[6] == lc2);
    REQUIRE(vector<uint8_t>(encoded + 4 + 3, encoded + 4 + 3 + lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(encoded[encoded_len - 2] == le1);
    REQUIRE(encoded[encoded_len - 1] == le2);

    // Cleanup
    apdu_destroy(&apdu);
    free(encoded);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 1 APDU data
 */
TEST_CASE("APDU decode case 1", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t encoded_len = 4 + 0 + 0 + 0;
    uint8_t encoded[] = {cla, ins, p1, p2};

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == 0);
    REQUIRE(apdu.data == NULL);
    REQUIRE(apdu.le == 0);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 2S APDU data
 */
TEST_CASE("APDU decode case 2S", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t le = 0x04;
    uint8_t encoded[] = {cla, ins, p1, p2, le};
    size_t encoded_len = sizeof(encoded);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == 0);
    REQUIRE(apdu.data == NULL);
    REQUIRE(apdu.le == le);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 2S APDU data (special case LE = {0x00})
 */
TEST_CASE("APDU decode case 2S (LE = {0x00})", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t le = 0x00;
    uint8_t encoded[] = {cla, ins, p1, p2, le};
    size_t encoded_len = sizeof(encoded);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == 0);
    REQUIRE(apdu.data == NULL);
    REQUIRE(apdu.le == 0x100);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 2E APDU data
 */
TEST_CASE("APDU decode case 2E", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t le = 0x0102;
    uint8_t encoded[] = {cla, ins, p1, p2, 0x00, (uint8_t) ((le & 0xff00) >> 8), (uint8_t) (le & 0xff)};
    size_t encoded_len = sizeof(encoded);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == 0);
    REQUIRE(apdu.data == NULL);
    REQUIRE(apdu.le == le);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 2E APDU data (special case LE = {0x00, 0x00})
 */
TEST_CASE("APDU decode case 2E (LE = {0x00 0x00})", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t le = 0x10000;
    uint8_t encoded[] = {cla, ins, p1, p2, 0x00, 0x00, 0x00};
    size_t encoded_len = sizeof(encoded);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == 0);
    REQUIRE(apdu.data == NULL);
    REQUIRE(apdu.le == le);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 3S APDU data
 */
TEST_CASE("APDU case 3S", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t data[] = {0x04, 0x05, 0x06, 0x07};
    uint8_t lc = sizeof(data);
    size_t encoded_len = 4 + 1 + lc + 0;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len * sizeof(uint8_t));
    REQUIRE(encoded != NULL);
    encoded[0] = cla;
    encoded[1] = ins;
    encoded[2] = p1;
    encoded[3] = p2;
    encoded[4] = lc;
    memcpy(encoded + 4 + 1, data, lc);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == lc);
    REQUIRE(vector<uint8_t>(apdu.data, apdu.data + apdu.lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(apdu.le == 0);

    // Cleanup
    apdu_destroy(&apdu);
    free(encoded);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 3E APDU data
 */
TEST_CASE("APDU case 3E", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t lc = 0x0108;
    uint8_t *data = (uint8_t *)calloc(lc, sizeof(uint8_t));
    REQUIRE(data != NULL);
    size_t encoded_len = 4 + 3 + lc + 0;
    size_t alloc_len = encoded_len + ((8 - (encoded_len % 8)) % 8); // Workaround to malloc issue with size % 8 != 0
    uint8_t *encoded = (uint8_t *)malloc(alloc_len);
    REQUIRE(encoded != NULL);
    encoded[0] = cla;
    encoded[1] = ins;
    encoded[2] = p1;
    encoded[3] = p2;
    encoded[4] = 0x00;
    encoded[5] = (lc & 0xff00) >> 8;
    encoded[6] = lc & 0xff;
    memcpy(encoded + 4 + 3, data, lc);

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == lc);
    REQUIRE(vector<uint8_t>(apdu.data, apdu.data + apdu.lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(apdu.le == 0);

    // Cleanup
    free(data);
    apdu_destroy(&apdu);
    free(encoded);
}


/**
 * \test Tests decoding of binary ISO7816-3 case 4S APDU data
 */
TEST_CASE("APDU decode case 4S", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    uint8_t data[] = {0x05, 0x06, 0x07, 0x08};
    uint8_t lc = sizeof(data);
    uint8_t le = 0x09;
    size_t encoded_len = 4 + 1 + lc + 1;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len * sizeof(uint8_t));
    REQUIRE(encoded != NULL);
    encoded[0] = cla;
    encoded[1] = ins;
    encoded[2] = p1;
    encoded[3] = p2;
    encoded[4] = lc;
    memcpy(encoded + 4 + 1, data, lc);
    encoded[encoded_len - 1] = le;

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == lc);
    REQUIRE(vector<uint8_t>(apdu.data, apdu.data + apdu.lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(apdu.le == le);

    // Cleanup
    apdu_destroy(&apdu);
    free(encoded);
}

/**
 * \test Tests decoding of binary ISO7816-3 case 4E APDU data
 */
TEST_CASE("APDU decode case 4E", "[APDU, apdu_decode]")
{
    // Prepare data
    uint8_t cla = 0x00;
    uint8_t ins = 0x01;
    uint8_t p1 = 0x02;
    uint8_t p2 = 0x03;
    size_t lc = 0x0108;
    uint8_t *data = (uint8_t *)calloc(lc, sizeof(uint8_t));
    REQUIRE(data != NULL);
    size_t le = 0x0304;
    size_t encoded_len = 4 + 3 + lc + 2;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len * sizeof(uint8_t));
    REQUIRE(encoded != NULL);
    encoded[0] = cla;
    encoded[1] = ins;
    encoded[2] = p1;
    encoded[3] = p2;
    encoded[4] = 0x00;
    encoded[5] = (lc & 0xff00) >> 8;
    encoded[6] = lc & 0xff;
    memcpy(encoded + 4 + 3, data, lc);
    encoded[encoded_len - 2] = (le & 0xff00) >> 8;
    encoded[encoded_len - 1] = le & 0xff;

    // Decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDU_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(apdu.cla == cla);
    REQUIRE(apdu.ins == ins);
    REQUIRE(apdu.p1 == p1);
    REQUIRE(apdu.p2 == p2);
    REQUIRE(apdu.lc == lc);
    REQUIRE(vector<uint8_t>(apdu.data, apdu.data + apdu.lc) == vector<uint8_t>(data, data + lc));
    REQUIRE(apdu.le == le);

    // Cleanup
    free(data);
    apdu_destroy(&apdu);
    free(encoded);
}

/**
 * \test Tests unsuccessful decoding of binary APDU data with too few bytes
 */
TEST_CASE("APDU decode too little data", "[APDU, apdu_decode, Error]")
{
    // Prepare data
    uint8_t encoded[] = {0x01, 0x02, 0x03};
    size_t encoded_len = sizeof(encoded);

    // Try to decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify error code
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBAPDU);
    REQUIRE(ifx_error_get_function(status) == APDU_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}

/**
 * \test Tests unsuccessful decoding of binary APDU data with invalid LC (too little data)
 */
TEST_CASE("APDU decode LC mismatch too little data", "[APDU, apdu_decode, Error]")
{
    // Prepare data
    uint8_t encoded[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x00};
    size_t encoded_len = sizeof(encoded);

    // Try to decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify error code
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBAPDU);
    REQUIRE(ifx_error_get_function(status) == APDU_DECODE);
    REQUIRE(ifx_error_get_reason(status) == LC_MISMATCH);
}

/**
 * \test Tests unsuccessful decoding of binary APDU data with invalid LC (too much data)
 */
TEST_CASE("APDU decode LC mismatch too much data", "[APDU, apdu_decode, Error]")
{
    // Prepare data
    uint8_t encoded[] = {0x01, 0x02, 0x03, 0x04, 0x01, 0x00, 0x01, 0xff, 0xff};
    size_t encoded_len = sizeof(encoded);

    // Try to decode APDU
    APDU apdu;
    int status = apdu_decode(&apdu, encoded, encoded_len);

    // Verify error code
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBAPDU);
    REQUIRE(ifx_error_get_function(status) == APDU_DECODE);
    REQUIRE(ifx_error_get_reason(status) == LC_MISMATCH);
}

/**
 * \test Tests encoding of APDUResponse with data
 */
TEST_CASE("APDU response encode", "[APDUResponse, apduresponse_encode]")
{
    // Prepare data
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    size_t data_len = sizeof(data);
    uint8_t sw1 = 0x05;
    uint8_t sw2 = 0x06;
    uint16_t sw = (sw1 << 8) | sw2;
    APDUResponse response = {data, data_len, sw};

    // Encode APDU response
    uint8_t *encoded;
    size_t encoded_len;
    int status = apduresponse_encode(&response, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDURESPONSE_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = data_len + 2;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(vector<uint8_t>(encoded, encoded + data_len) == vector<uint8_t>(data, data + data_len));
    REQUIRE(encoded[4] == sw1);
    REQUIRE(encoded[5] == sw2);

    // Cleanup
    free(encoded);
}

/**
 * \test Tests encoding of APDUResponse without data
 */
TEST_CASE("APDU response encode no data", "[APDUResponse, apduresponse_encode]")
{
    // Prepare data
    uint8_t *data = NULL;
    size_t data_len = 0;
    uint8_t sw1 = 0x01;
    uint8_t sw2 = 0x02;
    uint16_t sw = (sw1 << 8) | sw2;
    APDUResponse response = {data, data_len, sw};

    // Encode APDU response
    uint8_t *encoded;
    size_t encoded_len;
    int status = apduresponse_encode(&response, &encoded, &encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDURESPONSE_ENCODE_SUCCESS);

    // Verify actual data
    size_t expected_len = data_len + 2;
    REQUIRE(encoded_len == expected_len);
    REQUIRE(encoded != NULL);
    REQUIRE(encoded[0] == sw1);
    REQUIRE(encoded[1] == sw2);

    // Cleanup
    free(encoded);
}

/**
 * \test Tests decoding of binary APDU response with data
 */
TEST_CASE("APDU response decode", "[APDUResponse, apduresponse_decode]")
{
    // Prepare data
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    size_t len = sizeof(data);
    uint8_t sw1 = 0x05;
    uint8_t sw2 = 0x06;
    uint16_t sw = (sw1 << 8) | sw2;
    size_t encoded_len = len + 2;
    uint8_t *encoded = (uint8_t *)malloc(encoded_len * sizeof(uint8_t));
    REQUIRE(encoded != NULL);
    memcpy(encoded, data, len);
    encoded[encoded_len - 2] = sw1;
    encoded[encoded_len - 1] = sw2;

    // Decode APDU response
    APDUResponse response;
    int status = apduresponse_decode(&response, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDURESPONSE_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(vector<uint8_t>(response.data, response.data + response.len) == vector<uint8_t>(data, data + len));
    REQUIRE(response.sw == sw);

    // Cleanup
    apduresponse_destroy(&response);
    free(encoded);
}

/**
 * \test Tests decoding of binary APDU response without data
 */
TEST_CASE("APDU response decode minimal", "[APDUResponse, apduresponse_decode]")
{
    // Prepare data
    uint8_t sw1 = 0x01;
    uint8_t sw2 = 0x02;
    uint16_t sw = (sw1 << 8) | sw2;
    size_t encoded_len = 2;
    uint8_t encoded[] = {sw1, sw2};

    // Decode APDU response
    APDUResponse response;
    int status = apduresponse_decode(&response, encoded, encoded_len);

    // Verify return value
    REQUIRE(!ifx_is_error(status));
    REQUIRE(status == APDURESPONSE_DECODE_SUCCESS);

    // Verify actual data
    REQUIRE(response.data == NULL);
    REQUIRE(response.len == 0);
    REQUIRE(response.sw == sw);
}

/**
 * \test Tests unsuccessful decoding of binary APDU response with too few bytes
 */
TEST_CASE("APDU response decode too little data", "[APDUResponse, apduresponse_decode, Error]")
{
    // Prepare data
    uint8_t encoded[] = {0x01};
    size_t encoded_len = sizeof(encoded);

    // Try to decode APDU response
    APDUResponse response;
    int status = apduresponse_decode(&response, encoded, encoded_len);

    // Verify error code
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBAPDU);
    REQUIRE(ifx_error_get_function(status) == APDURESPONSE_DECODE);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);
}
