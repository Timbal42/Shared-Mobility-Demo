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
 * \file gp_general_errors.c
 * \brief Global Platform APDU error code lookup
 */
#include "gp_general_errors.h"
#include "ifx/error.h"

uint8_t gp_general_errors_from_statusword(uint16_t sw)
{
    switch(sw)
    {
        case 0x6400:
            return GP_ERROR_NO_SPECIFIC_DIAGNOSIS;
        case 0x6700:
            return GP_ERROR_WRONG_LENGTH_IN_LC;
        case 0x6881:
            return GP_ERROR_LOGICAL_CHANNEL_NOT_SUPPORTED_OR_NOT_ACTIVE;
        case 0x6982:
            return GP_ERROR_SECURITY_STATUS_NOT_SATISFIED;
        case 0x6985:
            return GP_ERROR_CONDITIONS_OF_USE_NOT_SATISFIED;
        case 0x6A86:
            return GP_ERROR_INCORRECT_P1_P2;
        case 0x6D00:
            return GP_ERROR_INVALID_INSTRUCTION;
        case 0x6E00:
            return GP_ERROR_INVALID_CLASS;
        default:
            return UNSPECIFIED_ERROR;
    }
}
