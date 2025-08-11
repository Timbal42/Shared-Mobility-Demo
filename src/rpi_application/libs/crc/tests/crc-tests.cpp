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
 * \file crc-tests.cpp
 * \brief Tests for CRC calculations
 */
#include "catch2/catch.hpp"
#include "ifx/crc.h"

/**
 * \test Tests CCITT x.25 CRC calculation
 */
TEST_CASE("CRC16 CCITT x.25", "[crc16_ccitt_x25]")
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    size_t data_len = sizeof(data);

    uint16_t expected = 0x3991;
    uint16_t actual = crc16_ccitt_x25(data, data_len);

    REQUIRE(actual == expected);
}

/**
 * \test Tests CCITT x.25 CRC calculation for empty input
 */
TEST_CASE("CRC16 CCITT x.25 no data", "[crc16_ccitt_x25]")
{
    uint8_t *data = NULL;
    size_t data_len = 0;

    uint16_t expected = 0x0000;
    uint16_t actual = crc16_ccitt_x25(data, data_len);

    REQUIRE(actual == expected);
}

/**
 * \test Tests MCRF4xx CRC calculation
 */
TEST_CASE("CRC16 MCRF4xx", "[crc16_mcrf4xx]")
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    size_t data_len = sizeof(data);

    uint16_t expected = 0xc66e;
    uint16_t actual = crc16_mcrf4xx(data, data_len);

    REQUIRE(actual == expected);
}

/**
 * \test Tests MCRF4xx CRC calculation for empty input
 */
TEST_CASE("CRC16 MCRF4xx no data", "[crc16_mcrf4xx]")
{
    uint8_t *data = NULL;
    size_t data_len = 0;

    uint16_t expected = 0xffff;
    uint16_t actual = crc16_mcrf4xx(data, data_len);

    REQUIRE(actual == expected);
}

/**
 * \test Tests LRC calculation
 */
TEST_CASE("LRC", "[lrc8]")
{
    uint8_t data[] = {0x01, 0x02, 0x04, 0x08};
    size_t data_len = sizeof(data);

    uint8_t expected = 0x0f;
    uint8_t actual = lrc8(data, data_len);

    REQUIRE(actual == expected);
}

/**
 * \test Tests LRC calculation for empty input
 */
TEST_CASE("LRC no data", "[lrc8]")
{
    uint8_t *data = NULL;
    size_t data_len = 0;

    uint8_t expected = 0x00;
    uint8_t actual = lrc8(data, data_len);

    REQUIRE(actual == expected);
}

TEST_CASE("CRC16 G+D T=1", "[crc16_t1gd]")
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    size_t data_len = sizeof(data);

    uint16_t expected = 0x0610;
    uint16_t actual = crc16_t1gd(data, data_len);

    REQUIRE(actual == expected);
}

TEST_CASE("CRC16 G+D T=1 no data", "[crc16_t1gd]")
{
    uint8_t *data = NULL;
    size_t data_len = 0;

    uint16_t expected = 0xffff;
    uint16_t actual = crc16_t1gd(data, data_len);

    REQUIRE(actual == expected);
}
