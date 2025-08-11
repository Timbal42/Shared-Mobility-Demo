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
 * \file ifx/apduprotocol.h
 * \brief Generic protocol API for exchanging APDUs with secure elements
 */
#ifndef _IFX_APDUPROTOCOL_H_
#define _IFX_APDUPROTOCOL_H_

#include "ifx/apdu.h"
#include "ifx/protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error encoding module identifer
 */
#define LIBAPDUPROTOCOL 0x28

/**
 * \brief Error reason if status word returned during \ref apdu_transceive(Protocol*, APDU*, APDUResponse*) indicates an error
 */
#define STATUS_WORD_ERROR 0xb0

/**
 * \brief Sends APDU to secure element reads back APDU response
 *
 * \details Encodes APDU, then sends it through ISO/OSI protocol.
 *          Reads back response and stores it in APDU response
 *
 * \param self Protocol stack for performing necessary operations
 * \param apdu APDU to be send to secure element
 * \param response Buffer to store response information in
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int apdu_transceive(Protocol *self, APDU* apdu, APDUResponse *response);

#ifdef __cplusplus
}
#endif

#endif // _IFX_APDUPROTOCOL_H_
