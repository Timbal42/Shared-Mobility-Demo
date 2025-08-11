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
 * \file t1prime-protocol-tests.cpp
 * \brief Tests for Global Platform T=1' protocol implementation
 */
#include "catch2/catch.hpp"
#include <vector>
#include <queue>
#include "ifx/crc.h"
#include "ifx/t1prime.h"
#include "t1prime.h"
#include "t1prime/datastructures.h"

using namespace std;

/**
 * \brief Utility class to wrap \ref Block objects and encode them for the mock driver
 */
class BlockWrapper
{
private:
    /**
     * \brief Protocol control byte mapping to \ref Block.pcb
     */
    const uint8_t _pcb;

    /**
     * \brief Information data mapping to \ref Block.information
     */
    const vector<uint8_t> &_information;

public:
    /**
     * \brief Constructor only stores parameters to members
     *
     * \param pcb See \ref BlockWrapper::_pcb
     * \param information See \ref BlockWrapper::_information
     */
    BlockWrapper(const uint8_t pcb, const vector<uint8_t> &information = vector<uint8_t>());

    /**
     * \brief Binary block encoding to be used by \ref mockdriver_transmit(Protocol*, uint8_t*, size_t)
     *
     * \details Blocks are not split into prologue/information/epilogue as the protocol implementation will write all data at once.
     *          Sets \ref Block.nad to the HD to SE case.
     *
     * \return shared_ptr<vector<uint8_t>> Binary block encoding to be used by \ref mockdriver_transmit(Protocol*, uint8_t*, size_t)
     */
    shared_ptr<vector<uint8_t>> block() const;

    /**
     * \brief Block encoding (frame granular) to be used by \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*)
     *
     * \details Frames are split into prologue/information/epilogue as they are expected to be read by the protocol implementation in chunks:
     *           * 4 bytes fixed length prologue
     *           * 0-n bytes optional information field
     *           * 2 bytes fixed length epilogue
     *          Sets \ref Block.nad to the SE to HD case.
     *
     * \return shared_ptr<vector<shared_ptr<vector<uint8_t>>>> Binary block encoding to be used by \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*)
     */
    shared_ptr<vector<shared_ptr<vector<uint8_t>>>> frames() const;
};

/**
 * \brief Constructor only stores parameters to members
 *
 * \param pcb See \ref BlockWrapper::_pcb
 * \param information See \ref BlockWrapper::_information
 */
BlockWrapper::BlockWrapper(const uint8_t pcb, const vector<uint8_t> &information) : _pcb(pcb), _information(information)
{
}

/**
 * \brief Binary block encoding to be used by \ref mockdriver_transmit(Protocol*, uint8_t*, size_t)
 *
 * \details Blocks are not split into prologue/information/epilogue as the protocol implementation will write all data at once.
 *          Sets \ref Block.nad to the HD to SE case.
 *
 * \return shared_ptr<vector<uint8_t>> Binary block encoding to be used by \ref mockdriver_transmit(Protocol*, uint8_t*, size_t)
 */
shared_ptr<vector<uint8_t>> BlockWrapper::block() const
{
    uint8_t *information = NULL;
    if (!_information.empty())
    {
        information = (uint8_t *)malloc(_information.size());
        REQUIRE(information != NULL);
        copy(begin(_information), end(_information), information);
    }
    Block block{
        NAD_HD_TO_SE,
        _pcb,
        _information.size(),
        information};
    uint8_t *encoded;
    size_t encoded_len;
    REQUIRE(t1prime_block_encode(&block, &encoded, &encoded_len) == T1PRIME_BLOCK_ENCODE_SUCCESS);
    auto result = new vector<uint8_t>(encoded, encoded + encoded_len);
    t1prime_block_destroy(&block);
    free(encoded);
    return shared_ptr<vector<uint8_t>>(result);
}

/**
 * \brief Block encoding (frame granular) to be used by \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*)
 *
 * \details Frames are split into prologue/information/epilogue as they are expected to be read by the protocol implementation in chunks:
 *           * 4 bytes fixed length prologue
 *           * 0-n bytes optional information field
 *           * 2 bytes fixed length epilogue
 *          Sets \ref Block.nad to the SE to HD case.
 *
 * \return shared_ptr<vector<shared_ptr<vector<uint8_t>>>> Binary block encoding to be used by \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*)
 */
shared_ptr<vector<shared_ptr<vector<uint8_t>>>> BlockWrapper::frames() const
{
    auto encoded = new vector<shared_ptr<vector<uint8_t>>>();

    // Single byte NAD for polling
    vector<uint8_t> nad{0x12};
    encoded->push_back(shared_ptr<vector<uint8_t>>(new vector<uint8_t>(nad)));

    // Vector for CRC calculation
    vector<uint8_t> crc_buffer(nad);

    // Fixed length prologue
    vector<uint8_t> prologue{_pcb, (uint8_t)((_information.size() & 0xff00) >> 8), (uint8_t)(_information.size() & 0x00ff)};
    encoded->push_back(shared_ptr<vector<uint8_t>>(new vector<uint8_t>(prologue)));
    crc_buffer.insert(end(crc_buffer), begin(prologue), end(prologue));

    // Optional information field
    if (!_information.empty())
    {
        encoded->push_back(shared_ptr<vector<uint8_t>>(new vector<uint8_t>(_information)));
        crc_buffer.insert(end(crc_buffer), begin(_information), end(_information));
    }

    // Fixed length epilogue
    auto crc = crc16_ccitt_x25(&crc_buffer[0], crc_buffer.size());
    auto epilogue = new vector<uint8_t>{(uint8_t)((crc & 0xff00) >> 8), (uint8_t)(crc & 0x00ff)};
    encoded->push_back(shared_ptr<vector<uint8_t>>(epilogue));

    return shared_ptr<vector<shared_ptr<vector<uint8_t>>>>(encoded);
}

/**
 * \brief Expected blocks to be transmitted according to Global Platform T=1' protocol
 *
 * \details When \ref mockdriver_transmit(Protocol*, uint8_t*, size_t) is called it will pop an expected block from this queue and verify that the given data matches the expected.
 */
static queue<shared_ptr<vector<uint8_t>>> mockdriver_expected_transmissions;

/**
 * \brief Responses to transmitted blocks
 *
 * \details When \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*) is called it will pop a response from this queue, verify that the expected length matches and return it.
 */
static queue<shared_ptr<vector<uint8_t>>> mockdriver_responses;

/**
 * \brief Adds a new request to \ref mockdriver_expected_transmissions
 *
 * \details Converts \ref BlockWrapper object to its binary representation and adds it to queue.
 *
 * \param transmission Expected transmission
 */
static void mockdriver_add_transmission(const BlockWrapper &transmission)
{
    auto transmission_block = transmission.block();
    REQUIRE(transmission_block.get() != NULL);
    mockdriver_expected_transmissions.push(transmission_block);
}

/**
 * \brief Adds a new response to \ref mockdriver_responses
 *
 * \details Converts \ref BlockWrapper object to its frame granular binary representation and adds it to queue.
 *
 * \param response Response to be returned by \ref mockdriver_receive(Protocol*, size_t, uint8_t**, size_t*)
 */
static void mockdriver_add_response(const BlockWrapper &response)
{
    auto response_frames = response.frames();
    REQUIRE(response_frames.get() != NULL);
    for (auto response_frame : *response_frames.get())
    {
        REQUIRE(response_frame.get() != NULL);
        mockdriver_responses.push(response_frame);
    }
}

/**
 * \brief Adds a new request/response pair to the mock driver
 *
 * \param transmission Expected block to be send to the secure element
 * \param response Response to be returned as if received from secure element
 * \see \ref mockdriver_add_transmission(const BlockWrapper&)
 * \see \ref mockdriver_add_response(const BlockWrapper&)
 */
static void mockdriver_expect(const BlockWrapper &transmission, const BlockWrapper &response)
{
    mockdriver_add_transmission(transmission);
    mockdriver_add_response(response);
}

/**
 * \brief Mock implementation of \ref protocol_transmitfunction_t
 * \see protocol_transmitfunction_t
 */
static int mockdriver_transmit(Protocol *self, uint8_t *data, size_t data_len)
{
    // Get block from queue
    REQUIRE(!mockdriver_expected_transmissions.empty());
    auto expected = mockdriver_expected_transmissions.front();
    mockdriver_expected_transmissions.pop();
    REQUIRE(expected.get() != NULL);

    // Verify that block matches
    vector<uint8_t> as_vector(data, data + data_len);
    REQUIRE(as_vector == *expected.get());

    return PROTOCOL_TRANSMIT_SUCCESS;
}

/**
 * \brief Mock implementation of \ref protocol_receivefunction_t
 * \see protocol_receivefunction_t
 */
static int mockdriver_receive(Protocol *self, size_t expected_len, uint8_t **response_buffer, size_t *response_len)
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
static void mockdriver_destroy(Protocol *self)
{
    REQUIRE(mockdriver_expected_transmissions.empty());
    REQUIRE(mockdriver_responses.empty());
}

/**
 * \brief Initializes \ref Protocol object for mock driver implementation
 *
 * \details Assures that the internal state of the mockdriver is clean.
 *
 * \param self Protocol object to be initialized
 */
static void mockdriver_initialize(Protocol *self)
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
    self->_transmit = mockdriver_transmit;
    self->_receive = mockdriver_receive;
    self->_destructor = mockdriver_destroy;
}

TEST_CASE("T=1' valid S(RESYNCH)", "[s_resynch]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe0));

    // Call protocol functionality
    REQUIRE(s_resynch(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' invalid S(RESYNCH) -> wrong response block", "[s_resynch, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe1));
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe1));
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe1));

    // Call protocol functionality
    auto status = s_resynch(&protocol);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' valid S(CIP)", "[s_cip]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe4, vector<uint8_t>{
                               0x01,
                               0x03, 0x00, 0x00, 0x00,
                               0x01,
                               0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x04, 0x00, 0x00, 0x00, 0x00,
                               0x00}));

    // Call protocol functionality
    CIP cip;
    REQUIRE(s_cip(&protocol, &cip) == PROTOCOL_TRANSCEIVE_SUCCESS);
    REQUIRE(cip.version == 0x01);
    REQUIRE(cip.iin_len == 3);
    REQUIRE(vector<uint8_t>(cip.iin, cip.iin + cip.iin_len) == vector<uint8_t>{0x00, 0x00, 0x00});
    REQUIRE(cip.plid == 0x01);
    REQUIRE(vector<uint8_t>(cip.plp, cip.plp + cip.plp_len) == vector<uint8_t>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    REQUIRE(cip.dllp_len == 4);
    REQUIRE(vector<uint8_t>(cip.dllp, cip.dllp + cip.dllp_len) == vector<uint8_t>{0x00, 0x00, 0x00, 0x00});
    REQUIRE(cip.hb_len == 0);
    REQUIRE(cip.hb == NULL);

    // Cleanup
    t1prime_cip_destroy(&cip);
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' invalid S(CIP) -> no response information data", "[s_cip, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe4));

    // Call protocol functionality
    CIP cip;
    auto status = s_cip(&protocol, &cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == TOO_LITTLE_DATA);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' invalid S(CIP) -> wrong response block", "[s_cip, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe0));

    // Call protocol functionality
    CIP cip;
    auto status = s_cip(&protocol, &cip);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' valid S(SWR)", "[s_swr]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xcf),
        BlockWrapper(0xef));

    // Call protocol functionality
    REQUIRE(s_swr(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' invalid S(SWR) -> wrong response block", "[s_swr, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xcf),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xcf),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xcf),
        BlockWrapper(0xe0));

    // Call protocol functionality
    auto status = s_swr(&protocol);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Get and set block waiting time", "[t1prime_get_bwt, t1prime_set_bwt]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Get default BWT value
    uint16_t bwt;
    REQUIRE(t1prime_get_bwt(&protocol, &bwt) == PROTOCOL_GETPROPERTY_SUCCESS);
    REQUIRE(bwt == 300);

    // Update BWT value
    REQUIRE(t1prime_set_bwt(&protocol, 100) == PROTOCOL_SETPROPERTY_SUCCESS);

    // Read back updated value
    REQUIRE(t1prime_get_bwt(&protocol, &bwt) == PROTOCOL_SETPROPERTY_SUCCESS);
    REQUIRE(bwt == 100);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD value <= 0xfe", "[t1prime_set_ifsd]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe1, vector<uint8_t>{0x01}));

    // Call protocol functionality
    REQUIRE(t1prime_set_ifsd(&protocol, 0x01) == PROTOCOL_SETPROPERTY_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD value > 0xfe", "[t1prime_set_ifsd]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01, 0x02}),
        BlockWrapper(0xe1, vector<uint8_t>{0x01, 0x02}));

    // Call protocol functionality
    REQUIRE(t1prime_set_ifsd(&protocol, 0x0102) == PROTOCOL_SETPROPERTY_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD value with invalid IFSD 0x00", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0x00);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == ILLEGAL_ARGUMENT);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD value with invalid IFSD > 0xff9", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0xff9 + 1);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == ILLEGAL_ARGUMENT);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD error > wrong response block", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe0));
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe0));

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0x01);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD error > wrong response information length 0", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe1));

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0x01);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD error > wrong response information length > 2", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe1, vector<uint8_t>{0x00, 0x00, 0x00}));

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0x01);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Set IFSD error > response IFS mismatch", "[t1prime_set_ifsd, error]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc1, vector<uint8_t>{0x01}),
        BlockWrapper(0xe1, vector<uint8_t>{0x02}));

    // Call protocol functionality
    auto status = t1prime_set_ifsd(&protocol, 0x01);
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_reason(status) == INVALID_BLOCK);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("Activate T=1' protocol", "[t1prime_activate, protocol_activate]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe4, vector<uint8_t>{
                               0x01,
                               0x03, 0x00, 0x00, 0x00,
                               0x01,
                               0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x04, 0x00, 0x80, 0x00, 0x80,
                               0x00}));
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe0));

    // Call protocol functionality
    uint8_t *atpo = NULL;
    size_t atpo_len = 0;
    REQUIRE(protocol_activate(&protocol, &atpo, &atpo_len) == PROTOCOL_ACTIVATE_SUCCESS);

    // Cleanup
    if ((atpo != NULL) && (atpo_len > 0))
    {
        free(atpo);
        atpo = NULL;
    }
    protocol_destroy(&protocol);
}

TEST_CASE("T=1' protocol activation parameter negotiation", "[t1prime_activate, protocol_activate]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    mockdriver_expect(
        BlockWrapper(0xc4),
        BlockWrapper(0xe4, vector<uint8_t>{
                               0x01,
                               0x03, 0x00, 0x00, 0x00,
                               0x01,
                               0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x04, 0x00, 0x80, 0x00, 0x80,
                               0x00}));
    mockdriver_expect(
        BlockWrapper(0xc0),
        BlockWrapper(0xe0));

    // Set protocol state to value as some values have been changed
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->bwt = 0x00;
    protocol_state->ifsc = 0x00;
    protocol_state->send_counter = 0x01;
    protocol_state->receive_counter = 0x01;

    // Call protocol functionality
    uint8_t *atpo = NULL;
    size_t atpo_len = 0;
    REQUIRE(protocol_activate(&protocol, &atpo, &atpo_len) == PROTOCOL_ACTIVATE_SUCCESS);

    // Verify that parameters have successfully been negotiated
    REQUIRE(protocol_state->bwt == 0x80);
    REQUIRE(protocol_state->ifsc == 0x80);
    REQUIRE(protocol_state->send_counter == 0x00);
    REQUIRE(protocol_state->receive_counter == 0x00);

    // Cleanup
    if ((atpo != NULL) && (atpo_len > 0))
    {
        free(atpo);
        atpo = NULL;
    }
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 1", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));
    // ---> I(1, 0)
    mockdriver_add_transmission(BlockWrapper(0x40, vector<uint8_t>{0x03, 0x04}));
    // <--- I(1, 0)
    mockdriver_add_response(BlockWrapper(0x40, vector<uint8_t>{0xf3, 0xf4}));

    // Call protocol functionality
    uint8_t first_data[] = {0x01, 0x02};
    size_t first_data_len = sizeof(first_data);
    uint8_t *first_response = NULL;
    size_t first_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, first_data, first_data_len, &first_response, &first_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify first response
    REQUIRE(first_response != NULL);
    REQUIRE(first_response_len == 2);
    REQUIRE(vector<uint8_t>(first_response, first_response + first_response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Call protocol functionality again
    uint8_t second_data[] = {0x03, 0x04};
    size_t second_data_len = sizeof(second_data);
    uint8_t *second_response = NULL;
    size_t second_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, second_data, second_data_len, &second_response, &second_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify second response
    REQUIRE(second_response != NULL);
    REQUIRE(second_response_len == 2);
    REQUIRE(vector<uint8_t>(second_response, second_response + second_response_len) == vector<uint8_t>{0xf3, 0xf4});

    // Cleanup
    free(first_response);
    free(second_response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 2", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- S(WTX request)
    mockdriver_add_response(BlockWrapper(0xc3, vector<uint8_t>{0x10}));
    // ---> S(WTX response)
    mockdriver_add_transmission(BlockWrapper(0xe3, vector<uint8_t>{0x10}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 3", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 4", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));
    // ---> S(IFS request)
    mockdriver_add_transmission(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // <--- S(IFS response)
    mockdriver_add_response(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // ---> I(1, 0)
    mockdriver_add_transmission(BlockWrapper(0x40, vector<uint8_t>{0x03, 0x04}));
    // <--- I(1, 0)
    mockdriver_add_response(BlockWrapper(0x40, vector<uint8_t>{0xf3, 0xf4}));

    // Call protocol functionality
    uint8_t first_data[] = {0x01, 0x02};
    size_t first_data_len = sizeof(first_data);
    uint8_t *first_response = NULL;
    size_t first_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, first_data, first_data_len, &first_response, &first_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify first response
    REQUIRE(first_response != NULL);
    REQUIRE(first_response_len == 2);
    REQUIRE(vector<uint8_t>(first_response, first_response + first_response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Update IFSC
    REQUIRE(t1prime_set_ifsd(&protocol, 0x79) == PROTOCOL_SETPROPERTY_SUCCESS);

    // Call protocol functionality again
    uint8_t second_data[] = {0x03, 0x04};
    size_t second_data_len = sizeof(second_data);
    uint8_t *second_response = NULL;
    size_t second_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, second_data, second_data_len, &second_response, &second_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify second response
    REQUIRE(second_response != NULL);
    REQUIRE(second_response_len == 2);
    REQUIRE(vector<uint8_t>(second_response, second_response + second_response_len) == vector<uint8_t>{0xf3, 0xf4});

    // Cleanup
    free(first_response);
    free(second_response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 5", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 1)
    mockdriver_add_transmission(BlockWrapper(0x20, vector<uint8_t>{0x01, 0x02}));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x90));
    // ---> I(1, 1)
    mockdriver_add_transmission(BlockWrapper(0x60, vector<uint8_t>{0x03, 0x04}));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x80));
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x05, 0x06}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 6", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 1)
    mockdriver_add_response(BlockWrapper(0x20, vector<uint8_t>{0xf1, 0xf2}));
    // ---> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <--- I(1, 0)
    mockdriver_add_response(BlockWrapper(0x40, vector<uint8_t>{0xf3, 0xf4}));
    // ---> I(1, 0)
    mockdriver_add_transmission(BlockWrapper(0x40, vector<uint8_t>{0x03, 0x04}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf5, 0xf6}));

    // Call protocol functionality
    uint8_t first_data[] = {0x01, 0x02};
    size_t first_data_len = sizeof(first_data);
    uint8_t *first_response = NULL;
    size_t first_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, first_data, first_data_len, &first_response, &first_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify first response
    REQUIRE(first_response != NULL);
    REQUIRE(first_response_len == 4);
    REQUIRE(vector<uint8_t>(first_response, first_response + first_response_len) == vector<uint8_t>{0xf1, 0xf2, 0xf3, 0xf4});

    // Call protocol functionality again
    uint8_t second_data[] = {0x03, 0x04};
    size_t second_data_len = sizeof(second_data);
    uint8_t *second_response = NULL;
    size_t second_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, second_data, second_data_len, &second_response, &second_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify second response
    REQUIRE(second_response != NULL);
    REQUIRE(second_response_len == 2);
    REQUIRE(vector<uint8_t>(second_response, second_response + second_response_len) == vector<uint8_t>{0xf5, 0xf6});

    // Cleanup
    free(first_response);
    free(second_response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 7", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 1)
    mockdriver_add_response(BlockWrapper(0x20, vector<uint8_t>{0xf1, 0xf2}));
    // ---> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <--- I(1, 0) [LEN = 00]
    mockdriver_add_response(BlockWrapper(0x40));
    // ---> I(1, 0)
    mockdriver_add_transmission(BlockWrapper(0x40, vector<uint8_t>{0x03, 0x04}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf3, 0xf4}));

    // Call protocol functionality
    uint8_t first_data[] = {0x01, 0x02};
    size_t first_data_len = sizeof(first_data);
    uint8_t *first_response = NULL;
    size_t first_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, first_data, first_data_len, &first_response, &first_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify first response
    REQUIRE(first_response != NULL);
    REQUIRE(first_response_len == 2);
    REQUIRE(vector<uint8_t>(first_response, first_response + first_response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Call protocol functionality again
    uint8_t second_data[] = {0x03, 0x04};
    size_t second_data_len = sizeof(second_data);
    uint8_t *second_response = NULL;
    size_t second_response_len = 0;
    REQUIRE(protocol_transceive(&protocol, second_data, second_data_len, &second_response, &second_response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify second response
    REQUIRE(second_response != NULL);
    REQUIRE(second_response_len == 2);
    REQUIRE(vector<uint8_t>(second_response, second_response + second_response_len) == vector<uint8_t>{0xf3, 0xf4});

    // Cleanup
    free(first_response);
    free(second_response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 8", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // -x-> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x81));
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 9", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 10", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // -x-> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- R(0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x81, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x81));
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 11", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x91));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 12", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <-x- R(1)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x91, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 13", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <-x- R(1)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x91, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // --x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x91));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 14", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- S(WTX request)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xc3, 0x00, 0x01}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x10}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- S(WTX request)
    mockdriver_add_response(BlockWrapper(0xc3, vector<uint8_t>{0x10}));
    // ---> S(WTX response)
    mockdriver_add_transmission(BlockWrapper(0xe3, vector<uint8_t>{0x10}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 15", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- S(WTX request)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xc3, 0x00, 0x01}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x10}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- S(WTX request)
    mockdriver_add_response(BlockWrapper(0xc3, vector<uint8_t>{0x10}));
    // ---> S(WTX response)
    mockdriver_add_transmission(BlockWrapper(0xe3, vector<uint8_t>{0x10}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 16", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- S(IFS request)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xc1, 0x00, 0x01}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x79}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 17", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <-x- S(IFS request)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xc1, 0x00, 0x01}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x79}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 18", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // -x-> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 19", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 20", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- S(IFS request)
    mockdriver_add_response(BlockWrapper(0xc1, vector<uint8_t>{0x79}));
    // ---> S(IFS response)
    mockdriver_add_transmission(BlockWrapper(0xe1, vector<uint8_t>{0x79}));
    // <-x- I(0, 0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00, 0x02}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xf1, 0xf2}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -x-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x91));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 21", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 1)
    mockdriver_add_transmission(BlockWrapper(0x20, vector<uint8_t>{0x01, 0x02}));
    // <-/- R(1)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x90, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x90));
    // ---> I(1, 1)
    mockdriver_add_transmission(BlockWrapper(0x60, vector<uint8_t>{0x03, 0x04}));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x80));
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x05, 0x06}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 22", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 1)
    mockdriver_add_transmission(BlockWrapper(0x20, vector<uint8_t>{0x01, 0x02}));
    // <-/- R(1)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x90, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // -/-> R(0)
    mockdriver_add_transmission(BlockWrapper(0x81));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x90));
    // ---> I(1, 1)
    mockdriver_add_transmission(BlockWrapper(0x60, vector<uint8_t>{0x03, 0x04}));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x80));
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x05, 0x06}));
    // <--- I(0, 0)
    mockdriver_add_response(BlockWrapper(0x00, vector<uint8_t>{0xf1, 0xf2}));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 2);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 23", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 1)
    mockdriver_add_response(BlockWrapper(0x20, vector<uint8_t>{0xf1, 0xf2}));
    // -/-> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x91));
    // ---> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <--- I(1, 0)
    mockdriver_add_response(BlockWrapper(0x40, vector<uint8_t>{0xf3, 0xf4}));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 4);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2, 0xf3, 0xf4});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 24", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 1)
    mockdriver_add_response(BlockWrapper(0x20, vector<uint8_t>{0xf1, 0xf2}));
    // -/-> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <-/- R(1)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x91, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> R(1)
    mockdriver_add_transmission(BlockWrapper(0x91));
    // <--- I(1, 0)
    mockdriver_add_response(BlockWrapper(0x40, vector<uint8_t>{0xf3, 0xf4}));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    REQUIRE(protocol_transceive(&protocol, data, data_len, &response, &response_len) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Verify response
    REQUIRE(response != NULL);
    REQUIRE(response_len == 4);
    REQUIRE(vector<uint8_t>(response, response + response_len) == vector<uint8_t>{0xf1, 0xf2, 0xf3, 0xf4});

    // Cleanup
    free(response);
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 26", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 0)
    mockdriver_add_transmission(BlockWrapper(0x00, vector<uint8_t>{0x01, 0x02}));
    // <--- I(0, 1)
    mockdriver_add_response(BlockWrapper(0x20, vector<uint8_t>{0xf1, 0xf2}));
    // ---> R(1)
    mockdriver_add_transmission(BlockWrapper(0x90));
    // <--- S(ABORT request)
    mockdriver_add_response(BlockWrapper(0xc2));
    // ---> S(ABORT response)
    mockdriver_add_transmission(BlockWrapper(0xe2));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x90));

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    int status = protocol_transceive(&protocol, data, data_len, &response, &response_len);

    // Verify response
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == PROTOCOL_TRANSCEIVE);
    REQUIRE(ifx_error_get_reason(status) == TRANSCEIVE_ABORTED);
    REQUIRE(response == NULL);
    REQUIRE(response_len == 0);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 27", "[t1prime_transceive, protocol_transceive]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> I(0, 1)
    mockdriver_add_transmission(BlockWrapper(0x20, vector<uint8_t>{0x01, 0x02}));
    // <--- R(1)
    mockdriver_add_response(BlockWrapper(0x90));
    // ---> I(1, 1)
    mockdriver_add_transmission(BlockWrapper(0x60, vector<uint8_t>{0x03, 0x04}));
    // <--- S(ABORT request)
    mockdriver_add_response(BlockWrapper(0xc2));
    // ---> S(ABORT response)
    mockdriver_add_transmission(BlockWrapper(0xe2));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x80));

    // Set IFSC to force chaining
    T1PrimeProtocolState *protocol_state;
    REQUIRE(t1prime_get_protocol_state(&protocol, &protocol_state) == PROTOCOL_GETPROPERTY_SUCCESS);
    protocol_state->ifsc = 2;

    // Call protocol functionality
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    size_t data_len = sizeof(data);
    uint8_t *response = NULL;
    size_t response_len = 0;
    int status = protocol_transceive(&protocol, data, data_len, &response, &response_len);

    // Verify response
    REQUIRE(ifx_is_error(status));
    REQUIRE(ifx_error_get_module(status) == LIBT1PRIME);
    REQUIRE(ifx_error_get_function(status) == PROTOCOL_TRANSCEIVE);
    REQUIRE(ifx_error_get_reason(status) == TRANSCEIVE_ABORTED);
    REQUIRE(response == NULL);
    REQUIRE(response_len == 0);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 29", "[s_resynch]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <--- S(RESYNCH response)
    mockdriver_add_response(BlockWrapper(0xe0));

    // Call protocol functionality
    REQUIRE(s_resynch(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 30", "[s_resynch]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // ---> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <-/- S(RESYNCH response)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0xe0, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <--- S(RESYNCH response)
    mockdriver_add_response(BlockWrapper(0xe0));

    // Call protocol functionality
    REQUIRE(s_resynch(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 31", "[s_resynch]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // -/-> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <--- R(0)
    mockdriver_add_response(BlockWrapper(0x81));
    // ---> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <--- S(RESYNCH response)
    mockdriver_add_response(BlockWrapper(0xe0));

    // Call protocol functionality
    REQUIRE(s_resynch(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}

TEST_CASE("ISO7816-3 Annex A Scenario 32", "[s_resynch]")
{
    // Initialize protocol
    Protocol driver;
    mockdriver_initialize(&driver);
    Protocol protocol;
    REQUIRE(t1prime_initialize(&protocol, &driver) == PROTOCOLLAYER_INITIALIZE_SUCCESS);

    // Prepare mock values
    // -/-> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <-/- R(0)
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x12}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x81, 0x00, 0x00}));
    mockdriver_responses.push(shared_ptr<vector<uint8_t>>(new vector<uint8_t>{0x00, 0x00}));
    // ---> S(RESYNCH request)
    mockdriver_add_transmission(BlockWrapper(0xC0));
    // <--- S(RESYNCH response)
    mockdriver_add_response(BlockWrapper(0xe0));

    // Call protocol functionality
    REQUIRE(s_resynch(&protocol) == PROTOCOL_TRANSCEIVE_SUCCESS);

    // Cleanup
    protocol_destroy(&protocol);
}
