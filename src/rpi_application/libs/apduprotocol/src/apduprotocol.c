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
 * \file apduprotocol.c
 * \brief Generic protocol API for exchanging APDUs with secure elements
 */
#include <stdlib.h>
#include "ifx/protocol.h"
#include "ifx/apduprotocol.h"
#include "ifx/logger.h"

/**
 * \brief String used as source information for logging
 */
#define LOG_TAG "APDU"

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
int apdu_transceive(Protocol *self, APDU* apdu, APDUResponse *response)
{
    // Encode APDU
    uint8_t *encoded = NULL;
    size_t encoded_len = 0;
    int status = apdu_encode(apdu, &encoded, &encoded_len);
    if (status != APDU_ENCODE_SUCCESS)
    {
        return status;
    }

    // Log transmitted data
    logger_log_bytearray(self->_logger, LOG_TAG, LOG_INFO, ">> ", encoded, encoded_len, " ");

    // Exchange data with secure element
    uint8_t *response_buffer = NULL;
    size_t response_len = 0;
    status = protocol_transceive(self, encoded, encoded_len, &response_buffer, &response_len);

    if ((encoded != NULL) && (encoded_len > 0))
    {
        free(encoded);
        encoded = NULL;
    }
    if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    // Log received data
    logger_log_bytearray(self->_logger, LOG_TAG, LOG_INFO, "<< ", response_buffer, response_len, " ");

    // Decode APDU response
    status = apduresponse_decode(response, response_buffer, response_len);

    if ((response_buffer != NULL) && (response_len > 0))
    {
        free(response_buffer);
        response_buffer = NULL;
    }
    if (status != APDURESPONSE_DECODE_SUCCESS)
    {
        return status;
    }
    return PROTOCOL_TRANSCEIVE_SUCCESS;
}
