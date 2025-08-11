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
 * \file ifx/scp03.h
 * \brief Global Platform Secure Channel Protocol v3
 */
#ifndef _IFX_SCP03_HOST_H_
#define _IFX_SCP03_HOST_H_

#include "ifx/scp03_crypto.h"
#include "ifx/error.h"
#include "ifx/protocol.h"
#include "ifx/apdu.h"
#include <stddef.h>
#include <stdint.h>

#define LIBSCP03 0x33

#define SCP03_SECURITY_NONE 0
#define SCP03_SECURITY_AUTHENTICATED 1

#define SCP03_SECURITY_LEVEL_R_MAC 0x10
#define SCP03_SECURITY_LEVEL_R_DECRYPTION 0x30
#define SCP03_SECURITY_LEVEL_C_MAC 0x01
#define SCP03_SECURITY_LEVEL_C_ENCRYPTION 0x03

#define SCP03_LOGICAL_CHANNEL_DEFAULT 0x00

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _Scp03StatusWordListItem Scp03StatusWordListItem;
struct _Scp03StatusWordListItem
{
    uint16_t value;
    Scp03StatusWordListItem* next;
};

typedef struct
{
    uint16_t sequence_counter;
    uint8_t authenticated;
    uint8_t session_security_level;
    uint8_t current_security_level;
    uint8_t host_challenge[SCP03_CHALLENGE_LEN];
    uint8_t card_challenge[SCP03_CHALLENGE_LEN];
    uint8_t host_cryptogram[SCP03_CRYPTOGRAM_LEN];
    uint8_t card_cryptogram[SCP03_CRYPTOGRAM_LEN];
    uint8_t cmac_chaining[16];
    Scp03StaticKeys static_keys;
    Scp03SessionKeys session_keys;
    Scp03StatusWordListItem* valid_success_sws;
    uint8_t logical_channel;
} Scp03ProtocolProperties;

typedef struct
{
    uint8_t reserved : 1;
    uint8_t r_security : 2;
    uint8_t card_challenge_type : 1;
    uint8_t rfu : 4;
} Scp03ProtocolConfiguration;

typedef struct
{
    uint8_t key_version;
    uint8_t scp_identifier;
    Scp03ProtocolConfiguration scp_parameter;
} Scp03KeyInformation;

typedef struct
{
    uint8_t key_diversification_data[10];
    Scp03KeyInformation key_information;
    uint8_t _card_challenge[SCP03_CHALLENGE_LEN];
    uint8_t _card_cryptogram[SCP03_CRYPTOGRAM_LEN];
} Scp03InitializeUpdateResponse;

#define SCP03_INITIALIZE 0x01
#define SCP03_INITIALIZE_SUCCESS SUCCESS
int scp03_initialize(Protocol* self, Protocol* driver);

#define SCP03_INITIALIZE_UPDATE 0x02
#define SCP03_INITIALIZE_UPDATE_SUCCESS SUCCESS
#define SCP03_INITIALIZE_UPDATE_REFERENCED_DATA_NOT_FOUND 0x01
#define SCP03_INITIALIZE_UPDATE_INVALID_RESPONSE_LENGTH 0x02
int scp03_initialize_update(Protocol* self, uint8_t key_version, uint8_t key_identifier, uint8_t logical_channel, Scp03InitializeUpdateResponse* response);

#define SCP03_EXTERNAL_AUTHENTICATE 0x03
#define SCP03_EXTERNAL_AUTHENTICATE_SUCCESS SUCCESS
#define SCP03_EXTERNAL_AUTHENTICATE_AUTH_OF_HOST_CRYPTOGRAM_FAILED 0x01
int scp03_external_authenticate(Protocol* self, Scp03StaticKeys* static_keys, uint8_t security_level);

int scp03_transceive_apdu(Protocol* self, APDU* request, APDUResponse* response);

#define SCP03_BEGIN_RMAC 0x04
#define SCP03_BEGIN_RMAC_SUCCESS SUCCESS
#define SCP03_BEGIN_RMAC_SESSION_ALREADY_ACTIVE 0x01
#define SCP03_BEGIN_RMAC_SECURITY_ALREADY_SAME_OR_HIGHER 0x02
#define SCP03_BEGIN_RMAC_COMMAND_SEC_MUST_BE_SAME_OR_HIGHER_THAN_RESPONSE_SEC 0x03
int scp03_begin_rmac_session(Protocol* self, uint8_t security_level, uint8_t* session_data, size_t session_data_len);

#define SCP03_END_RMAC 0x05
#define SCP03_END_RMAC_SUCCESS SUCCESS
int scp03_end_rmac_session(Protocol* self);

void scp03_abort(Protocol* self);
void scp03_terminate(Protocol* self);
void scp03_add_custom_statusword(Protocol* self, uint16_t sw);

#ifdef __cplusplus
}
#endif

#endif
