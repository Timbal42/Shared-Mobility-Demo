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
 * \file gp_general_errors.h
 * \brief Global Platform APDU error code lookup
 */
#ifndef _IFX_GP_GENERAL_ERRORS_H_
#define _IFX_GP_GENERAL_ERRORS_H_

#define GP_ERROR_NO_SPECIFIC_DIAGNOSIS 0x88
#define GP_ERROR_WRONG_LENGTH_IN_LC 0x89
#define GP_ERROR_LOGICAL_CHANNEL_NOT_SUPPORTED_OR_NOT_ACTIVE 0x8A
#define GP_ERROR_SECURITY_STATUS_NOT_SATISFIED 0x8B
#define GP_ERROR_CONDITIONS_OF_USE_NOT_SATISFIED 0x8C
#define GP_ERROR_INCORRECT_P1_P2 0x8D
#define GP_ERROR_INVALID_INSTRUCTION 0x8E
#define GP_ERROR_INVALID_CLASS 0x8F

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t gp_general_errors_from_statusword(uint16_t sw);

#ifdef __cplusplus
}
#endif

#endif
