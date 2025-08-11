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
 * \file apdu.c
 * \brief APDU (response) en-/decoding utility
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ifx/apdu.h"

/**
 * \brief Decodes binary data to its member representation in \ref APDU object
 *
 * \param apdu APDU object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c APDU_DECODE_SUCCESS if successful, any other value in case of error
 */
int apdu_decode(APDU *apdu, uint8_t *data, size_t data_len)
{
    // Minimum APDU length 4 bytes -> header only
    if (data_len < 4)
    {
        return IFX_ERROR(LIBAPDU, APDU_DECODE, TOO_LITTLE_DATA);
    }

    // Parse out header information
    apdu->cla = data[0];
    apdu->ins = data[1];
    apdu->p1 = data[2];
    apdu->p2 = data[3];

    // Set default values just to be sure
    apdu->lc = 0;
    apdu->data = NULL;
    apdu->le = 0;

    // Parse body information
    data += 4;
    data_len -= 4;

    // ISO7816-3: Case 1
    if (data_len == 0)
    {
        return APDU_DECODE_SUCCESS;
    }

    // ISO7816-3: Case 2S
    if (data_len == 1)
    {
        // Special case {0x00} extends to 0x100
        if (data[0] == 0x00)
        {
            apdu->le = APDU_LE_ANY;
        }
        else
        {
            apdu->le = data[0];
        }
        return APDU_DECODE_SUCCESS;
    }

    // ISO7816-3: Case 2E
    if (data_len == 3 && data[0] == 0x00)
    {
        uint16_t parsed_le = (data[1] << 8) | data[2];

        // Special case {0x00, 0x00} extends to 0x10000
        if (parsed_le == 0x0000)
        {
            apdu->le = APDU_LE_ANY_EXTENDED;
        }
        else
        {
            apdu->le = parsed_le;
        }
        return APDU_DECODE_SUCCESS;
    }

    // ISO7816-3: Case 3S or Case 4S
    bool extended_length = false;
    if (data[0] != 0x00)
    {
        apdu->lc = data[0];
        data += 1;
        data_len -= 1;
    }
    // ISO7816-3: Case 3E or Case 4E
    else
    {
        if (data_len < 3)
        {
            return IFX_ERROR(LIBAPDU, APDU_DECODE, LC_MISMATCH);
        }
        apdu->lc = (data[1] << 8) | data[2];
        data += 3;
        data_len -= 3;
        extended_length = true;
    }

    // Check if LC matches data
    if (data_len < apdu->lc)
    {
        return IFX_ERROR(LIBAPDU, APDU_DECODE, LC_MISMATCH);
    }

    // Copy data
    apdu->data = malloc(apdu->lc);
    if (apdu->data == NULL)
    {
        return IFX_ERROR(LIBAPDU, APDU_DECODE, OUT_OF_MEMORY);
    }
    memcpy(apdu->data, data, apdu->lc);
    data += apdu->lc;
    data_len -= apdu->lc;

    // ISO7816-3: Case 3S or Case 3E
    if (data_len == 0)
    {
        return APDU_DECODE_SUCCESS;
    }

    // ISO7816-3: Case 4S
    if (data_len == 1)
    {
        // ISO7816-3 Case 4S requires LC to also have short form
        if (extended_length)
        {
            free(apdu->data);
            apdu->data = NULL;
            return IFX_ERROR(LIBAPDU, APDU_DECODE, EXTENDED_LENGTH_MISMATCH);
        }

        // Special case {0x00} extends to 0x100
        if (data[0] == 0x00)
        {
            apdu->le = APDU_LE_ANY;
        }
        else
        {
            apdu->le = data[0];
        }
        return APDU_DECODE_SUCCESS;
    }

    // ISO7816-3: Case 4E
    if (data_len == 2)
    {
        // ISO7816-3 Case 4E requires LC to also have extended form
        if (!extended_length)
        {
            free(apdu->data);
            apdu->data = NULL;
            return IFX_ERROR(LIBAPDU, APDU_DECODE, EXTENDED_LENGTH_MISMATCH);
        }

        uint16_t parsed_le = (data[0] << 8) | data[1];
        // Special case {0x00, 0x00} extends to 0x10000
        if (parsed_le == 0x00)
        {
            apdu->le = APDU_LE_ANY_EXTENDED;
        }
        else
        {
            apdu->le = parsed_le;
        }
        return APDU_DECODE_SUCCESS;
    }

    // Otherwise incorrect data
    free(apdu->data);
    apdu->data = NULL;

    return IFX_ERROR(LIBAPDU, APDU_DECODE, LC_MISMATCH);
}

/**
 * \brief Encodes \ref APDU to its binary representation
 *
 * \param apdu APDU  to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c APDU_ENCODE_SUCCESS if successful, any other value in case of error
 */
int apdu_encode(APDU *apdu, uint8_t **buffer, size_t *buffer_len)
{
    // Calculate required buffer size (minimum 4 bytes for header)
    size_t buffer_size = 4 + apdu->lc;
    bool extended_length = (apdu->lc > 0xff) || (apdu->le > APDU_LE_ANY);
    if (extended_length)
    {
        // ISO7816-3 Case 3E or 4E
        if (apdu->lc > 0)
        {
            buffer_size += 3;

            // ISO7816-3 Case 4E
            if (apdu->le > 0)
            {
                buffer_size += 2;
            }
        }
        // ISO7816-3 Case 2E
        else
        {
            buffer_size += 3;
        }
    }
    else
    {
        // ISO7816-3 Case 3S or 4S
        if (apdu->lc > 0)
        {
            buffer_size += 1;
        }

        // ISO7816-3 Case 2S or 4S
        if (apdu->le > 0)
        {
            buffer_size += 1;
        }
    }

    // Allocate memory for buffer
    *buffer = malloc(buffer_size);
    if (*buffer == NULL)
    {
        return IFX_ERROR(LIBAPDU, APDU_ENCODE, OUT_OF_MEMORY);
    }

    // Encode header information
    (*buffer)[0] = apdu->cla;
    (*buffer)[1] = apdu->ins;
    (*buffer)[2] = apdu->p1;
    (*buffer)[3] = apdu->p2;

    // ISO7816-3 Case 3 or Case 4
    if (apdu->lc > 0x00)
    {
        size_t offset = 4;

        // ISO7816-3 Case 3E or Case 4E
        if (extended_length)
        {
            (*buffer)[offset] = 0x00;
            (*buffer)[offset + 1] = (apdu->lc & 0xff00) >> 8;
            (*buffer)[offset + 2] = apdu->lc & 0xff;
            offset += 3;
        }
        // ISO7816-3 Case 3S or Case 4S
        else
        {
            (*buffer)[offset] = apdu->lc & 0xff;
            offset += 1;
        }
        memcpy((*buffer) + offset, apdu->data, apdu->lc);
        offset += apdu->lc;

        // ISO7816-3 Case 4
        if (apdu->le > 0)
        {
            // ISO7816-3 Case 4E
            if (extended_length)
            {
                // Special case 0x10000 extends to {0x00, 0x00}
                if (apdu->le == APDU_LE_ANY_EXTENDED)
                {
                    (*buffer)[offset] = 0x00;
                    (*buffer)[offset + 1] = 0x00;
                }
                else
                {
                    (*buffer)[offset] = (apdu->le & 0xff00) >> 8;
                    (*buffer)[offset + 1] = apdu->le & 0xff;
                }
            }
            // ISO7816-3 Case 4S
            else
            {
                // Special case 0x100 extends to {0x00}
                if (apdu->le == APDU_LE_ANY)
                {
                    (*buffer)[offset] = 0x00;
                }
                else
                {
                    (*buffer)[offset] = apdu->le & 0xff;
                }
            }
        }
    }
    // ISO7816-3 Case 1 or Case 2
    else
    {
        // ISO7816-3 Case 2
        if (apdu->le > 0)
        {
            // ISO7816-3 Case 2E
            if (extended_length)
            {
                (*buffer)[4] = 0x00;
                // Special case 0x10000 extends to {0x00, 0x00}
                if (apdu->le == APDU_LE_ANY_EXTENDED)
                {
                    (*buffer)[5] = 0x00;
                    (*buffer)[6] = 0x00;
                }
                else
                {
                    (*buffer)[5] = (apdu->le & 0xff00) >> 8;
                    (*buffer)[6] = apdu->le & 0xff;
                }
            }
            // ISO7816-3 Case 2S
            else
            {
                // Special case 0x100 extends to {0x00}
                if (apdu->le == APDU_LE_ANY)
                {
                    (*buffer)[4] = 0x00;
                }
                else
                {
                    (*buffer)[4] = apdu->le & 0xff;
                }
            }
        }
    }

    *buffer_len = buffer_size;
    return APDU_ENCODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref APDU object (but not object itself)
 *
 * \details APDU objects may contain dynamically allocated data (e.g. \ref APDU.data).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param apdu APDU object whose data shall be freed
 */
void apdu_destroy(APDU *apdu)
{
    if ((apdu->lc > 0) && (apdu->data != NULL))
    {
        free(apdu->data);
    }
    apdu->data = NULL;
    apdu->lc = 0;
}

/**
 * \brief Decodes binary data to its member representation in \ref APDUResponse object
 *
 * \param response APDU response object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c APDURESPONSE_DECODE_SUCCESS if successful, any other value in case of error
 */
int apduresponse_decode(APDUResponse *response, uint8_t *data, size_t data_len)
{
    // Minimum APDU response length 2 bytes -> status word only
    if (data_len < 2)
    {
        return IFX_ERROR(LIBAPDU, APDURESPONSE_DECODE, TOO_LITTLE_DATA);
    }

    // Copy data
    response->len = data_len - 2;
    if (data_len > 2)
    {
        response->data = malloc(response->len);
        if (response->data == NULL)
        {
            return IFX_ERROR(LIBAPDU, APDURESPONSE_DECODE, OUT_OF_MEMORY);
        }
        memcpy(response->data, data, response->len);
    }
    else
    {
        response->data = NULL;
    }

    // Decode status word
    response->sw = (data[response->len] << 8) | data[response->len + 1];

    return APDURESPONSE_DECODE_SUCCESS;
}

/**
 * \brief Encodes \ref APDUResponse to its binary representation
 *
 * \param response APDU response to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c APDURESPONSE_ENCODE_SUCCESS if successful, any other value in case of error
 */
int apduresponse_encode(APDUResponse *response, uint8_t **buffer, size_t *buffer_len)
{
    // Allocate memory for buffer
    *buffer = malloc(response->len + 2);
    if (*buffer == NULL)
    {
        return IFX_ERROR(LIBAPDU, APDURESPONSE_ENCODE, OUT_OF_MEMORY);
    }

    // Add data
    if (response->len > 0)
    {
        memcpy(*buffer, response->data, response->len);
    }

    // Add status word
    (*buffer)[response->len] = (response->sw & 0xff00) >> 8;
    (*buffer)[response->len + 1] = response->sw & 0xff;

    *buffer_len = (response->len + 2) & 0x1ffff;
    return APDURESPONSE_ENCODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref APDUResponse object (but not object itself)
 *
 * \details APDUResponse objects will most likely be populated by \ref apduresponse_encode(APDUResponse*, uint8_t**, size_t*).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param response APDUResponse object whose data shall be freed
 */
void apduresponse_destroy(APDUResponse *response)
{
    if ((response->len > 0) && (response->data != NULL))
    {
        free(response->data);
    }
    response->data = NULL;
    response->len = 0;
}
