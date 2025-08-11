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
 * \file mockdriver.h
 * \brief Mock implementation of Protocol layer for tests
 */

#ifndef _IFX_MOCKDRIVER_H_
#define _IFX_MOCKDRIVER_H_

#include <vector>
#include <queue>
#include "ifx/protocol.h"
#include "ifx/apdu.h"

using namespace std;

typedef struct
{
    std::vector<uint8_t> data;
    uint8_t ignore_data_value;
} MockdriverExpectedTransmission;

/**
 * \brief Adds a new APDU to \ref mockdriver_expected_transmissions
 *
 * \details Creates \ref APDU object based on parameters, encodes it to its binary representation and adds it to queue.
 *
 * \param cla APDU instruction class
 * \param ins APDU instruction code
 * \param p1 First APDU instruction parameter byte
 * \param p2 Second APDU instruction parameter byte
 * \param data Actual APDU content dataActual APDU content data
 * \param le Expected number of bytes in response
 */
void mockdriver_add_transmission(const uint8_t cla, const uint8_t ins, const uint8_t p1, const uint8_t p2, const vector<uint8_t> &data = vector<uint8_t>(), const size_t le = 0, const uint8_t ignore_data_value = 0);
void mockdriver_add_transmission_apdu(APDU* apdu, const uint8_t ignore_data_value = 0);

/**
 * \brief Adds a new APDU response to \ref mockdriver_responses
 *
 * \details Creates \ref APDUResponse object based on parameters, encodes it to its binary representation and adds it to queue.
 *
 * \param data Actual response data
 * \param sw APDU response status word
 */
void mockdriver_add_response(const vector<uint8_t> &data, const uint16_t sw);
void mockdriver_add_response_apdu(APDUResponse* response);

/**
 * \brief Overload of \ref mockdriver_add_response(const vector<uint8_t>&, const uint16_t) with empty \p data
 */
void mockdriver_add_response(const uint16_t sw);

/**
 * \brief Mock implementation of \ref protocol_transmitfunction_t
 * \see protocol_transmitfunction_t
 */
int mockdriver_transmit(Protocol *self, uint8_t *data, size_t data_len);

/**
 * \brief Mock implementation of \ref protocol_receivefunction_t
 * \see protocol_receivefunction_t
 */
int mockdriver_receive(Protocol *self, size_t expected_len, uint8_t **response_buffer, size_t *response_len);

/**
 * \brief Mock implementation of \ref protocol_destroyfunction_t
 * \see protocol_destroyfunction_t
 */
void mockdriver_destroy(Protocol *self);

/**
 * \brief Initializes \ref Protocol object for mock driver implementation
 *
 * \details Assures that the internal state of the mockdriver is clean.
 *
 * \param self Protocol object to be initialized
 */
void mockdriver_initialize(Protocol *self);

#endif
