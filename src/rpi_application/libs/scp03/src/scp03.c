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
 * \file scp03.c
 * \brief Global Platform Secure Channel Protocol v3
 */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ifx/scp03.h"
#include "ifx/protocol.h"
#include "ifx/apdu.h"
#include "ifx/apduprotocol.h"
#include "gp_general_errors.h"

/**
 * \brief Checks if given APDU response status word indicates a successful value
 *
 * \param sw APDU response status word to be checked
 * \return bool \c true if status word indicates success
 * \todo Define in header to be reusable
 */
static bool _sw_success(Scp03ProtocolProperties* properties, uint16_t sw)
{
    Scp03StatusWordListItem* current_sw = properties->valid_success_sws;
    do
    {
        if(sw == current_sw->value)
            return true;
        current_sw = current_sw->next;
    } while(current_sw != 0);

    return false;
}

Scp03ProtocolProperties* scp03_get_properties(Protocol* self)
{
    return (Scp03ProtocolProperties*)(self->_properties);
}

int scp03_activate(Protocol* self, uint8_t** response, size_t* response_len)
{
    // Activate driver
    return protocol_activate(self->_base, response, response_len);
}

void scp03_destroy(Protocol* self)
{
    scp03_terminate(self);
    if (self->_properties != NULL)
    {
        Scp03StatusWordListItem *current = ((Scp03ProtocolProperties*)self->_properties)->valid_success_sws;
        while (current != NULL)
        {
            Scp03StatusWordListItem *to_delete = current;
            current = to_delete->next;
            free(to_delete);
        }
        free(self->_properties);
        self->_properties = NULL;
    }
}

int scp03_transceive(Protocol* self, uint8_t* data, size_t data_len, uint8_t** response, size_t* response_len)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);

    APDU request_apdu;
    int status = apdu_decode(&request_apdu, data, data_len);
    if(status)
    {
        return status;
    }

    APDUResponse response_apdu;
    status = scp03_transceive_apdu(self, &request_apdu, &response_apdu);
    apdu_destroy(&request_apdu);
    if(status)
    {
        return status;
    }

    status = apduresponse_encode(&response_apdu, response, response_len);
    apduresponse_destroy(&response_apdu);
    if (status != APDURESPONSE_ENCODE_SUCCESS)
    {
        return status;
    }
    return PROTOCOL_TRANSCEIVE_SUCCESS;
}

Scp03StatusWordListItem* scp03_malloc_sw_listitem(uint16_t value, Scp03StatusWordListItem* next)
{
    Scp03StatusWordListItem* result = (Scp03StatusWordListItem*)malloc(sizeof(Scp03StatusWordListItem));
    result->value = value;
    result->next = next;

    return result;
}

void scp03_add_custom_statusword(Protocol* self, uint16_t sw)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);
    properties->valid_success_sws = scp03_malloc_sw_listitem(sw, properties->valid_success_sws);
}

int scp03_initialize(Protocol* self, Protocol* driver)
{
    int status = protocollayer_initialize(self);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        return status;
    }

    self->_base = driver;
    self->_activate = &scp03_activate;
    self->_destructor = &scp03_destroy;
    self->_transceive = &scp03_transceive;

    Scp03ProtocolProperties* properties = (Scp03ProtocolProperties*)malloc(sizeof(Scp03ProtocolProperties));
    self->_properties = properties;

    properties->valid_success_sws = scp03_malloc_sw_listitem(0x9000, 0);

    // Initialize keys
    memset(&(properties->static_keys), 0, sizeof(Scp03StaticKeys));
    memset(&(properties->session_keys), 0, sizeof(Scp03SessionKeys));

    // Set initial security state
    properties->authenticated = SCP03_SECURITY_NONE;
    properties->current_security_level = SCP03_SECURITY_NONE;
    properties->session_security_level = SCP03_SECURITY_NONE;

    return SCP03_INITIALIZE_SUCCESS;
}

int scp03_logical_channel_set_in_cla(uint8_t* cla, uint8_t logical_channel)
{
    // First CLA encoding
    if(logical_channel <= 3)
    {
        // Unset bits 7, 2 and 1
        (*cla) &= ~0x43;
        // Set logical channel (bits 2 and 1)
        (*cla) |= logical_channel;
    }
    // Further CLA encoding
    else if(logical_channel <= 19)
    {
        // Unset bits 4, 3, 2 and 1
        (*cla) &= ~0x0f;
        // Set bit 7
        (*cla) |= 0x40;
        // Set logical channel
        (*cla) |= (logical_channel - 4);
    }
    // Invalid channel
    else
    {
        return -1;
    }

    return 0;
}

int scp03_initialize_update(Protocol* self, uint8_t key_version, uint8_t key_identifier, uint8_t logical_channel, Scp03InitializeUpdateResponse* response)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);

    // Reset session
    scp03_terminate(self);

    // Validate and set logical channel
    if(logical_channel > 19)
    {
        // Invalid channel
        return -1;
    }
    properties->logical_channel = logical_channel;

    int status = scp03_generate_host_challenge(properties->host_challenge);
    if(status)
        return status;

    APDU request = {
        .cla = 0x80,
        .ins = 0x50,
        .p1 = key_version,
        .p2 = key_identifier,
        .lc = SCP03_CHALLENGE_LEN,
        .data = properties->host_challenge,
        .le = 0x1d // FIXME: SE does not behave according to specification
        // .le = 0x0100 // encoded to 0x00 by apdu lib
    };

    status = scp03_logical_channel_set_in_cla(&(request.cla), properties->logical_channel);
    if(status)
        return status;

    // Reset response
    APDUResponse apdu_response;

    status = apdu_transceive(self->_base, &request, &apdu_response);
    if(status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    if(apdu_response.sw == 0x6A88)
    {
        apduresponse_destroy(&apdu_response);
        return IFX_ERROR(LIBSCP03, SCP03_INITIALIZE_UPDATE, SCP03_INITIALIZE_UPDATE_REFERENCED_DATA_NOT_FOUND);
    }

    if (!_sw_success(properties, apdu_response.sw))
    {
        apduresponse_destroy(&apdu_response);
        return IFX_ERROR(LIBSCP03, SCP03_INITIALIZE_UPDATE,
            gp_general_errors_from_statusword(apdu_response.sw));
    }

    const size_t basic_response_len = sizeof(Scp03InitializeUpdateResponse);
    if(apdu_response.len < basic_response_len)
    {
        apduresponse_destroy(&apdu_response);
        return IFX_ERROR(LIBSCP03, SCP03_INITIALIZE_UPDATE, TOO_LITTLE_DATA);
    }

    memcpy(response, apdu_response.data, sizeof(Scp03InitializeUpdateResponse));
    memcpy(properties->card_challenge, response->_card_challenge, SCP03_CHALLENGE_LEN);
    memcpy(properties->card_cryptogram, response->_card_cryptogram, SCP03_CRYPTOGRAM_LEN);

    // Pseudo-random card challenge generation updates seuqence counter
    if(response->key_information.scp_parameter.card_challenge_type)
    {
        if(apdu_response.len != basic_response_len + 3)
        {
            apduresponse_destroy(&apdu_response);
            return IFX_ERROR(LIBSCP03, SCP03_INITIALIZE_UPDATE, SCP03_INITIALIZE_UPDATE_INVALID_RESPONSE_LENGTH);
        }

        properties->sequence_counter = 0;
        uint8_t* sequence_counter = apdu_response.data + basic_response_len;

        for(size_t i = 0; i < 3; i++)
        {
            properties->sequence_counter <<= 8;
            properties->sequence_counter = sequence_counter[i];
        }
    }
    apduresponse_destroy(&apdu_response);
    return SCP03_INITIALIZE_UPDATE_SUCCESS;
}

int scp03_external_authenticate(Protocol* self, Scp03StaticKeys* static_keys, uint8_t security_level)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);

    // Set static keys
    properties->static_keys.enc = malloc(static_keys->enc_len * sizeof(uint8_t));
    if (properties->static_keys.enc == NULL)
    {
        return IFX_ERROR(LIBSCP03, SCP03_EXTERNAL_AUTHENTICATE, OUT_OF_MEMORY);
    }
    properties->static_keys.mac = malloc(static_keys->mac_len * sizeof(uint8_t));
    if (properties->static_keys.mac == NULL)
    {
        free(properties->static_keys.enc);
        properties->static_keys.enc = NULL;
        return IFX_ERROR(LIBSCP03, SCP03_EXTERNAL_AUTHENTICATE, OUT_OF_MEMORY);
    }
    properties->static_keys.dek = malloc(static_keys->dek_len * sizeof(uint8_t));
    if (properties->static_keys.dek == NULL)
    {
        free(properties->static_keys.enc);
        properties->static_keys.enc = NULL;
        free(properties->static_keys.mac);
        properties->static_keys.mac = NULL;
        return IFX_ERROR(LIBSCP03, SCP03_EXTERNAL_AUTHENTICATE, OUT_OF_MEMORY);
    }
    properties->static_keys.enc_len = static_keys->enc_len;
    properties->static_keys.mac_len = static_keys->mac_len;
    properties->static_keys.dek_len = static_keys->dek_len;
    memcpy(properties->static_keys.dek, static_keys->dek, static_keys->dek_len);
    memcpy(properties->static_keys.enc, static_keys->enc, static_keys->enc_len);
    memcpy(properties->static_keys.mac, static_keys->mac, static_keys->mac_len);

    int status = scp03_generate_session_keys(properties->host_challenge, properties->card_challenge, &(properties->static_keys), &(properties->session_keys));
    if(status)
        return status;

    status = scp03_verify_card_cryptogram(&(properties->session_keys), properties->host_challenge, properties->card_challenge, properties->card_cryptogram);
    if(status)
        return status;

    status = scp03_generate_host_cryptogram(&(properties->session_keys), properties->host_challenge, properties->card_challenge, properties->host_cryptogram);
    if(status)
        return status;

    // Sanitize with bit mask 0011 0011 (0x33)
    properties->session_security_level = security_level & 0x33;

    APDU request = {
        .cla = 0x80, // Security (bit 3) will be set in transceive
        .ins = 0x82,
        .p1 = properties->session_security_level,
        .p2 = 0x00,
        .lc = SCP03_CRYPTOGRAM_LEN,
        .data = properties->host_cryptogram,
        .le = 0x00 // removed by apdu lib
    };
    status = scp03_logical_channel_set_in_cla(&(request.cla), properties->logical_channel);
    if(status)
        return status;

    // Temporarily set security for EXTERNAL AUTHENTICATE
    properties->authenticated = SCP03_SECURITY_AUTHENTICATED;
    properties->current_security_level = SCP03_SECURITY_LEVEL_C_MAC;

    APDUResponse response;

    // EXTENAL AUTHENTICATE
    status = scp03_transceive_apdu(self, &request, &response);

    // Reset security
    properties->authenticated = SCP03_SECURITY_NONE;
    properties->current_security_level = SCP03_SECURITY_NONE;

    if(status)
    {
        return status;
    }

    uint16_t sw = response.sw;
    apduresponse_destroy(&response);
    if(sw == 0x6300)
    {
        return IFX_ERROR(LIBSCP03, SCP03_EXTERNAL_AUTHENTICATE,
            SCP03_EXTERNAL_AUTHENTICATE_AUTH_OF_HOST_CRYPTOGRAM_FAILED);
    }

    if (!_sw_success(properties, sw))
    {
        scp03_terminate(self);
        return IFX_ERROR(LIBSCP03, SCP03_EXTERNAL_AUTHENTICATE,
            gp_general_errors_from_statusword(response.sw));
    }

    properties->authenticated = SCP03_SECURITY_AUTHENTICATED;
    properties->current_security_level = properties->session_security_level;
    properties->sequence_counter = 1;
    return SCP03_EXTERNAL_AUTHENTICATE_SUCCESS;
}

int scp03_transceive_apdu(Protocol* self, APDU* request, APDUResponse* response)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);
    if(properties->authenticated == SCP03_SECURITY_NONE)
    {
        // Session terminated or not activated
        return -1;
    }

    int status = 0;
    bool apdu_dynamically_allocated = false;
    APDU* request_ptr = request;
    APDU encrypted_request;
    APDU wrapped_request;
    if((properties->current_security_level & SCP03_SECURITY_LEVEL_C_ENCRYPTION) == SCP03_SECURITY_LEVEL_C_ENCRYPTION)
    {
        if(request_ptr->lc > 0)
        {
            status = scp03_encrypt(&(properties->session_keys), properties->sequence_counter, request_ptr, &encrypted_request);
            if(status)
                return status;
            apdu_dynamically_allocated = true;
            request_ptr = &encrypted_request;
        }
    }

    if((properties->current_security_level & SCP03_SECURITY_LEVEL_C_MAC) == SCP03_SECURITY_LEVEL_C_MAC)
    {
        status = scp03_wrap(&(properties->session_keys), properties->cmac_chaining, request_ptr, &wrapped_request, properties->cmac_chaining);
        if(status)
            return status;

        // Clean up previously allocated APDU
        if (apdu_dynamically_allocated)
        {
            apdu_destroy(request_ptr);
        }

        apdu_dynamically_allocated = true;
        request_ptr = &wrapped_request;
    }

    // TODO: Set Le according to response security level (R_MAC, R_ENCRYPTION)

    APDUResponse plain_response;
    APDUResponse* response_ptr = &plain_response;
    status = apdu_transceive(self->_base, request_ptr, response_ptr);
    if (apdu_dynamically_allocated)
    {
        apdu_destroy(request_ptr);
    }
    if(status)
        return status;

    APDUResponse unwrapped_response;
    APDUResponse decrypted_response;
    if((properties->current_security_level & SCP03_SECURITY_LEVEL_R_MAC) == SCP03_SECURITY_LEVEL_R_MAC)
    {
        status = scp03_unwrap(&(properties->session_keys), properties->cmac_chaining, response_ptr, &unwrapped_response);
        if(status)
            return status;
        apduresponse_destroy(response_ptr);
        response_ptr = &unwrapped_response;
    }

    if((properties->current_security_level & SCP03_SECURITY_LEVEL_R_DECRYPTION) == SCP03_SECURITY_LEVEL_R_DECRYPTION)
    {
        if(response_ptr->len > 0)
        {
            status = scp03_decrypt(&(properties->session_keys), properties->sequence_counter, response_ptr, &decrypted_response);
            if(status)
                return status;
            apduresponse_destroy(response_ptr);
            response_ptr = &decrypted_response;
        }
    }

    memcpy(response, response_ptr, sizeof(APDUResponse));
    properties->sequence_counter++;
    return 0;
}

int scp03_begin_rmac_session(Protocol* self, uint8_t security_level, uint8_t* session_data, size_t session_data_len)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);

    uint8_t r_security_level = security_level & 0x30;

    if(properties->current_security_level != properties->session_security_level)
    {
        // RMAC session already active
        return IFX_ERROR(LIBSCP03, SCP03_BEGIN_RMAC,
            SCP03_BEGIN_RMAC_SESSION_ALREADY_ACTIVE);
    }

    if((properties->current_security_level & 0x30) >= r_security_level)
    {
        // Response security level already the same or higher
        return IFX_ERROR(LIBSCP03, SCP03_BEGIN_RMAC,
            SCP03_BEGIN_RMAC_SECURITY_ALREADY_SAME_OR_HIGHER);
    }

    if((properties->current_security_level & 0x03) < (r_security_level >> 4))
    {
        // Command Security must be the same or higher than Response Security
        return IFX_ERROR(LIBSCP03, SCP03_BEGIN_RMAC,
            SCP03_BEGIN_RMAC_COMMAND_SEC_MUST_BE_SAME_OR_HIGHER_THAN_RESPONSE_SEC);
    }

    APDU request = {
        .cla = 0x80, // Security (bit 3) will be set in transceive
        .ins = 0x7A,
        .p1 = r_security_level,
        .p2 = 1,
        .lc = session_data_len,
        .data = session_data,
        .le = 0x00 // removed by apdu lib
    };
    int status = scp03_logical_channel_set_in_cla(&(request.cla), properties->logical_channel);
    if(status)
        return status;

    APDUResponse response;
    status = scp03_transceive_apdu(self, &request, &response);
    if(status)
    {
        return status;
    }

    if (!_sw_success(properties, response.sw))
    {
        return IFX_ERROR(LIBSCP03, SCP03_BEGIN_RMAC,
            gp_general_errors_from_statusword(response.sw));
    }

    // Clear and set current Response Security Level
    properties->current_security_level &= 0xF0;
    properties->current_security_level |= r_security_level;
    return SCP03_BEGIN_RMAC_SUCCESS;
}

int scp03_end_rmac_session(Protocol* self)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);
    if(properties->current_security_level == properties->session_security_level)
    {
        // No RMAC session active
        return -1;
    }

    APDU request = {
        .cla = 0x80, // Security (bit 3) will be set in transceive
        .ins = 0x78,
        .p1 = 0,
        .p2 = 3,
        .lc = 0,
        .data = 0,
        .le = 0x0100 // encoded to 0x00 by apdu lib
    };
    int status = scp03_logical_channel_set_in_cla(&(request.cla), properties->logical_channel);
    if(status)
        return status;

    APDUResponse response;
    status = scp03_transceive_apdu(self, &request, &response);
    if(status)
    {
        return status;
    }

    if (!_sw_success(properties, response.sw))
    {
        return IFX_ERROR(LIBSCP03, SCP03_BEGIN_RMAC,
            gp_general_errors_from_statusword(response.sw));
    }

    // Reset Security Level
    properties->current_security_level = properties->session_security_level;
    return 0;
}

void scp03_abort(Protocol* self)
{
    Scp03ProtocolProperties* properties = scp03_get_properties(self);
    properties->authenticated = SCP03_SECURITY_NONE;
    properties->current_security_level = SCP03_SECURITY_NONE;
}

void scp03_terminate(Protocol* self)
{
    scp03_abort(self);
    Scp03ProtocolProperties* properties = scp03_get_properties(self);

    // Delete Session Keys
    if(properties->session_keys.enc != 0)
    {
        free(properties->session_keys.enc);
        properties->session_keys.enc = 0;
        properties->session_keys.enc_len = 0;
    }
    if(properties->session_keys.mac != 0)
    {
        free(properties->session_keys.mac);
        properties->session_keys.mac = 0;
        properties->session_keys.mac_len = 0;
    }
    if(properties->session_keys.rmac != 0)
    {
        free(properties->session_keys.rmac);
        properties->session_keys.rmac = 0;
        properties->session_keys.rmac_len = 0;
    }

    // Delete Static Keys
    if(properties->static_keys.enc != 0)
    {
        free(properties->static_keys.enc);
        properties->static_keys.enc = 0;
        properties->static_keys.enc_len = 0;
    }
    if(properties->static_keys.mac != 0)
    {
        free(properties->static_keys.mac);
        properties->static_keys.mac = 0;
        properties->static_keys.mac_len = 0;
    }
    if(properties->static_keys.dek != 0)
    {
        free(properties->static_keys.dek);
        properties->static_keys.dek = 0;
        properties->static_keys.dek = 0;
    }

    // Delete crypto data
    memset(properties->host_challenge, 0, SCP03_CHALLENGE_LEN);
    memset(properties->host_cryptogram, 0, SCP03_CRYPTOGRAM_LEN);
    memset(properties->card_challenge, 0, SCP03_CHALLENGE_LEN);
    memset(properties->card_cryptogram, 0, SCP03_CRYPTOGRAM_LEN);
    memset(properties->cmac_chaining, 0, 16);

    properties->session_security_level = SCP03_SECURITY_NONE;
}
