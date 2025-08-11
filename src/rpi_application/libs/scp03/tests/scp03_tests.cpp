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
 * \file scp03_tests.cpp
 * \brief Tests for Global Platform Secure Channel Protocol v3
 */
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "mockdriver.h"
#include "ifx/scp03.h"

class TestData
{
    public:
        uint8_t security_level;
        std::vector<uint8_t> host_challenge;
        std::vector<uint8_t> card_challenge;
        std::vector<uint8_t> host_cryptogram;
        std::vector<uint8_t> card_cryptogram;
        std::vector<uint8_t> external_auth_cmac;
};

TestData data1
{
    /* Security Level     */ SCP03_SECURITY_LEVEL_C_MAC | SCP03_SECURITY_LEVEL_R_MAC,
    /* Host Challenge     */ { 0x90, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 },
    /* Card Challenge     */ { 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 },
    /* Host Cryptogram    */ { 0x7B, 0xA2, 0x7C, 0x4A, 0x0A, 0xEE, 0x45, 0xCC },
    /* Card Cryptogram    */ { 0x03, 0x3C, 0x9E, 0x87, 0x42, 0x30, 0x7A, 0xF4 },
    /* External Auth CMAC */ { 0x9A, 0x31, 0x72, 0x92, 0x25, 0x18, 0x12, 0x51 }
};

TestData data2
{
    /* Security Level     */ SCP03_SECURITY_NONE,
    /* Host Challenge     */ { 0x90, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 },
    /* Card Challenge     */ { 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 },
    /* Host Cryptogram    */ { 0x7B, 0xA2, 0x7C, 0x4A, 0x0A, 0xEE, 0x45, 0xCC },
    /* Card Cryptogram    */ { 0x03, 0x3C, 0x9E, 0x87, 0x42, 0x30, 0x7A, 0xF4 },
    /* External Auth CMAC */ { 0xDD, 0x7B, 0x17, 0x23, 0x9F, 0x3C, 0x2C, 0xF4 }
};

Scp03StaticKeys static_keys1
{
    /* ENC        */ new uint8_t[32] { 0 },
    /* ENC_LEN    */ 32,
    /* MAC        */ new uint8_t[32] { 0 },
    /* MAC_LEN    */ 32,
    /* DEK        */ new uint8_t[32] { 0 },
    /* DEK_LEN    */ 32
};

TEST_CASE("Initialize sets member functions")
{
    Protocol protocol;
    Protocol driver;
    scp03_initialize(&protocol, &driver);

    REQUIRE(protocol._activate != NULL);
    REQUIRE(protocol._destructor != NULL);
    REQUIRE(protocol._transceive != NULL);

    protocol._base = NULL;
    protocol_destroy(&protocol);
}

TEST_CASE("Initialize allocates properties")
{
    Protocol protocol;
    Protocol driver;
    scp03_initialize(&protocol, &driver);

    Scp03ProtocolProperties* properties = (Scp03ProtocolProperties*)(protocol._properties);
    REQUIRE((properties->authenticated || !properties->authenticated));

    protocol._base = NULL;
    protocol_destroy(&protocol);
}

TEST_CASE("Initialize resets static keys")
{
    Protocol protocol;
    Protocol driver;

    scp03_initialize(&protocol, &driver);

    Scp03ProtocolProperties* properties = (Scp03ProtocolProperties*)(protocol._properties);
    REQUIRE(properties->static_keys.dek_len == 0);
    REQUIRE(properties->static_keys.enc_len == 0);
    REQUIRE(properties->static_keys.mac_len == 0);

    protocol._base = NULL;
    protocol_destroy(&protocol);
}

TEST_CASE("Initialize resets session keys")
{
    Protocol protocol;
    Protocol driver;
    scp03_initialize(&protocol, &driver);

    Scp03ProtocolProperties* properties = (Scp03ProtocolProperties*)(protocol._properties);
    REQUIRE(properties->session_keys.enc_len == 0);
    REQUIRE(properties->session_keys.mac_len == 0);
    REQUIRE(properties->session_keys.rmac_len == 0);

    protocol._base = NULL;
    protocol_destroy(&protocol);
}

TEST_CASE("Initialize resets security")
{
    Protocol protocol;
    Protocol driver;
    scp03_initialize(&protocol, &driver);

    Scp03ProtocolProperties* properties = (Scp03ProtocolProperties*)(protocol._properties);
    REQUIRE(properties->authenticated == SCP03_SECURITY_NONE);
    REQUIRE(properties->session_security_level == SCP03_SECURITY_NONE);
    REQUIRE(properties->current_security_level == SCP03_SECURITY_NONE);

    protocol._base = NULL;
    protocol_destroy(&protocol);
}

void scp03_tests_mock_initialize_update(Protocol* driver, uint8_t key_version, uint8_t key_identifier, const std::vector<uint8_t>& card_challenge, const std::vector<uint8_t>& card_cryptogram)
{
    std::vector<uint8_t> host_challenge(SCP03_CHALLENGE_LEN);
    mockdriver_add_transmission(0x80, 0x50, key_version, key_identifier, host_challenge, 0x1d, 1); // TODO: SE does not behave according to specification

    std::vector<uint8_t> expected_response;
    size_t num_zeros = sizeof(Scp03InitializeUpdateResponse) - SCP03_CHALLENGE_LEN - SCP03_CRYPTOGRAM_LEN;
    for(size_t i = 0; i < num_zeros; i++)
        expected_response.push_back(0);
    for(size_t i = 0; i < SCP03_CHALLENGE_LEN; i++)
        expected_response.push_back(card_challenge[i]);
    for(size_t i = 0; i < SCP03_CRYPTOGRAM_LEN; i++)
        expected_response.push_back(card_cryptogram[i]);
    mockdriver_add_response(expected_response, 0x9000);
}

void scp03_tests_mock_external_authenticate(Protocol* driver, const uint8_t security_level, const std::vector<uint8_t>& host_cryptogram, const std::vector<uint8_t>& cmac)
{
    std::vector<uint8_t> data(host_cryptogram);
    data.insert(data.end(), cmac.begin(), cmac.end());
    mockdriver_add_transmission(0x84, 0x82, security_level, 0, data); // TODO data
    mockdriver_add_response(0x9000); // TODO data
}

void scp03_require_session_aborted(Scp03ProtocolProperties* properties, const uint8_t security_level)
{
    REQUIRE(properties->authenticated == SCP03_SECURITY_NONE);
    REQUIRE(properties->session_security_level == security_level);
    REQUIRE(properties->current_security_level == SCP03_SECURITY_NONE);
    REQUIRE(properties->session_keys.enc != 0);
    REQUIRE(properties->session_keys.enc_len != 0);
    REQUIRE(properties->session_keys.mac != 0);
    REQUIRE(properties->session_keys.mac_len != 0);
    REQUIRE(properties->session_keys.rmac != 0);
    REQUIRE(properties->session_keys.rmac_len != 0);
}

void scp03_require_session_terminated(Scp03ProtocolProperties* properties)
{
    REQUIRE(properties->authenticated == SCP03_SECURITY_NONE);
    REQUIRE(properties->session_security_level == SCP03_SECURITY_NONE);
    REQUIRE(properties->current_security_level == SCP03_SECURITY_NONE);
    REQUIRE(properties->session_keys.enc == 0);
    REQUIRE(properties->session_keys.enc_len == 0);
    REQUIRE(properties->session_keys.mac == 0);
    REQUIRE(properties->session_keys.mac_len == 0);
    REQUIRE(properties->session_keys.rmac == 0);
    REQUIRE(properties->session_keys.rmac_len == 0);
}

// Spec 5.6

// 1. The successful initiation of a Secure Channel Session shall set the Current Security Level to the
//    security level indicated in the EXTERNAL AUTHENTICATE command: it is at least set to AUTHENTICATED.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 1")
{
    auto data = &data1;
    auto keys = &static_keys1;

    Protocol driver;
    mockdriver_initialize(&driver);
    scp03_tests_mock_initialize_update(&driver, 0, 0, data->card_challenge, data->card_cryptogram);
    scp03_tests_mock_external_authenticate(&driver, data->security_level, data->host_cryptogram, data->external_auth_cmac);

    Protocol protocol;
    scp03_initialize(&protocol, &driver);


    uint8_t* response;
    size_t response_len;
    REQUIRE(protocol_activate(&protocol, &response, &response_len) == 0);

    Scp03InitializeUpdateResponse initialize_update_response;
    int status = scp03_initialize_update(&protocol, 0, 0, SCP03_LOGICAL_CHANNEL_DEFAULT, &initialize_update_response);
    REQUIRE(status == 0);

    auto properties = (Scp03ProtocolProperties*)protocol._properties;
    for(size_t i = 0; i < SCP03_CHALLENGE_LEN; i++)
    {
        properties->host_challenge[i] = data->host_challenge[i];
    }

    status = scp03_external_authenticate(&protocol, keys, data->security_level);
    REQUIRE(status == 0);

    REQUIRE(properties->authenticated == SCP03_SECURITY_AUTHENTICATED);
    REQUIRE(properties->session_security_level == data->security_level);
    REQUIRE(properties->current_security_level == data->security_level);

    protocol_destroy(&protocol);
}


// 2. The Current Security Level shall apply to the entire Secure Channel Session unless successfully
//    modified at the request of the Application.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 2")
{
    // TODO: Configurable Security Level
}


// 3. When the Current Security Level is set to NO_SECURITY_LEVEL:
//    a) If the Secure Channel Session was aborted during the same Application Session, the incoming
//       command shall be rejected with a security error;
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 3a")
{
    auto data = &data1;
    auto keys = &static_keys1;

    Protocol driver;
    mockdriver_initialize(&driver);
    scp03_tests_mock_initialize_update(&driver, 0, 0, data->card_challenge, data->card_cryptogram);
    scp03_tests_mock_external_authenticate(&driver, data->security_level, data->host_cryptogram, data->external_auth_cmac);

    Protocol protocol;
    scp03_initialize(&protocol, &driver);

    uint8_t* activate_response;
    size_t activate_response_len;
    protocol_activate(&protocol, &activate_response, &activate_response_len);

    Scp03InitializeUpdateResponse initialize_update_response;
    int status = scp03_initialize_update(&protocol, 0, 0, SCP03_LOGICAL_CHANNEL_DEFAULT, &initialize_update_response);
    REQUIRE(status == 0);

    auto properties = (Scp03ProtocolProperties*)protocol._properties;
    for(size_t i = 0; i < SCP03_CHALLENGE_LEN; i++)
    {
        properties->host_challenge[i] = data->host_challenge[i];
    }

    status = scp03_external_authenticate(&protocol, keys, data->security_level);
    REQUIRE(status == 0);

    scp03_abort(&protocol);

    APDU apdu;
    APDUResponse response;
    REQUIRE(scp03_transceive_apdu(&protocol, &apdu, &response) == -1);

    protocol_destroy(&protocol);
}

//    b) Otherwise no security verification of the incoming command shall be performed. The Application
//       processing the command is responsible to apply its own security rules.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 3b")
{
    // Established channel with SECURITY_NONE (authenticated only) does not apply CMAC or DECRYPTION
    auto data = &data2;
    auto keys = &static_keys1;

    Protocol driver;
    mockdriver_initialize(&driver);
    scp03_tests_mock_initialize_update(&driver, 0, 0, data->card_challenge, data->card_cryptogram);
    scp03_tests_mock_external_authenticate(&driver, data->security_level, data->host_cryptogram, data->external_auth_cmac);


    Protocol protocol;
    scp03_initialize(&protocol, &driver);

    uint8_t* activate_response;
    size_t activate_response_len;
    protocol_activate(&protocol, &activate_response, &activate_response_len);

    Scp03InitializeUpdateResponse initialize_update_response;
    int status = scp03_initialize_update(&protocol, 0, 0, SCP03_LOGICAL_CHANNEL_DEFAULT, &initialize_update_response);
    REQUIRE(status == 0);

    auto properties = (Scp03ProtocolProperties*)protocol._properties;
    for(size_t i = 0; i < SCP03_CHALLENGE_LEN; i++)
    {
        properties->host_challenge[i] = data->host_challenge[i];
    }

    status = scp03_external_authenticate(&protocol, keys, data->security_level);
    REQUIRE(status == 0);

    const size_t request_data_len = 8;
    uint8_t request_data[request_data_len] { 0x01, 0x02, 0x03, 0x04, 0x10, 0x20, 0x30, 0x40 };
    APDU request = {
        0x80,   // cla
        0x82,   // ins
        0x00,   // p1
        0x00,   // p2
        request_data_len, // lc
        request_data, // data
        0       // le
    };

    uint8_t response_data[request_data_len] { 0x10, 0x20, 0x30, 0x40, 0x01, 0x02, 0x03, 0x04 };
    uint16_t response_sw = 0x9000;

    mockdriver_add_transmission(request.cla, request.ins, request.p1, request.p2, std::vector<uint8_t>(request.data, request.data + request.lc));
    mockdriver_add_response(std::vector<uint8_t>(response_data, response_data + request_data_len), response_sw);

    APDUResponse response;
    scp03_transceive_apdu(&protocol, &request, &response);

    REQUIRE(response.sw == response_sw);
    REQUIRE(response.len == request_data_len);
    for(size_t i = 0; i < response.len; i++)
        REQUIRE(response.data[i] == response_data[i]);

    apduresponse_destroy(&response);
    protocol_destroy(&protocol);
}


// 4. If a Secure Channel Session is active (i.e. Current Security Level at least set to AUTHENTICATED),
//    the security of the incoming command shall be checked according to the Current Security Level
//    regardless of the command secure messaging indicator:
//    a) When the security of the command does not match (nor exceeds) the Current Security Level, the
//       command shall be rejected with a security error, the Secure Channel Session aborted and the
//       Current Security Level reset to NO_SECURITY_LEVEL.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 4a")
{
    // TODO
}

//    b) If a security error is found, the command shall be rejected with a security error, the Secure
//       Channel Session aborted and the Current Security Level reset to NO_SECURITY_LEVEL.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 4b")
{
    // TODO
}

//    c) In all other cases, the Secure Channel Session shall remain active and the Current Security Level
//       unmodified. The Application is responsible for further processing the command.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 4c")
{
    // TODO
}


// 5. If a Secure Channel Session is aborted, it is still considered not terminated.
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 5")
{
    auto data = &data1;
    auto keys = &static_keys1;

    Protocol driver;
    mockdriver_initialize(&driver);
    scp03_tests_mock_initialize_update(&driver, 0, 0, data->card_challenge, data->card_cryptogram);
    scp03_tests_mock_external_authenticate(&driver, data->security_level, data->host_cryptogram, data->external_auth_cmac);


    Protocol protocol;
    scp03_initialize(&protocol, &driver);


    uint8_t* response;
    size_t response_len;
    protocol_activate(&protocol, &response, &response_len);

    Scp03InitializeUpdateResponse initialize_update_response;
    int status = scp03_initialize_update(&protocol, 0, 0, SCP03_LOGICAL_CHANNEL_DEFAULT, &initialize_update_response);
    REQUIRE(status == 0);

    auto properties = (Scp03ProtocolProperties*)protocol._properties;
    for(size_t i = 0; i < SCP03_CHALLENGE_LEN; i++)
    {
        properties->host_challenge[i] = data->host_challenge[i];
    }

    status = scp03_external_authenticate(&protocol, keys, data->security_level);
    REQUIRE(status == 0);

    scp03_abort(&protocol);
    scp03_require_session_aborted(properties, data->security_level);

    scp03_terminate(&protocol);
    scp03_require_session_terminated(properties);

    protocol_destroy(&protocol);
}


// 6. The current Secure Channel Session shall be terminated (if aborted or still open) and the Current
//    Security Level reset to NO_SECURITY_LEVEL on either:
//    a) Attempt to initiate a new Secure Channel Session (new INITIALIZE UPDATE command);
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 6a")
{
    // TODO
}

//    b) Termination of the Application Session (e.g. new Application selection);
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 6b")
{
    // TODO
}

//    c) Termination of the associated logical channel;
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 6c")
{
    // TODO
}

//    d) Termination of the Card Session (card reset or power off);
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 6d")
{
    // TODO
}

//    e) Explicit termination by the Application (e.g. invoking GlobalPlatform API).
TEST_CASE("GPC SCP03 v1.1.1 Chapter 5.6 Case 6e")
{
    // TODO
}

void scp03_tests_set_session_keys(Scp03SessionKeys* keys)
{
    const size_t key_len = 32;
    keys->enc_len = key_len;
    keys->mac_len = key_len;
    keys->rmac_len = key_len;
    keys->enc = new uint8_t[32]
        { 0x85, 0x22, 0xCA, 0x50, 0x94, 0x3F, 0x37, 0x20,
          0xE0, 0xDD, 0x8C, 0x3D, 0x83, 0x11, 0x83, 0xE8,
          0x7E, 0xFA, 0x5D, 0xE5, 0x6B, 0x83, 0x7B, 0x4D,
          0x3A, 0x01, 0x54, 0x74, 0x4B, 0xFC, 0x9E, 0x74 };
    keys->mac = new uint8_t[32]
        { 0xCD, 0xC4, 0xAD, 0x71, 0xB2, 0xB9, 0x66, 0x27,
          0x57, 0x0A, 0xAE, 0xDD, 0x6B, 0x71, 0x08, 0x41,
          0x28, 0x40, 0x37, 0xCE, 0x5F, 0x4A, 0x34, 0x4F,
          0x3F, 0x29, 0xDC, 0x42, 0x0E, 0xC6, 0x54, 0x96 };
    keys->rmac = new uint8_t[32]
        { 0xBA, 0xC4, 0x51, 0xCD, 0x6C, 0xA5, 0x6E, 0xB1,
          0x26, 0x30, 0x55, 0xC1, 0xC9, 0xB5, 0xF7, 0xA8,
          0xD2, 0x1F, 0xB6, 0x9E, 0x57, 0x93, 0x97, 0xE9,
          0x6F, 0xEC, 0x11, 0x80, 0x85, 0xBC, 0x9E, 0x33 };
}

void scp03_tests_set_cmac_chaining(uint8_t dst[16], uint8_t src[16])
{
    for(size_t i = 0; i < 16; i++)
        dst[i] = src[i];
}

void scp03_tests_get_initialized_protocol(Protocol* protocol, Protocol* driver, uint8_t logical_channel, uint8_t security_level, uint8_t cmac_chaining[16], uint16_t sequence_counter)
{
    scp03_initialize(protocol, driver);
    scp03_add_custom_statusword(protocol, 0x9001);
    auto properties = (Scp03ProtocolProperties*) protocol->_properties;
    properties->authenticated = SCP03_SECURITY_AUTHENTICATED;
    properties->session_security_level = security_level;
    properties->current_security_level = properties->session_security_level;
    properties->sequence_counter = sequence_counter;
    properties->logical_channel = logical_channel;

    scp03_tests_set_session_keys(&(properties->session_keys));
    scp03_tests_set_cmac_chaining(properties->cmac_chaining, cmac_chaining);
}

void scp03_tests_validate_logical_channel_end_rmac(uint8_t logical_channel, uint8_t cla)
{
    APDU expected {
        /* CLA  */ cla,
        /* INS  */ 0x78,
        /* P1   */ 0,
        /* P2   */ 3,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100 // encoded to 0x00 by apdu lib
    };

    Protocol driver;
    mockdriver_initialize(&driver);

    Protocol protocol;
    uint8_t cmac_chaining[16] = { 0 };
    scp03_tests_get_initialized_protocol(&protocol, &driver, logical_channel, SCP03_SECURITY_NONE, cmac_chaining, 0);
    auto properties = (Scp03ProtocolProperties*)(protocol._properties);
    // Fake RMAC session without enabling CMAC or encryption
    properties->session_security_level = 0xFF;

    mockdriver_add_transmission_apdu(&expected);
    mockdriver_add_response(0x9000);

    int status = scp03_end_rmac_session(&protocol);
    REQUIRE(status == 0);
}

TEST_CASE("Logical channel 0-3")
{
    for(uint8_t channel = 0; channel < 4; channel++)
    {
        uint8_t cla = 0x80 + channel;
        scp03_tests_validate_logical_channel_end_rmac(channel, cla);
    }
}

TEST_CASE("Logical channel 4-19")
{
    for(uint8_t channel = 4; channel < 20; channel++)
    {
        uint8_t cla = 0xC0 + channel - 4;
        scp03_tests_validate_logical_channel_end_rmac(channel, cla);
    }
}

void scp03_tests_transceive_apdu(uint8_t security_level, uint16_t sequence_counter, uint8_t cmac_chaining[16], APDU* request, APDU* request_encrypted_cmacd, APDUResponse* expected_encrypted_cmacd, APDUResponse* expected, uint16_t expected_success_sw = 0x9000)
{
    Protocol driver;
    mockdriver_initialize(&driver);

    Protocol protocol;
    scp03_tests_get_initialized_protocol(&protocol, &driver, SCP03_LOGICAL_CHANNEL_DEFAULT, security_level, cmac_chaining, sequence_counter);
    if(expected_success_sw != 0x9000)
        scp03_add_custom_statusword(&protocol, expected_success_sw);

    mockdriver_add_transmission_apdu(request_encrypted_cmacd);
    mockdriver_add_response_apdu(expected_encrypted_cmacd);

    APDUResponse response;
    int status = scp03_transceive_apdu(&protocol, request, &response);

    REQUIRE(status == 0);
    REQUIRE(expected->sw == response.sw);
    REQUIRE(expected->len == response.len);
    if(response.len > 0)
    {
        std::vector<uint8_t> expected_data(expected->data, expected->data + expected->len);
        std::vector<uint8_t> response_data(response.data, response.data + response.len);
        REQUIRE(expected_data == response_data);
    }
}

TEST_CASE("APDU Transceive: Get Info: Firmware Version (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 1;
    uint8_t cmac_chaining[16]
        { 0x9E, 0x7A, 0x55, 0x8C, 0xC8, 0xAF, 0x22, 0xE7,
          0xC4, 0xF2, 0x5E, 0xFE, 0xE7, 0x16, 0x07, 0x46 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0xEF, 0x86, 0xEA, 0xC4, 0x83, 0xDC, 0xBB, 0xBE },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x44, 0x84, 0x53, 0x44, 0xAB, 0x2D, 0x43, 0x2C, 0xFD, 0xE3, 0xA0, 0x11, 0x6E, 0x46, 0x7C, 0x1E, 0xD9, 0x6E, 0xDC, 0x7C, 0x84, 0x2B, 0xB7, 0xBA },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[8] { 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x6B, 0x00, 0x00 },
        /* len  */ 8,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: VFUL Firmware version (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 2;
    uint8_t cmac_chaining[16]
        { 0xEF, 0x86, 0xEA, 0xC4, 0x83, 0xDC, 0xBB, 0xBE,
          0xE8, 0x3F, 0xC5, 0x08, 0xBD, 0xC9, 0x27, 0xFE };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x01,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x01,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0xB8, 0x19, 0x87, 0xA4, 0xEA, 0x29, 0x3E, 0x4A },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x58, 0x6A, 0xE9, 0xBE, 0xDA, 0x7A, 0xA0, 0x2E, 0x0C, 0x25, 0x12, 0xDD, 0x81, 0x71, 0x0B, 0x6B, 0xCE, 0x36, 0x5B, 0x1B, 0xBD, 0x40, 0xE7, 0x47 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[8] { 0x00, 0x01, 0x00, 0x00, 0x0A, 0x77, 0x00, 0x00 },
        /* len  */ 8,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: Lifecycle state (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 3;
    uint8_t cmac_chaining[16]
        { 0xB8, 0x19, 0x87, 0xA4, 0xEA, 0x29, 0x3E, 0x4A,
          0xD9, 0xEF, 0xC0, 0x06, 0x14, 0xBD, 0x6C, 0x4D };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x03,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x03,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x4E, 0x07, 0x58, 0x2A, 0x45, 0x9F, 0x1B, 0x1F },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x8E, 0xDF, 0xDD, 0x09, 0xBE, 0xE4, 0xA5, 0x44, 0x15, 0x84, 0xB8, 0xB5, 0x91, 0xA9, 0x87, 0x41, 0xFF, 0xEE, 0xCC, 0x99, 0xE9, 0x5B, 0xFC, 0xCC },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[1] { 0x00 },
        /* len  */ 1,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Custom response status word")
{
    const uint16_t custom_sw = 0x9001;
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 3;
    uint8_t cmac_chaining[16]
        { 0xB8, 0x19, 0x87, 0xA4, 0xEA, 0x29, 0x3E, 0x4A,
          0xD9, 0xEF, 0xC0, 0x06, 0x14, 0xBD, 0x6C, 0x4D };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x03,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x03,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x4E, 0x07, 0x58, 0x2A, 0x45, 0x9F, 0x1B, 0x1F },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x8E, 0xDF, 0xDD, 0x09, 0xBE, 0xE4, 0xA5, 0x44, 0x15, 0x84, 0xB8, 0xB5, 0x91, 0xA9, 0x87, 0x41, 0x61, 0xCB, 0x5A, 0x20, 0xD8, 0x61, 0xBD, 0x94 },
        /* len  */ 24,
        /* sw   */ custom_sw
    };

    APDUResponse expected {
        /* data */ new uint8_t[1] { 0x00 },
        /* len  */ 1,
        /* sw   */ custom_sw
    };
    
    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected, custom_sw);
}

TEST_CASE("APDU Transceive: Get Info: Maximum number of file slots (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 4;
    uint8_t cmac_chaining[16]
        { 0x4E, 0x07, 0x58, 0x2A, 0x45, 0x9F, 0x1B, 0x1F,
          0xA3, 0x20, 0xD6, 0xCA, 0xF0, 0x72, 0xD7, 0x62 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x04,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x04,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x89, 0x10, 0x21, 0x2C, 0x61, 0x53, 0xC1, 0xCE },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x89, 0x78, 0x1B, 0x7A, 0xF8, 0x2E, 0x18, 0x09, 0x9F, 0x70, 0x12, 0x71, 0x6B, 0x50, 0x2A, 0x92, 0x20, 0xA1, 0xA2, 0xCB, 0x6D, 0x75, 0x54, 0xB5 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[2] { 0x00, 0x14 },
        /* len  */ 2,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: Maximum number of key slots (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 5;
    uint8_t cmac_chaining[16]
        { 0x89, 0x10, 0x21, 0x2C, 0x61, 0x53, 0xC1, 0xCE,
          0x4A, 0xA3, 0x03, 0x77, 0x7F, 0xF7, 0xB3, 0x27 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x05,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x05,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x07, 0x25, 0x2A, 0x1D, 0xAF, 0x45, 0xEB, 0x70 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x43, 0x21, 0x4E, 0x58, 0xA7, 0x98, 0x3B, 0x32, 0x8D, 0x7B, 0x17, 0xA8, 0x10, 0x19, 0x0B, 0x85, 0xFC, 0x5A, 0x07, 0x8B, 0x9F, 0x51, 0xBA, 0x98 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x07, 0xD0, 0x07, 0xD0 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: Free key slots (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 6;
    uint8_t cmac_chaining[16]
        { 0x07, 0x25, 0x2A, 0x1D, 0xAF, 0x45, 0xEB, 0x70,
          0x56, 0xA3, 0xB0, 0xBD, 0xDC, 0x12, 0x0C, 0x50 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x05,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x05,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x23, 0x17, 0x05, 0x97, 0xBA, 0xB7, 0x2F, 0xD6 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xCD, 0x7E, 0x0E, 0xCD, 0xB5, 0x14, 0x80, 0xD4, 0x0E, 0x7D, 0x04, 0x15, 0x60, 0xC0, 0x5D, 0x8E, 0xA0, 0xFB, 0x60, 0xDE, 0xF5, 0x9E, 0xB7, 0x10 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x07, 0xD0, 0x07, 0xD0 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: Serial number (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 7;
    uint8_t cmac_chaining[16]
        { 0x23, 0x17, 0x05, 0x97, 0xBA, 0xB7, 0x2F, 0xD6,
          0xF9, 0x81, 0x6F, 0x94, 0x39, 0xD3, 0xAF, 0x99 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x07,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x07,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0x05, 0x1C, 0x6E, 0x17, 0x8A, 0xDD, 0x75, 0x86 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[104] { 0x77, 0x57, 0x5E, 0x9A, 0x93, 0x13, 0x68, 0x77, 0x09, 0x81, 0x54, 0x62, 0xF5, 0x1F, 0xD6, 0xEB, 0x41, 0x53, 0x59, 0xE1, 0xA5, 0x87, 0x82, 0xAF, 0xC1, 0xC4, 0x43, 0x7D, 0x71, 0xCB, 0x4B, 0x6D, 0x32, 0xA7, 0x09, 0xAE, 0x17, 0x06, 0x41, 0x8F, 0x60, 0x46, 0x8B, 0x0E, 0x01, 0x69, 0x12, 0x22, 0x7A, 0xF3, 0xF8, 0xD3, 0xF9, 0xFD, 0x7F, 0x33, 0xD0, 0xEA, 0xA5, 0xC5, 0xF6, 0xAD, 0xC7, 0x2A, 0xDB, 0x9F, 0x7C, 0x00, 0xAD, 0x5C, 0x35, 0x9C, 0x14, 0x19, 0xEF, 0x2C, 0x5B, 0x47, 0x2D, 0xE1, 0x30, 0xB3, 0x16, 0xC8, 0x39, 0xD9, 0xF8, 0xD4, 0x7E, 0xCE, 0x3A, 0x95, 0x11, 0xF0, 0x50, 0xAB, 0xC3, 0x7F, 0xF0, 0xA2, 0x62, 0x32, 0x37, 0xC9 },
        /* len  */ 104,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[84] { 0x10, 0x32, 0x54, 0x76, 0xCD, 0x32, 0x33, 0x30, 0x01, 0x00, 0x4B, 0x00, 0x01, 0x00, 0x00, 0x06, 0x0B, 0x0A, 0x09, 0x75, 0x43, 0x00, 0x0F, 0x00, 0x6C, 0x00, 0x41, 0x63, 0x7A, 0x30, 0x38, 0x37, 0xFF, 0x31, 0x7A, 0x80, 0x20, 0x30, 0x02, 0x84, 0x03, 0x32, 0x01, 0x94, 0x03, 0x88, 0x00, 0x0A, 0x01, 0x00, 0x00, 0x3C, 0x01, 0x06, 0x02, 0x20, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0xCD, 0xAB, 0x89 },
        /* len  */ 84,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Get Info: Name (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 8;
    uint8_t cmac_chaining[16]
        { 0x05, 0x1C, 0x6E, 0x17, 0x8A, 0xDD, 0x75, 0x86,
          0x84, 0x7C, 0xEA, 0x01, 0x23, 0x89, 0x2C, 0x1C };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x08,
        /* Lc   */ 0,
        /* Data */ nullptr,
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xCA,
        /* P1   */ 0x00,
        /* P2   */ 0x08,
        /* Lc   */ 8,
        /* Data */ new uint8_t[8] { 0xCD, 0x4B, 0xF5, 0x14, 0x99, 0x29, 0x2F, 0xB1 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[40] { 0xA8, 0x30, 0x7A, 0x18, 0x04, 0x5B, 0xB5, 0xCF, 0xD8, 0x9E, 0x23, 0x34, 0xC5, 0xB2, 0xB3, 0x54, 0xF4, 0x7A, 0xE5, 0xD2, 0x2C, 0x01, 0x6E, 0x9D, 0xC1, 0xE0, 0xF5, 0x2D, 0x17, 0x19, 0x04, 0xCA, 0x48, 0x59, 0x02, 0x9A, 0x6B, 0x16, 0x63, 0x08 },
        /* len  */ 40,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[22] { 0x49, 0x6E, 0x66, 0x69, 0x6E, 0x65, 0x6F, 0x6E, 0x20, 0x53, 0x4C, 0x53, 0x33, 0x37, 0x20, 0x56, 0x32, 0x58, 0x20, 0x48, 0x53, 0x4D },
        /* len  */ 22,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: Application code integrity (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 9;
    uint8_t cmac_chaining[16]
        { 0xCD, 0x4B, 0xF5, 0x14, 0x99, 0x29, 0x2F, 0xB1,
          0x2F, 0xB0, 0x1E, 0xFC, 0x2E, 0x26, 0xB1, 0xE5 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x80, 0x00, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x3B, 0x0B, 0x31, 0x97, 0x7E, 0xD9, 0x2B, 0xDF, 0x3E, 0xD4, 0x1B, 0x9A, 0x2A, 0x7F, 0xB8, 0xCC, 0xBA, 0x2A, 0x1D, 0x82, 0xF5, 0xB1, 0x59, 0x0A },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x26, 0x18, 0xE0, 0x3E, 0x43, 0x13, 0xCD, 0xE9, 0xAA, 0x4B, 0x50, 0xE6, 0x9F, 0x1F, 0x14, 0x94, 0x46, 0xB2, 0x09, 0x25, 0xEF, 0x13, 0x38, 0x1A },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x80, 0x00, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: UMSLC cache (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 10;
    uint8_t cmac_chaining[16]
        { 0xBA, 0x2A, 0x1D, 0x82, 0xF5, 0xB1, 0x59, 0x0A,
          0xC1, 0x4C, 0x38, 0xC9, 0x78, 0x4C, 0x6D, 0x03 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x59, 0x40, 0xA4, 0x2B, 0xBA, 0xF6, 0x65, 0x06, 0x98, 0xA5, 0xDD, 0x9F, 0x16, 0x04, 0x3C, 0x01, 0x2C, 0xE8, 0xE3, 0xA6, 0x02, 0xC4, 0xA5, 0x23 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x32, 0x36, 0x21, 0xEA, 0xCE, 0xE3, 0x81, 0x60, 0x4E, 0xDC, 0x98, 0x02, 0x42, 0xE9, 0x1B, 0x86, 0x96, 0xB5, 0xE5, 0x39, 0x8F, 0x00, 0x2B, 0x59 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: UMSLC hardware (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 11;
    uint8_t cmac_chaining[16]
        { 0x2C, 0xE8, 0xE3, 0xA6, 0x02, 0xC4, 0xA5, 0x23,
          0xD5, 0x0C, 0xD6, 0xDB, 0xED, 0xF4, 0x84, 0xB5 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x4E, 0x13, 0xFC, 0xD8, 0xCC, 0x9B, 0x9E, 0xC0, 0x30, 0x52, 0x52, 0xF8, 0x55, 0x4B, 0xD4, 0x46, 0x8D, 0x17, 0xD2, 0x66, 0xFA, 0x8A, 0xA6, 0x48 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xAB, 0x92, 0x49, 0x8D, 0xAD, 0x78, 0x2A, 0x1E, 0x69, 0x46, 0x28, 0x37, 0x2C, 0x40, 0x6E, 0xD9, 0xDD, 0x11, 0xC0, 0xCA, 0xEE, 0x25, 0x74, 0x8E },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: Code integrity (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 12;
    uint8_t cmac_chaining[16]
        { 0x8D, 0x17, 0xD2, 0x66, 0xFA, 0x8A, 0xA6, 0x48,
          0x95, 0x92, 0xE5, 0x70, 0x85, 0xAD, 0xD5, 0xAB };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x08, 0x00, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0xC5, 0xBE, 0xF4, 0xCA, 0x2F, 0xB1, 0x63, 0xF3, 0x68, 0x39, 0xF9, 0x23, 0x01, 0x51, 0x5B, 0xD8, 0x3C, 0xF6, 0x48, 0x9A, 0x4D, 0xF5, 0x21, 0xC2 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x30, 0x77, 0xC2, 0x6E, 0xF2, 0x29, 0x1D, 0xE8, 0xC9, 0x06, 0x4F, 0x47, 0x92, 0x57, 0xA5, 0xD6, 0x02, 0x9E, 0xA6, 0x52, 0xA6, 0x99, 0xC4, 0x51 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x08, 0x00, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: AES CMAC (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 13;
    uint8_t cmac_chaining[16]
        { 0x3C, 0xF6, 0x48, 0x9A, 0x4D, 0xF5, 0x21, 0xC2,
          0xC3, 0x44, 0xBD, 0xD8, 0xF2, 0xB1, 0x7C, 0xF3 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x01, 0x00, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x23, 0xBD, 0x58, 0x0E, 0x2F, 0xFE, 0x17, 0xDE, 0xD8, 0xD8, 0xCB, 0xEC, 0x1C, 0x09, 0xC5, 0x61, 0xD7, 0x2E, 0x59, 0xA1, 0x3C, 0x9C, 0xEB, 0x17 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xD9, 0xC5, 0x81, 0x37, 0x74, 0xEC, 0x73, 0x12, 0xD3, 0x6D, 0xAC, 0xB5, 0x5D, 0x0B, 0xD3, 0xEF, 0x95, 0x8F, 0xBA, 0xAC, 0xE1, 0xF0, 0xF9, 0x60 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x01, 0x00, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: AES 256 (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 14;
    uint8_t cmac_chaining[16]
        { 0xD7, 0x2E, 0x59, 0xA1, 0x3C, 0x9C, 0xEB, 0x17,
          0x42, 0x92, 0x62, 0x9A, 0xCD, 0x6D, 0x29, 0x61 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x80, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0xEF, 0xCE, 0x4C, 0xC0, 0x39, 0x33, 0x1D, 0x5D, 0x91, 0xAC, 0x60, 0x94, 0x73, 0xE1, 0x80, 0x1C, 0x10, 0x22, 0x26, 0x61, 0x99, 0x8B, 0x7C, 0x7F },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xC1, 0x09, 0x6A, 0x21, 0x37, 0x22, 0x35, 0x03, 0xBF, 0x9C, 0x1D, 0xBF, 0x66, 0xCB, 0x2F, 0x11, 0x15, 0x98, 0x9A, 0xB7, 0xF2, 0xB0, 0xC5, 0x63 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x80, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: TRNG (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 15;
    uint8_t cmac_chaining[16]
        { 0x10, 0x22, 0x26, 0x61, 0x99, 0x8B, 0x7C, 0x7F,
          0xB3, 0xF3, 0x40, 0xB3, 0x37, 0x26, 0x86, 0x38 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x20, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0xBD, 0x7C, 0x62, 0x95, 0x4A, 0xC1, 0x8D, 0x8B, 0x00, 0x1F, 0xD9, 0x5C, 0x47, 0x49, 0x88, 0xED, 0x2D, 0x59, 0x6A, 0x5E, 0x0F, 0x21, 0xAB, 0x08 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xE6, 0x5B, 0x78, 0x39, 0xD1, 0x86, 0x95, 0x95, 0xCD, 0x50, 0xD0, 0x12, 0x06, 0x12, 0x84, 0x86, 0xDE, 0x07, 0x90, 0x50, 0xD0, 0x3C, 0x0A, 0x92 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x20, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: DRNG (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 16;
    uint8_t cmac_chaining[16]
        { 0x2D, 0x59, 0x6A, 0x5E, 0x0F, 0x21, 0xAB, 0x08,
          0x48, 0xDD, 0xE8, 0xCD, 0x02, 0xC6, 0x5B, 0x77 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x10, 0x00, 0x00 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0xFD, 0x43, 0x5C, 0x55, 0x8A, 0x31, 0xFA, 0x95, 0xE1, 0x60, 0xFA, 0xA9, 0xF4, 0x39, 0x55, 0x3B, 0x2A, 0x9D, 0xCE, 0xB5, 0xCA, 0xB0, 0xF1, 0x13 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x9C, 0x55, 0xF4, 0xFC, 0xB4, 0x44, 0xAC, 0x93, 0xCE, 0x12, 0x71, 0x87, 0x0B, 0xAA, 0x60, 0xF5, 0xEE, 0xBC, 0xF3, 0xBC, 0x5E, 0xD9, 0xA2, 0xA9 },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x10, 0x00, 0x00 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: ECC verify (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 17;
    uint8_t cmac_chaining[16]
        { 0x2A, 0x9D, 0xCE, 0xB5, 0xCA, 0xB0, 0xF1, 0x13,
          0x61, 0x33, 0x93, 0x66, 0x06, 0x6C, 0x9E, 0xA0 };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x04 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x82, 0x2B, 0x57, 0xCB, 0x13, 0x45, 0x11, 0x00, 0x26, 0xDF, 0x7E, 0x3E, 0x66, 0x63, 0x31, 0xC1, 0x6A, 0xCF, 0xD8, 0x96, 0xDD, 0xF4, 0x8B, 0x90 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0xD8, 0x2F, 0x52, 0xA5, 0xA9, 0xD7, 0xAB, 0x51, 0xA0, 0xB4, 0x76, 0x20, 0x15, 0x75, 0x95, 0xCE, 0x13, 0xB1, 0x63, 0xD8, 0x2D, 0xF2, 0x1D, 0x5E },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x04 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}

TEST_CASE("APDU Transceive: Self Test: ECC sign (C_ENCRYPTION | R_DECRYPTION)")
{
    const uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION | SCP03_SECURITY_LEVEL_R_DECRYPTION;
    const uint16_t sequence_counter = 18;
    uint8_t cmac_chaining[16]
        { 0x6A, 0xCF, 0xD8, 0x96, 0xDD, 0xF4, 0x8B, 0x90,
          0xF9, 0x17, 0x13, 0x87, 0xB5, 0xBB, 0x63, 0xDA };

    APDU request {
        /* CLA  */ 0xA0,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 4,
        /* Data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x04 },
        /* Le   */ 0x0100
    };

    APDU request_encrypted_cmacd {
        /* CLA  */ 0xA4,
        /* INS  */ 0xFB,
        /* P1   */ 0x00,
        /* P2   */ 0x00,
        /* Lc   */ 24,
        /* Data */ new uint8_t[24] { 0x1F, 0x86, 0xC2, 0x0A, 0xAE, 0x39, 0x33, 0x0F, 0xA7, 0x48, 0x4F, 0xAD, 0x3A, 0x3B, 0xF6, 0x64, 0x51, 0xF8, 0xCF, 0x7F, 0xA1, 0x1A, 0xD5, 0xF5 },
        /* Le   */ 0x0100
    };

    APDUResponse expected_encrypted_cmacd {
        /* data */ new uint8_t[24] { 0x42, 0x38, 0x77, 0xAB, 0xA9, 0xCF, 0x02, 0x34, 0xE5, 0xFF, 0x67, 0x53, 0x54, 0x64, 0xF2, 0x01, 0x4C, 0xE0, 0x6E, 0x64, 0x6E, 0x04, 0xBB, 0x6B },
        /* len  */ 24,
        /* sw   */ 0x9000
    };

    APDUResponse expected {
        /* data */ new uint8_t[4] { 0x00, 0x00, 0x00, 0x04 },
        /* len  */ 4,
        /* sw   */ 0x9000
    };

    scp03_tests_transceive_apdu(security_level, sequence_counter, cmac_chaining,
        &request, &request_encrypted_cmacd,
        &expected_encrypted_cmacd, &expected);
}
