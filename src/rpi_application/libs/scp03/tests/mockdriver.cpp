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
 * \file mockdriver.cpp
 * \brief Mock implementation of Protocol layer for tests
 */
#include "mockdriver.h"
#include "catch2/catch.hpp"
#include <vector>
#include <queue>
#include "ifx/protocol.h"

using namespace std;

/**
 * \brief Expected APDUs to be transmitted to Secure Element
 *
 * \details When \ref mockdriver_transmit(Protocol*, uint8_t*, size_t) is called it will pop an expected APDU from this queue and verify that the given data matches the expected.
 */
queue<shared_ptr<MockdriverExpectedTransmission>> mockdriver_expected_transmissions;

/**
 * \brief Responses to transmitted APDUs
 *
 * \details When \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*) is called it will pop an APDU response from this queue, verify that the expected length matches and return it.
 */
queue<shared_ptr<vector<uint8_t>>> mockdriver_responses;

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
void mockdriver_add_transmission(const uint8_t cla, const uint8_t ins, const uint8_t p1, const uint8_t p2, const vector<uint8_t> &data, const size_t le, const uint8_t ignore_data_value)
{
    uint8_t *c_data = NULL;
    if (!data.empty())
    {
        c_data = (uint8_t *)malloc(data.size());
        REQUIRE(c_data != NULL);
        copy(begin(data), end(data), c_data);
    }
    APDU apdu = {cla, ins, p1, p2, data.size(), c_data, le};
    mockdriver_add_transmission_apdu(&apdu, ignore_data_value);
}

void mockdriver_add_transmission_apdu(APDU* apdu, const uint8_t ignore_data_value)
{
    uint8_t *encoded;
    size_t encoded_len;
    REQUIRE(apdu_encode(apdu, &encoded, &encoded_len) == APDU_ENCODE_SUCCESS);
    auto transmission = new MockdriverExpectedTransmission();
    transmission->ignore_data_value = ignore_data_value;
    for(size_t i = 0; i < encoded_len; i++)
        transmission->data.push_back(encoded[i]);
    apdu_destroy(apdu);
    free(encoded);
    mockdriver_expected_transmissions.push(shared_ptr<MockdriverExpectedTransmission>(transmission));
}

/**
 * \brief Adds a new APDU response to \ref mockdriver_responses
 *
 * \details Creates \ref APDUResponse object based on parameters, encodes it to its binary representation and adds it to queue.
 *
 * \param data Actual response data
 * \param sw APDU response status word
 */
void mockdriver_add_response(const vector<uint8_t> &data, const uint16_t sw)
{
    uint8_t *c_data = NULL;
    if (!data.empty())
    {
        c_data = (uint8_t *)malloc(data.size());
        REQUIRE(c_data != NULL);
        copy(begin(data), end(data), c_data);
    }
    APDUResponse response = {c_data, data.size(), sw};
    mockdriver_add_response_apdu(&response);
}

void mockdriver_add_response_apdu(APDUResponse* response)
{
    uint8_t *encoded;
    size_t encoded_len;
    REQUIRE(apduresponse_encode(response, &encoded, &encoded_len) == APDURESPONSE_ENCODE_SUCCESS);
    auto as_vector = new vector<uint8_t>(encoded, encoded + encoded_len);
    apduresponse_destroy(response);
    free(encoded);
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(as_vector));
}

/**
 * \brief Overload of \ref mockdriver_add_response(const vector<uint8_t>&, const uint16_t) with empty \p data
 */
void mockdriver_add_response(const uint16_t sw)
{
    mockdriver_add_response(vector<uint8_t>(), sw);
}

/**
 * \brief Mock implementation of \ref protocol_transmitfunction_t
 * \see protocol_transmitfunction_t
 */
int mockdriver_transmit(Protocol *self, uint8_t *data, size_t data_len)
{
    // Get APDU response from queue
    REQUIRE(!mockdriver_expected_transmissions.empty());
    auto expected = mockdriver_expected_transmissions.front();
    mockdriver_expected_transmissions.pop();
    REQUIRE(expected.get() != NULL);

    // Verify that data matches
    if(expected->ignore_data_value != 0)
    {
        vector<uint8_t> as_vector(data, data + data_len);
        APDU expected_apdu;
        apdu_decode(&expected_apdu, expected->data.data(), expected->data.size());
        APDU data_as_apdu;
        apdu_decode(&data_as_apdu, data, data_len);

        REQUIRE(expected_apdu.cla == data_as_apdu.cla);
        REQUIRE(expected_apdu.ins == data_as_apdu.ins);
        REQUIRE(expected_apdu.p1 == data_as_apdu.p1);
        REQUIRE(expected_apdu.p2 == data_as_apdu.p2);
        REQUIRE(expected_apdu.lc == data_as_apdu.lc);
        REQUIRE(expected_apdu.le == data_as_apdu.le);

        apdu_destroy(&expected_apdu);
        apdu_destroy(&data_as_apdu);
        return PROTOCOL_TRANSMIT_SUCCESS;
    }

    vector<uint8_t> as_vector(data, data + data_len);
    REQUIRE(as_vector == expected->data);
    return PROTOCOL_TRANSMIT_SUCCESS;
}

/**
 * \brief Mock implementation of \ref protocol_receivefunction_t
 * \see protocol_receivefunction_t
 */
int mockdriver_receive(Protocol *self, size_t expected_len, uint8_t **response_buffer, size_t *response_len)
{
    // Get response from queue
    REQUIRE(!mockdriver_responses.empty());
    auto response = mockdriver_responses.front();
    mockdriver_responses.pop();
    REQUIRE(response.get() != NULL);

    // Validate response
    if (expected_len != PROTOCOL_RECEIVE_LENGTH_UNKOWN)
    {
        REQUIRE(response.get()->size() == expected_len);
    }

    // Copy to buffer
    *response_buffer = (uint8_t *)malloc(response.get()->size());
    REQUIRE(*response_buffer != NULL);
    copy(begin(*response.get()), end(*response.get()), *response_buffer);
    *response_len = response.get()->size();

    return PROTOCOL_RECEIVE_SUCCESS;
}

/**
 * \brief Mock implementation of \ref protocol_destroyfunction_t
 * \see protocol_destroyfunction_t
 */
void mockdriver_destroy(Protocol *self)
{
    REQUIRE(mockdriver_expected_transmissions.empty());
    REQUIRE(mockdriver_responses.empty());
}

int mockdriver_activate(Protocol* self, uint8_t** response, size_t* response_len)
{
    return 0;
}

/**
 * \brief Initializes \ref Protocol object for mock driver implementation
 *
 * \details Assures that the internal state of the mockdriver is clean.
 *
 * \param self Protocol object to be initialized
 */
void mockdriver_initialize(Protocol *self)
{
    // Assure valid initial state
    while (!mockdriver_expected_transmissions.empty())
    {
        mockdriver_expected_transmissions.pop();
    }
    while (!mockdriver_responses.empty())
    {
        mockdriver_responses.pop();
    }

    // Populate Protocol struct
    REQUIRE(protocollayer_initialize(self) == PROTOCOLLAYER_INITIALIZE_SUCCESS);
    self->_activate = mockdriver_activate;
    self->_transmit = mockdriver_transmit;
    self->_receive = mockdriver_receive;
    self->_destructor = mockdriver_destroy;
}
