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
 * \file ifx/protocol.h
 * \brief Generic protocol API (ISO/OSI stack)
 */
#ifndef _IFX_PROTOCOL_H_
#define _IFX_PROTOCOL_H_

#include <stdint.h>
#include <stddef.h>
#include "ifx/error.h"
#include "ifx/logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error encoding  module identifer
 */
#define LIBPROTOCOL 0x20

/**
 * \brief Function independent error reason for invalid protocol stack (missing required function).
 */
#define INVALID_PROTOCOLSTACK 0x8f

typedef struct Protocol Protocol;

/**
 * \brief IFX error encoding function identifier for \ref protocol_activate(Protocol*, uint8_t**, size_t*) and \ref protocol_activatefunction_t
 */
#define PROTOCOL_ACTIVATE 0x81

/**
 * \brief Return code for successful calls to \ref protocol_activate(Protocol*, uint8_t**, size_t*) and \ref protocol_activatefunction_t
 */
#define PROTOCOL_ACTIVATE_SUCCESS SUCCESS

/**
 * \brief Protocol layer specific secure element activation function
 *
 * \details Depending on the protocol data needs to be exchanged with a secure element to negotiate certain protocol aspects like frame sizes, waiting times, etc.
 *
 * \param self \ref Protocol stack for performing necessary operations.
 * \param response Buffer to store response in (e.g. ATR, ATPO, ...)
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response)
 * \return int \c PROTOCOL_ACTIVATE_SUCCESS if successful, any other value in case of error
 */
typedef int (*protocol_activatefunction_t)(Protocol *self, uint8_t **response, size_t *response_len);

/**
 * \brief Activates secure element and performs protocol negotiation
 *
 * \details Depending on the protocol data needs to be exchanged with a secure element to negotiate certain protocol aspects like frame sizes, waiting times, etc.
 *
 * \param self \ref Protocol stack for performing necessary operations
 * \param response Buffer to store response in (e.g. ATR, ATPO, ...)
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response)
 * \return int \c PROTOCOL_ACTIVATE_SUCCESS if successful, any other value in case of error
 */
int protocol_activate(Protocol *self, uint8_t **response, size_t *response_len);

/**
 * \brief IFX error encoding function identifier for \ref protocol_transceive(Protocol*, uint8_t*, size_t, uint8_t**, size_t*) and \ref protocol_transceivefunction_t
 */
#define PROTOCOL_TRANSCEIVE 0x82

/**
 * \brief Return code for successful calls to \ref protocol_transceive(Protocol*, uint8_t*, size_t, uint8_t**, size_t*) and \ref protocol_transceivefunction_t
 */
#define PROTOCOL_TRANSCEIVE_SUCCESS SUCCESS

/**
 * \brief Protocol layer specific transceive (send + receive) function
 *
 * \param self \ref Protocol stack for performing necessary operations
 * \param data Data to be send via protocol
 * \param data_len Number of bytes in \p data
 * \param response Buffer to store response in
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response )
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
typedef int (*protocol_transceivefunction_t)(Protocol *self, uint8_t *data, size_t data_len, uint8_t **response, size_t *response_len);

/**
 * \brief Sends data via protocol and reads back response
 *
 * \details Goes through ISO/OSI protocol stack and performs necessary protocol operations (chaining, crc, ...)
 *
 * \param self \ref Protocol stack for performing necessary operations
 * \param data Data to be send via protocol
 * \param data_len Number of bytes in \p data
 * \param response Buffer to store response in
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response )
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int protocol_transceive(Protocol *self, uint8_t *data, size_t data_len, uint8_t **response, size_t *response_len);

/**
 * \brief IFX error encoding function identifier for \ref protocol_transmitfunction_t
 */
#define PROTOCOL_TRANSMIT 0x83

/**
 * \brief Return code for successful calls to \ref protocol_transmitfunction_t
 */
#define PROTOCOL_TRANSMIT_SUCCESS SUCCESS

/**
 * \brief Protocol layer specific transmit function
 *
 * \param self \ref Protocol stack for performing necessary operations
 * \param data Data to be send via protocol
 * \param data_len Number of bytes in \p data
 * \return int \c PROTOCOL_TRANSMIT_SUCCESS if successful, any other value in case of error
 */
typedef int (*protocol_transmitfunction_t)(Protocol *self, uint8_t *data, size_t data_len);

/**
 * \brief IFX error encoding function identifier for \ref protocol_receivefunction_t
 */
#define PROTOCOL_RECEIVE 0x84

/**
 * \brief Return code for successful calls to \ref protocol_receivefunction_t
 */
#define PROTOCOL_RECEIVE_SUCCESS SUCCESS

/**
 * \brief Indicator for unkown length in \ref protocol_receivefunction_t.
 */
#define PROTOCOL_RECEIVE_LENGTH_UNKOWN SIZE_MAX

/**
 * \brief Protocol layer specific receive function
 *
 * \param self \ref Protocol stack for performing necessary operations.
 * \param expected_len Expected number of bytes in response (use \c PROTOCOL_RECEIVE_LENGTH_UNKOWN if not known before)
 * \param response Buffer to store response in
 * \param response_len Buffer to store number of received bytes in (number of bytes in \p response)
 * \return int \c PROTOCOL_RECEIVE_SUCCESS if successful, any other value in case of error
 */
typedef int (*protocol_receivefunction_t)(Protocol *self, size_t expected_len, uint8_t **response, size_t *response_len);

/**
 * \brief IFX error encoding function identifier for any protocol property getter
 */
#define PROTOCOL_GETPROPERTY 0x85

/**
 * \brief Return code for successful calls to any protocol property getter
 */
#define PROTOCOL_GETPROPERTY_SUCCESS SUCCESS

/**
 * \brief IFX error encoding function identifier for any protocol property setter
 */
#define PROTOCOL_SETPROPERTY 0x86

/**
 * \brief Return code for successful calls to any protocol property setter
 */
#define PROTOCOL_SETPROPERTY_SUCCESS SUCCESS

/**
 * \brief Protocol layer specific destructor
 *
 * \details Different protocol (layers) need different clean-up functionality.
 *          This function type gives a generic interface for performing a clean shutdown.
 *
 * \param self \ref Protocol stack for performing necessary operations
 */
typedef void (*protocol_destroyfunction_t)(Protocol *self);

/**
 * \brief Frees memory associated with \ref Protocol object (but not object itself)
 *
 * \details Protocol objects can contain of several layers each of which might hold dynamically allocated data.
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param self Protocol object whose data shall be freed
 */
void protocol_destroy(Protocol *self);

/**
 * \brief Sets logger to be used by protocol
 *
 * \details Sets logger for whole protocol stack, so all layers below will also have the logger set.
 *
 * \param self \ref Protocol object to set logger for
 * \param logger \ref Logger object to be used (might be \c NULL to clear logger)
 */
void protocol_set_logger(Protocol *self, Logger *logger);

/**
 * \brief IFX error encoding function identifier for \ref protocollayer_initialize(Protocol*)
 */
#define PROTOCOLLAYER_INITIALIZE 0x87

/**
 * \brief Return code for successful calls to \ref protocollayer_initialize(Protocol*)
 */
#define PROTOCOLLAYER_INITIALIZE_SUCCESS SUCCESS

/**
 * \brief Initializes \ref Protocol object by setting all members to valid values
 *
 * \param self Protocol object to be initialized
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error
 */
int protocollayer_initialize(Protocol *self);

/**@struct Protocol
 * \brief Generic protocol struct for building ISO/OSI layer stack
 *
 * \details Implementations can either implement \ref Protocol._transceive or \ref Protocol._transmit and \ref Protocol._receive
 */
struct Protocol
{
    /**
     * \brief Private base layer in ISO/OSI stack
     *
     * \details Set by implementations initialization function, do **NOT** set manually
     */
    Protocol *_base;

    /**
     * \brief Private layer identification to verify that correct protocol layer called member functions
     *
     * \details Set by implementations initialization function, do **NOT** set manually
     */
    uint64_t _layer_id;

    /**
     * \brief Private protocol activation function for negotiating protocol specific parameters
     *
     * \details Set by implementations initialization function, do **NOT** set manually.
     *          Can be \c NULL if ISO/OSI layer has no specific activation
     */
    protocol_activatefunction_t _activate;

    /**
     * \brief Private function for sending and receiving data at once
     *
     * \details Set by implementations initialization function, do **NOT** set manually.
     *          Might be \c NULL in which case \ref Protocol._transmit and \ref Protocol._receive must no be \c NULL.
     */
    protocol_transceivefunction_t _transceive;

    /**
     * \brief Private function for sending data
     *
     * \details Set by implementations initialization function, do **NOT** set manually.
     *          Might be \c NULL in which case \ref Protocol._transceive must no be \c NULL.
     */
    protocol_transmitfunction_t _transmit;

    /**
     * \brief Private function for receiving data
     *
     * \details Set by implementations initialization function, do **NOT** set manually.
     *          Might be \c NULL in which case \ref Protocol._transceive must no be \c NULL.
     */
    protocol_receivefunction_t _receive;

    /**
     * \brief Private destructor if further cleanup is necessary
     *
     * \details Set by implementations initialization function, do **NOT** set manually.
     *          \ref protocol_destroy(Protocol*) will call `free()` for \ref Protocol._properties .
     *          If any further cleanup necessary implement it in this function otherwise use \c NULL
     */
    protocol_destroyfunction_t _destructor;

    /**
     * \brief Private member for optional \ref Logger
     *
     * \details Set by \ref protocol_set_logger(Protocol*, Logger*), do **NOT** set manually.
     *          Might be \c NULL.
     */
    Logger *_logger;

    /**
     * \brief Private member for generic properties as \c void*
     *
     * \details Only used internally, do **NOT** set manually.
     *          Might be \c NULL
     */
    void *_properties;
};

#ifdef __cplusplus
}
#endif

#endif // _IFX_PROTOCOL_H_
