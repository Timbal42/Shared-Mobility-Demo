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
 * \file ifx/error.h
 * \brief Infineon specific error code creation and parsing
 */
#ifndef _IFX_ERROR_H_
#define _IFX_ERROR_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief General status code for successfuly calls to any function
 */
#define SUCCESS 0x00000000

/**
 * \brief General error indicator bitmask
 */
#define ERROR_INDICATOR 0x80000000

/**
 * \brief Function independent error reason for unspecified errors
 */
#define UNSPECIFIED_ERROR 0xff

/**
 * \brief Function independent error reason for out of memory errors
 */
#define OUT_OF_MEMORY 0xfe

/**
 * \brief Function independent error reason for illegal argument value
 */
#define ILLEGAL_ARGUMENT 0xfd

/**
 * \brief Function independent error reason if too little data available
 */
#define TOO_LITTLE_DATA 0xfc

/**
 * \brief Function independent error reason if any object is in an invalid state
 */
#define INVALID_STATE 0xfb

/**
 * \brief Function independent error reason for errors that should not occur because they should be prevented by the code
 */
#define PROGRAMMING_ERROR 0xfa

/**
 * \brief Creates IFX encoded error code for given module, function and reason
 *
 * \details IFX error codes have the following schema:
 *          bit 31      Error indicator
 *          bits 30-24  RFU
 *          bits 23-16  Module identification (e.g. libapdu)
 *          bits 15-8   Function identification in module (e.g. apdu_decode)
 *          bits 7-0    Function specific reason (e.g. apdu_decode too little data)
 *
 * \param module 8 bit module identifcator
 * \param function 8 bit function identificator
 * \param reason 8 bit function specific reason
 */
#define IFX_ERROR(module, function, reason) (ERROR_INDICATOR | (((module)&0xff) << 16) | (((function)&0xff) << 8) | ((reason)&0xff))

/**
 * \brief Checks if status code indicates error
 *
 * \param status_code Status code to be checked
 * \return bool \c true if \p status_code indicates error
 */
bool ifx_is_error(int status_code);

/**
 * \brief Extracts module identifier from error code
 *
 * \details No checks for error indicator are performed
 *
 * \param error_code Error code to extract module identifier from
 * \return uint8_t Module identifier
 */
uint8_t ifx_error_get_module(int error_code);

/**
 * \brief Extracts function identifier from error code
 *
 * \details No checks for error indicator are performed
 *
 * \param error_code Error code to extract function identifier from
 * \return uint8_t Function identifier
 */
uint8_t ifx_error_get_function(int error_code);

/**
 * \brief Extracts function specific reason from error code
 *
 * \details No checks for error indicator are performed
 *
 * \param error_code Error code to extract function specific reason from
 * \return uint8_t Function specific reason
 */
uint8_t ifx_error_get_reason(int error_code);

#ifdef __cplusplus
}
#endif

#endif // _IFX_ERROR_H_
