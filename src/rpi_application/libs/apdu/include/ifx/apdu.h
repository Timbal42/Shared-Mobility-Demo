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
 * \file ifx/apdu.h
 * \brief APDU (response) en-/decoding utility
 */
#ifndef _IFX_APDU_H_
#define _IFX_APDU_H_

#include <stddef.h>
#include <stdint.h>
#include "ifx/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error encoding module identifer
 */
#define LIBAPDU 0x10

/**@struct APDU
 * \brief Data storage for APDU fields
 */
typedef struct
{
   /**
    * \brief APDU instruction class
    */
   uint8_t cla;

   /**
    * \brief APDU instruction code
    */
   uint8_t ins;

   /**
    * \brief First APDU instruction parameter byte
    */
   uint8_t p1;

   /**
    * \brief Second APDU intruction parameter byte
    */
   uint8_t p2;

   /**
    * \brief Length of \ref APDU.data
    */
   size_t lc;

   /**
    * \brief Actual APDU content data (might be \c NULL )
    */
   uint8_t *data;

   /**
    * \brief Expected number of bytes in response
    */
   size_t le;
} APDU;

/**
 * \brief LE value for expecting any number of bytes <= 256
 *
 * \details The short length APDU LE encoding according to ISO7816-3 Case 2S or Case 4S is a single byte `0x00` meaning 256 bytes
 */
#define APDU_LE_ANY 0x100

/**
 * \brief LE value for expecting any number of bytes <= 65536
 *
 * \details The extended length APDU LE encoding according to ISO7816-3 Case 2E or Case 4E is a 2 bytes `{0x00, 0x00}` meaning 65536 bytes
 */
#define APDU_LE_ANY_EXTENDED 0x10000

/**
 * \brief IFX error encoding function identifier for \ref apdu_decode(APDU*, uint8_t*, size_t)
 */
#define APDU_DECODE 0x01

/**
 * \brief Return code for successful calls to \ref apdu_decode(APDU*, uint8_t*, size_t)
 */
#define APDU_DECODE_SUCCESS SUCCESS

/**
 * \brief Error reason if LC does not match length of data in \ref apdu_decode(APDU*, uint8_t*, size_t)
 */
#define LC_MISMATCH 0x01

/**
 * \brief Error reason if LC and LE do not use same form (short / extended) length in \ref apdu_decode(APDU*, uint8_t*, size_t)
 */
#define EXTENDED_LENGTH_MISMATCH 0x02

/**
 * \brief Decodes binary data to its member representation in \ref APDU object
 *
 * \param apdu APDU object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c APDU_DECODE_SUCCESS if successful, any other value in case of error
 */
int apdu_decode(APDU *apdu, uint8_t *data, size_t data_len);

/**
 * \brief IFX error encoding function identifier for \ref apdu_encode(APDU*, uint8_t**, size_t*)
 */
#define APDU_ENCODE 0x02

/**
 * \brief Return code for successful calls to \ref apdu_encode(APDU*, uint8_t**, size_t*)
 */
#define APDU_ENCODE_SUCCESS SUCCESS

/**
 * \brief Encodes \ref APDU to its binary representation
 *
 * \param apdu APDU  to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c APDU_ENCODE_SUCCESS if successful, any other value in case of error
 */
int apdu_encode(APDU *apdu, uint8_t **buffer, size_t *buffer_len);

/**
 * \brief Frees memory associated with \ref APDU object (but not object itself)
 *
 * \details APDU objects may contain dynamically allocated data (e.g. \ref APDU.data).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param apdu APDU object whose data shall be freed
 */
void apdu_destroy(APDU *apdu);

/**@struct APDUResponse
 * \brief Data storage struct for APDU response information
 */
typedef struct
{
   /**
    * \brief Actual response data (might be \c NULL )
    */
   uint8_t *data;

   /**
    * \brief Number of bytes in \ref APDUResponse.data
    */
   size_t len;

   /**
    * \brief APDU response status word
    */
   uint16_t sw;
} APDUResponse;

/**
 * \brief IFX error encoding function identifier for \ref apduresponse_decode(APDUResponse*, uint8_t*, size_t)
 */
#define APDURESPONSE_DECODE 0x03

/**
 * \brief Return code for successful calls to \ref apduresponse_decode(APDUResponse*, uint8_t*, size_t)
 */
#define APDURESPONSE_DECODE_SUCCESS SUCCESS

/**
 * \brief Decodes binary data to its member representation in \ref APDUResponse object
 *
 * \param response APDU response object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c APDURESPONSE_DECODE_SUCCESS if successful, any other value in case of error
 */
int apduresponse_decode(APDUResponse *response, uint8_t *data, size_t data_len);

/**
 * \brief IFX error encoding function identifier for \ref apduresponse_encode(APDUResponse*, uint8_t**, size_t*)
 */
#define APDURESPONSE_ENCODE 0x04

/**
 * \brief Return code for successful calls to \ref apduresponse_encode(APDUResponse*, uint8_t**, size_t*)
 */
#define APDURESPONSE_ENCODE_SUCCESS SUCCESS

/**
 * \brief Encodes \ref APDUResponse to its binary representation
 *
 * \param response APDU response to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c APDURESPONSE_ENCODE_SUCCESS if successful, any other value in case of error
 */
int apduresponse_encode(APDUResponse *response, uint8_t **buffer, size_t *buffer_len);

/**
 * \brief Frees memory associated with \ref APDUResponse object (but not object itself)
 *
 * \details APDUResponse objects will most likely be populated by \ref apduresponse_encode(APDUResponse*, uint8_t**, size_t*).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param response APDUResponse object whose data shall be freed
 */
void apduresponse_destroy(APDUResponse *response);

#ifdef __cplusplus
}
#endif

#endif // _IFX_APDU_H_
