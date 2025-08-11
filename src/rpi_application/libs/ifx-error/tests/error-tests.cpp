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
 * \file error-tests.cpp
 * \brief Tests for Infineon specific error code creation and parsing
 */
#include "catch2/catch.hpp"
#include <cstdint>
#include "ifx/error.h"

/**
 * \test Tests creation of IFX error code
 */
TEST_CASE("create", "[IFX_ERROR]")
{
    int status_code = IFX_ERROR(0x70, 0x60, 0x50);
    REQUIRE(status_code == 0x80706050);
}

/**
 * \test Checks that error indicator is correctly detected
 */
TEST_CASE("detect error", "[is_error]")
{
    int status_code = 0x80000000;
    REQUIRE(ifx_is_error(status_code));
}

/**
 * \test Checks that missing error indicator is correctly detected
 */
TEST_CASE("detect success", "[is_error]")
{
    int status_code = 0x00000000;
    REQUIRE(!ifx_is_error(status_code));
}

/**
 * \test Tests extraction of module identifier from status code
 */
TEST_CASE("get module identifier", "[get_module]")
{
    int status_code = 0x80706050;
    REQUIRE(ifx_error_get_module(status_code) == 0x70);
}

/**
 * \test Tests extraction of function identifier from status code
 */
TEST_CASE("get function identifier", "[get_function]")
{
    int status_code = 0x80706050;
    REQUIRE(ifx_error_get_function(status_code) == 0x60);
}

/**
 * \test Tests extraction of function specific error reason from status code
 */
TEST_CASE("get function specific reason", "[get_reason]")
{
    int status_code = 0x80706050;
    REQUIRE(ifx_error_get_reason(status_code) == 0x50);
}

