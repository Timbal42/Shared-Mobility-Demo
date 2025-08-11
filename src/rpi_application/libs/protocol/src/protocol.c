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
 * \file protocol.c
 * \brief Generic protocol API (ISO/OSI stack)
 */
#include <stdio.h>
#include <stdlib.h>
#include "ifx/protocol.h"

/**
 * \brief Activates secure element and performs protocol negotiation
 *
 * \details Depending on the protocol data needs to be exchanged with a secure element to negotiate certain protocol aspects like frame sizes, waiting times, etc.
 *
 * \param self Protocol stack for performing necessary operations
 * \param response Buffer to store response in (e.g. ATR, ATPO, ...)
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response )
 * \return int \c PROTOCOL_ACTIVATE_SUCCESS if successful, any other value in case of error
 */
int protocol_activate(Protocol *self, uint8_t **response, size_t *response_len)
{
    // Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBPROTOCOL, PROTOCOL_ACTIVATE, INVALID_PROTOCOLSTACK);
    }

    // Check if current layer has activation function
    if (self->_activate != NULL)
    {
        return self->_activate(self, response, response_len);
    }

    // Otherwise try next layer
    return protocol_activate(self->_base, response, response_len);
}

/**
 * \brief Sends data via protocol and reads back response
 *
 * \details Goes through ISO/OSI protocol stack and performs necessary protocol operations (chaining, crc, ...)
 *
 * \param self Protocol stack for performing necessary operations
 * \param data Data to be send via protocol
 * \param data_len Number of bytes in \p data
 * \param response Buffer to store response in
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response )
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int protocol_transceive(Protocol *self, uint8_t *data, size_t data_len, uint8_t **response, size_t *response_len)
{
    // Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBPROTOCOL, PROTOCOL_TRANSCEIVE, INVALID_PROTOCOLSTACK);
    }
    if ((data == NULL) || (response == NULL) || (response_len == NULL))
    {
        return IFX_ERROR(LIBPROTOCOL, PROTOCOL_TRANSCEIVE, ILLEGAL_ARGUMENT);
    }

    // If protocol defines transceive function then directly use it
    if (self->_transceive != NULL)
    {
        return self->_transceive(self, data, data_len, response, response_len);
    }
    // Otherwise fall back to transmit / receive
    else
    {
        if ((self->_transmit) == NULL || (self->_receive == NULL))
        {
            return IFX_ERROR(LIBPROTOCOL, PROTOCOL_TRANSCEIVE, INVALID_PROTOCOLSTACK);
        }
        int status = self->_transmit(self, data, data_len);
        if (status != PROTOCOL_TRANSMIT_SUCCESS)
        {
            return status;
        }
        return self->_receive(self, PROTOCOL_RECEIVE_LENGTH_UNKOWN, response, response_len);
    }
}

/**
 * \brief Frees memory associated with \ref Protocol object (but not object itself)
 *
 * \details Protocol objects can contain of several layers each of which might hold dynamically allocated data.
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param self Protocol object whose data shall be freed
 */
void protocol_destroy(Protocol *self)
{
    if (self != NULL)
    {
        // Check if layer has custom cleanup function
        if (self->_destructor != NULL)
        {
            self->_destructor(self);
        }

        // Check if properties have been missed by protocol layer
        if (self->_properties != NULL)
        {
            free(self->_properties);
            self->_properties = NULL;
        }

        // Go down protocol stack
        if (self->_base != NULL)
        {
            protocol_destroy(self->_base);
        }
    }
}


/**
 * \brief Sets logger to be used by protocol
 *
 * \details Sets logger for whole protocol stack, so all layers below will also have the logger set.
 *
 * \param self \ref Protocol object to set logger for
 * \param logger \ref Logger object to be used (might be \c NULL to clear logger)
 */
void protocol_set_logger(Protocol *self, Logger *logger)
{
    if (self != NULL)
    {
        // Set logger for current layer
        self->_logger = logger;

        // Go down protocol stack
        protocol_set_logger(self->_base, logger);
    }
}

/**
 * \brief Initializes \ref Protocol object by setting all members to valid values
 *
 * \param self Protocol object to be initialized
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error
 */
int protocollayer_initialize(Protocol *self)
{
    self->_base = NULL;
    self->_layer_id = 0;
    self->_activate = NULL;
    self->_transceive = NULL;
    self->_transmit = NULL;
    self->_receive = NULL;
    self->_destructor = NULL;
    self->_logger = NULL;
    self->_properties = NULL;
    return PROTOCOLLAYER_INITIALIZE_SUCCESS;
}
