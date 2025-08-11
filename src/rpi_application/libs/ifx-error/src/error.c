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
 * \file error.c
 * \brief Infineon specific error code creation and parsing
 */
#include "ifx/error.h"

/**
 * \brief Checks if status code indicates error
 *
 * \param status_code Status code to be checked
 * \return bool \c true if \p status_code indicates error
 */
bool ifx_is_error(int status_code)
{
    return (status_code & ERROR_INDICATOR) != 0;
}

/**
 * \brief Extracts module identifier from error code
 *
 * \param error_code Error code to extract module identifier from
 * \return uint8_t Module identifier
 */
uint8_t ifx_error_get_module(int error_code)
{
    return (error_code & 0x00ff0000) >> 16;
}

/**
 * \brief Extracts function identifier from error code
 *
 * \param error_code Error code to extract function identifier from
 * \return uint8_t Function identifier
 */
uint8_t ifx_error_get_function(int error_code)
{
    return (error_code & 0x0000ff00) >> 8;
}

/**
 * \brief Extracts function specific reason from error code
 *
 * \param error_code Error code to extract function specific reason from
 * \return uint8_t Function specific reason
 */
uint8_t ifx_error_get_reason(int error_code)
{
    return error_code & 0x000000ff;
}
