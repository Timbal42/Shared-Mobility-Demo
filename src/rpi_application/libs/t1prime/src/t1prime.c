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
 * \file t1prime.c
 * \brief Global Platform T=1' protocol
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ifx/crc.h"
#include "ifx/timer.h"
#include "ifx/t1prime.h"
#include "t1prime.h"

#ifdef INTERFACE_I2C
#include "ifx/i2c.h"
#else
#include "ifx/spi.h"
#endif

/**
 * \brief Initializes \ref Protocol object for Global Platform T=1' protocol.
 *
 * \param self \ref Protocol object to be initialized.
 * \param driver \ref Physical (driver) layer used for communication.
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
int t1prime_initialize(Protocol *self, Protocol *driver)
{
    // Validate driver layer
    if ((driver == NULL) || (driver->_transmit == NULL) || (driver->_receive == NULL))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOLLAYER_INITIALIZE, INVALID_PROTOCOLSTACK);
    }

    // Populate object
    int status = protocollayer_initialize(self);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        return status;
    }
    self->_layer_id = T1PRIME_PROTOCOLLAYER_ID;
    self->_base = driver;
    self->_activate = t1prime_activate;
    self->_transceive = t1prime_transceive;
    self->_destructor = t1prime_destroy;

#ifndef INTERFACE_I2C
    // Set clock phase and polarity
    status = spi_set_clock_polarity(self, false);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
    status = spi_set_clock_phase(self, false);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
#endif

    return PROTOCOLLAYER_INITIALIZE_SUCCESS;
}

/**
 * \brief protocol_activatefunction_t for Global Platform T=1' protocol
 *
 * \see protocol_activatefunction_t
 */
int t1prime_activate(Protocol *self, uint8_t **response, size_t *response_len)
{
    // Set default communication values in case SE changed
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->ifsc = T1PRIME_DEFAULT_IFSC;
    protocol_state->bwt = T1PRIME_DEFAULT_BWT;

#ifdef INTERFACE_I2C
    status = i2c_set_clock_frequency(self, T1PRIME_DEFAULT_I2C_CLOCK_FREQUENCY);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
#else
    status = spi_set_clock_frequency(self, T1PRIME_DEFAULT_SPI_CLOCK_FREQUENCY);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
    status = spi_set_guard_time(self, T1PRIME_DEFAULT_SPI_SEGT);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
    status = spi_set_buffer_size(self, T1PRIME_DEFAULT_SPI_SEAL);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
#endif

    // Base layer should not need activation but use just in case
    uint8_t *atpo = NULL;
    size_t atpo_len;
    protocol_activate(self->_base, &atpo, &atpo_len);
    if (atpo != NULL)
    {
        free(atpo);
        atpo = NULL;
    }

    // Read communication interface parameters to negotiate protocol parameters
    CIP cip;
    status = s_cip(self, &cip);
    if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    // Set data-link layer parameters
    DLLP dllp;
    status = t1prime_dllp_decode(&dllp, cip.dllp, cip.dllp_len);
    if (status != T1PRIME_DLLP_DECODE_SUCCESS)
    {
        t1prime_cip_destroy(&cip);
        return status;
    }
    protocol_state->bwt = dllp.bwt;
    protocol_state->ifsc = dllp.ifsc;
    t1prime_dllp_destroy(&dllp);

    // Set physical layer parameters depending on interface
    if (cip.plid == PLID_I2C)
    {
#ifndef INTERFACE_I2C
        t1prime_cip_destroy(&cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, INVALID_PLID);
#else
        I2CPLP plp;
        status = t1prime_i2c_plp_decode(&plp, cip.plp, cip.plp_len);
        if (status != T1PRIME_PLP_DECODE_SUCCESS)
        {
            t1prime_cip_destroy(&cip);
            return status;
        }

        // Set clock frequency
        status = i2c_set_clock_frequency(self, plp.mcf * 1000u);
        if (status != PROTOCOL_SETPROPERTY_SUCCESS)
        {
            t1prime_i2c_plp_destroy(&plp);
            t1prime_cip_destroy(&cip);
            return status;
        }

        // Set polling time
        protocol_state->mpot = plp.mpot;
        t1prime_i2c_plp_destroy(&plp);
#endif
    }
    else if (cip.plid == PLID_SPI)
    {
#ifdef INTERFACE_I2C
        t1prime_cip_destroy(&cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, INVALID_PLID);
#else
        SPIPLP plp;
        status = t1prime_spi_plp_decode(&plp, cip.plp, cip.plp_len);
        if (status != T1PRIME_PLP_DECODE_SUCCESS)
        {
            t1prime_cip_destroy(&cip);
            return status;
        }

        // Set clock frequency
        status = spi_set_clock_frequency(self, plp.mcf * 1000u);
        if (status != PROTOCOL_SETPROPERTY_SUCCESS)
        {
            t1prime_spi_plp_destroy(&plp);
            t1prime_cip_destroy(&cip);
            return status;
        }

        // Set polling time
        protocol_state->mpot = plp.mpot;

        // Set guard time
        status = spi_set_guard_time(self, plp.segt);
        if (status != PROTOCOL_SETPROPERTY_SUCCESS)
        {
            t1prime_spi_plp_destroy(&plp);
            t1prime_cip_destroy(&cip);
            return status;
        }

        // Set SPI buffer size
        status = spi_set_buffer_size(self, plp.seal);
        if (status != PROTOCOL_SETPROPERTY_SUCCESS)
        {
            t1prime_spi_plp_destroy(&plp);
            t1prime_cip_destroy(&cip);
            return status;
        }

        t1prime_spi_plp_destroy(&plp);
#endif
    }
    else
    {
        t1prime_cip_destroy(&cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, INVALID_PLID);
    }
    t1prime_cip_destroy(&cip);

    // Resynchronize sequence counters
    status = s_resynch(self);
    if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    // Do not send pseudo ATR
    return PROTOCOL_ACTIVATE_SUCCESS;
}

/**
 * \brief \ref protocol_transceivefunction_t for Global Platform T=1' protocol
 *
 * \see protocol_transceivefunction_t
 */
int t1prime_transceive(Protocol *self, uint8_t *data, size_t data_len, uint8_t **response, size_t *response_len)
{
    // Validate parameters
    if ((data == NULL) || (data_len == 0) || (response == NULL) || (response_len == NULL))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, ILLEGAL_ARGUMENT);
    }

    // Get protocol state for communication
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Prepare first block to be send
    Block transmission_block;
    transmission_block.information_size = data_len < protocol_state->ifsc ? data_len : protocol_state->ifsc;
    size_t last_information_size = transmission_block.information_size;
    size_t offset = 0;
    size_t remaining = data_len;
    transmission_block.nad = 0x21;
    transmission_block.pcb = T1PRIME_PCB_I(protocol_state->send_counter, (remaining - last_information_size) > 0);
    transmission_block.information = malloc(transmission_block.information_size);
    if (transmission_block.information == NULL)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, OUT_OF_MEMORY);
    }
    memcpy(transmission_block.information, data, transmission_block.information_size);

    // Send blocks in loop to handle state
    Block response_block;
    bool aborted = false;
    while (true)
    {
        status = t1prime_block_transceive(self, &transmission_block, &response_block);
        t1prime_block_destroy(&transmission_block);
        if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
        {
            return status;
        }

        // I(N(S), M) -> SE starts sending response
        if (T1PRIME_PCB_IS_I(response_block.pcb))
        {
            // Cannot receive I block response while not all data has been sent
            if ((remaining - last_information_size) > 0)
            {
                t1prime_block_destroy(&response_block);
                return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
            }
            protocol_state->send_counter ^= 0x01;
            break;
        }
        // R(N(R)) -> SE wants (another) block
        else if (T1PRIME_PCB_IS_R(response_block.pcb))
        {
            // SE expects next block
            if ((protocol_state->send_counter ^ 0x01) == T1PRIME_PCB_R_GET_NR(response_block.pcb))
            {
                t1prime_block_destroy(&response_block);

                // Check if chain was aborted
                if (aborted)
                {
                    return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, TRANSCEIVE_ABORTED);
                }

                // SE has last last block
                if ((remaining - last_information_size) == 0)
                {
                    transmission_block.pcb = T1PRIME_PCB_R_CRC(protocol_state->receive_counter);
                    transmission_block.information = NULL;
                    transmission_block.information_size = 0;
                }
                else
                {
                    // Update state to move to next part of data
                    remaining -= last_information_size;
                    offset += last_information_size;
                    protocol_state->send_counter ^= 0x01;

                    // Prepare next block
                    transmission_block.information_size = remaining < protocol_state->ifsc ? remaining : protocol_state->ifsc;
                    last_information_size = transmission_block.information_size;
                    transmission_block.pcb = T1PRIME_PCB_I(protocol_state->send_counter, (remaining - last_information_size) > 0);
                    transmission_block.information = malloc(transmission_block.information_size);
                    if (transmission_block.information == NULL)
                    {
                        t1prime_block_destroy(&response_block);
                        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, OUT_OF_MEMORY);
                    }
                    memcpy(transmission_block.information, data + offset, transmission_block.information_size);
                }
            }
            // SE wants a retransmission
            else
            {
                t1prime_block_destroy(&response_block);

                // Retransmit last I block
                transmission_block.pcb = T1PRIME_PCB_I(protocol_state->send_counter, (remaining - last_information_size) > 0);
                transmission_block.information_size = last_information_size;
                transmission_block.information = malloc(transmission_block.information_size);
                if (transmission_block.information == NULL)
                {
                    return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, OUT_OF_MEMORY);
                }
                memcpy(transmission_block.information, data + offset, transmission_block.information_size);
            }
        }
        // S(WTX REQ) -> SE needs more time
        else if (response_block.pcb == T1PRIME_PCB_S_WTX_REQ)
        {
            // Verify information field
            if ((response_block.information == NULL) || (response_block.information_size != 1))
            {
                t1prime_block_destroy(&response_block);
                return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
            }
            protocol_state->wtx_delay = response_block.information[0] * protocol_state->bwt;

            // Send S(WTX RESP)
            transmission_block.pcb = T1PRIME_PCB_S_WTX_RESP;
            transmission_block.information = response_block.information;
            transmission_block.information_size = response_block.information_size;
        }
        // S(IFS REQ) -> SE wants to indicate that it can send more or less data
        else if (response_block.pcb == T1PRIME_PCB_S_IFS_REQ)
        {
            // Verify IFS value
            size_t ifs;
            status = t1prime_ifs_decode(&ifs, response_block.information, response_block.information_size);
            if (status != T1PRIME_IFS_DECODE_SUCCESS)
            {
                t1prime_block_destroy(&response_block);
                return status;
            }

            // Update state in case new IFSC is smaller and SE wants a retransmission
            last_information_size = ifs < last_information_size ? ifs : last_information_size;

            // Send S(IFS RESP)
            transmission_block.pcb = T1PRIME_PCB_S_IFS_RESP;
            transmission_block.information = response_block.information;
            transmission_block.information_size = response_block.information_size;
        }
        // S(ABORT REQ) -> SE wants to stop chain request
        else if (response_block.pcb == T1PRIME_PCB_S_ABORT_REQ)
        {
            // Send S(ABORT RESP)
            t1prime_block_destroy(&response_block);
            transmission_block.pcb = T1PRIME_PCB_S_ABORT_RESP;
            transmission_block.information = NULL;
            transmission_block.information_size = 0;
            aborted = true;
        }
        else
        {
            t1prime_block_destroy(&response_block);
            return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
        }
    }
    // Validate response in loop to handle state
    *response = NULL;
    *response_len = 0;
    while (true)
    {
        // I(N(S), M) -> SE sent response
        if (T1PRIME_PCB_IS_I(response_block.pcb))
        {
            // Validate sequence counter
            if (T1PRIME_PCB_I_GET_NS(response_block.pcb) != protocol_state->receive_counter)
            {
                t1prime_block_destroy(&response_block);
                if ((*response) != NULL)
                {
                    free(*response);
                    *response = NULL;
                }
                *response_len = 0;
                return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
            }

            // Create new buffer for first response I block
            if ((*response) == NULL)
            {
                // Check that data available
                if (response_block.information_size == 0)
                {
                    t1prime_block_destroy(&response_block);
                    return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
                }
                *response = response_block.information;
                *response_len = response_block.information_size;
                response_block.information = NULL;
                response_block.information_size = 0;
            }
            // Otherwise append to buffer
            else
            {
                // Check for special case of forced acknowledgement
                if (response_block.information_size > 0)
                {
                    uint8_t *realloc_cache = *response;
                    *response = realloc(*response, *response_len + response_block.information_size);
                    if ((*response) == NULL)
                    {
                        free(realloc_cache);
                        *response_len = 0;
                        t1prime_block_destroy(&response_block);
                        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, OUT_OF_MEMORY);
                    }
                    memcpy((*response) + *response_len, response_block.information, response_block.information_size);
                    *response_len += response_block.information_size;
                }
            }

            t1prime_block_destroy(&response_block);
            protocol_state->receive_counter ^= 0x01;

            // Check if more data will be transmitted
            if (T1PRIME_PCB_I_HAS_MORE(response_block.pcb))
            {
                transmission_block.pcb = T1PRIME_PCB_R_ACK(protocol_state->receive_counter);
                transmission_block.information = NULL;
                transmission_block.information_size = 0;
                status = t1prime_block_transceive(self, &transmission_block, &response_block);
                t1prime_block_destroy(&transmission_block);
                if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
                {
                    if ((*response) != NULL)
                    {
                        free(*response);
                        *response = NULL;
                    }
                    *response_len = 0;
                    return status;
                }
            }
            // All data received
            else
            {
                break;
            }
        }
        // R(N(R)) -> SE needs a retransmission
        else if (T1PRIME_PCB_IS_R(response_block.pcb))
        {
            // Validate that card sent correct R(N(R))
            if (T1PRIME_PCB_R_GET_NR(response_block.pcb) != protocol_state->send_counter)
            {
                t1prime_block_destroy(&response_block);
                if ((*response) != NULL)
                {
                    free(*response);
                    *response = NULL;
                }
                *response_len = 0;
            }

            // Send retransmission request
            t1prime_block_destroy(&response_block);
            transmission_block.pcb = T1PRIME_PCB_R_ACK(protocol_state->receive_counter);
            transmission_block.information = NULL;
            transmission_block.information_size = 0;
            status = t1prime_block_transceive(self, &transmission_block, &response_block);
            t1prime_block_destroy(&transmission_block);
            if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
            {
                if ((*response) != NULL)
                {
                    free(*response);
                    *response = NULL;
                }
                *response_len = 0;
                return status;
            }
        }
        // S(ABORT request) -> end chain
        else if (response_block.pcb == T1PRIME_PCB_S_ABORT_REQ)
        {
            // Delete response cache
            if ((*response) != NULL)
            {
                free(*response);
                *response = NULL;
            }
            *response_len = 0;

            // Answer with S(ABORT response)
            transmission_block.pcb = T1PRIME_PCB_S_ABORT_RESP;
            transmission_block.information = NULL;
            transmission_block.information_size = 0;
            t1prime_block_transceive(self, &transmission_block, &response_block);
            t1prime_block_destroy(&transmission_block);
            t1prime_block_destroy(&response_block);

            return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, TRANSCEIVE_ABORTED);
        }
        else
        {
            // TODO: Handle other blocks
            t1prime_block_destroy(&response_block);
            if ((*response) != NULL)
            {
                free(*response);
                *response = NULL;
            }
            *response_len = 0;
            return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
        }
    }

    return PROTOCOL_TRANSCEIVE_SUCCESS;
}

/**
 * \brief \ref protocol_destroyfunction_t for Global Platform T=1' protocol
 *
 * \see protocol_destroyfunction_t
 */
void t1prime_destroy(Protocol *self)
{
    if (self != NULL)
    {
        if (self->_properties != NULL)
        {
            free(self->_properties);
            self->_properties = NULL;
        }
    }
}

/**
 * \brief Performs Global Platform T=1' RESYNCH operation
 *
 * \details Sends S(RESYNCH request) and expects S(RESYNCH response)
 *
 * \param self Protocol stack for performing necessary operations
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_resynch(Protocol *self)
{
    // Send S(RESYNCH request) to secure element and read back response
    Block request = {
        .nad = NAD_HD_TO_SE,
        .pcb = T1PRIME_PCB_S_RESYNCH_REQ,
        .information_size = 0,
        .information = NULL};
    Block response;
    int status = t1prime_block_transceive(self, &request, &response);
    t1prime_block_destroy(&request);
    if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    // Validate response is S(RESYNCH response)
    if (response.pcb != T1PRIME_PCB_S_RESYNCH_RESP)
    {
        t1prime_block_destroy(&response);
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    t1prime_block_destroy(&response);

    // Reset protocol state
    T1PrimeProtocolState *protocol_state;
    status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->send_counter = 0x00;
    protocol_state->receive_counter = 0x00;
    return PROTOCOL_TRANSCEIVE_SUCCESS;
}

/**
 * \brief Queries Global Platform T=1' Communication Interface Parameters (CIP)
 *
 * \details Sends S(CIP request) and expects S(CIP response)
 *
 * \param self Protocol stack for performing necessary operations
 * \param cip Buffer to store received \ref CIP in
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_cip(Protocol *self, CIP *cip)
{
    // Send S(CIP request) to secure element and read back response
    Block request = {
        .nad = NAD_HD_TO_SE,
        .pcb = T1PRIME_PCB_S_CIP_REQ,
        .information_size = 0,
        .information = NULL};
    Block response;
    int status = t1prime_block_transceive(self, &request, &response);
    t1prime_block_destroy(&request);
    if (status != PROTOCOL_TRANSMIT_SUCCESS)
    {
        return status;
    }

    // Validate response is S(CIP response) -> also implicitely checked by t1prime_block_transceive
    if (response.pcb != T1PRIME_PCB_S_CIP_RESP)
    {
        t1prime_block_destroy(&response);
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    // Decode CIP information (implicitely validating data)
    status = t1prime_cip_decode(cip, response.information, response.information_size);
    t1prime_block_destroy(&response);
    if (status != T1PRIME_CIP_DECODE_SUCCESS)
    {
        return status;
    }

    return PROTOCOL_TRANSCEIVE_SUCCESS;
}

/**
 * \brief Performs Global Platform T=1' software reset (SWR)
 *
 * \details Sends S(SWR request) and expects S(SWR response)
 *
 * \param self Protocol stack for performing necessary operations
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_swr(Protocol *self)
{
    // Send S(SWR request) to secure element and read back response
    Block request = {
        .nad = NAD_HD_TO_SE,
        .pcb = T1PRIME_PCB_S_SWR_REQ,
        .information_size = 0,
        .information = NULL};
    Block response;
    int status = t1prime_block_transceive(self, &request, &response);
    t1prime_block_destroy(&request);
    if (status != PROTOCOL_TRANSMIT_SUCCESS)
    {
        return status;
    }

    // Validate response is S(SWR response) -> also implicitely checked by t1prime_block_transceive
    if (response.pcb != T1PRIME_PCB_S_SWR_RESP)
    {
        t1prime_block_destroy(&response);
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    t1prime_block_destroy(&response);

    // Reset protocol state
    T1PrimeProtocolState *protocol_state;
    status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->send_counter = 0x00;
    protocol_state->receive_counter = 0x00;
    return PROTOCOL_TRANSCEIVE_SUCCESS;
}

/**
 * \brief Sends \ref Block to secure element
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to be send to secure element
 * \return int \c PROTOCOL_TRANSMIT_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_transmit(Protocol *self, Block *block)
{
    // Validate protocol stack
    if ((self->_base == NULL) || (self->_base->_transmit == NULL))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSMIT, INVALID_PROTOCOLSTACK);
    }

    // Validate data
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    if (block->information_size > protocol_state->ifsc)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSMIT, ILLEGAL_ARGUMENT);
    }

    // Encode block
    uint8_t *encoded;
    size_t encoded_len;
    status = t1prime_block_encode(block, &encoded, &encoded_len);
    if (status != T1PRIME_BLOCK_ENCODE_SUCCESS)
    {
        return status;
    }

    // Actually transmit block
    status = self->_base->_transmit(self->_base, encoded, encoded_len);
    free(encoded);
    return status;
}

/**
 * \brief Reads \ref Block from secure element
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to store received data in
 * \return int \c PROTOCOL_RECEIVE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_receive(Protocol *self, Block *block)
{
    // Validate protocol stack
    if ((self->_base == NULL) || (self->_base->_receive == NULL))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_PROTOCOLSTACK);
    }

    // Get protocol state for timing information
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Poll for NAD
    block->nad = 0x00;
    Timer bwt_timer;
    status = timer_set(&bwt_timer, protocol_state->bwt * 1000);
    if (status != TIMER_SET_SUCCESS)
    {
        return status;
    }
    while (!timer_has_elapsed(&bwt_timer))
    {
        // Try to read NAD
        uint8_t *nad_buffer = NULL;
        size_t nad_length;
        status = self->_base->_receive(self->_base, 1, &nad_buffer, &nad_length);
        if (status == PROTOCOL_RECEIVE_SUCCESS)
        {
            // Check if valid NAD
            if ((nad_buffer != NULL) && (nad_length == 1))
            {
                if ((nad_buffer[0] != 0x00) && (nad_buffer[0] != 0xff))
                {
                    block->nad = nad_buffer[0];
                    free(nad_buffer);
                    break;
                }
            }

            // Invalid NAD -> wait for polling time
            if ((nad_buffer != NULL) && (nad_length > 0))
            {
                free(nad_buffer);
                nad_buffer = NULL;
            }
            Timer pot_timer;
            status = timer_set(&pot_timer, protocol_state->mpot * 100);
            if (status != TIMER_SET_SUCCESS)
            {
                timer_destroy(&bwt_timer);
                return status;
            }
            timer_join(&pot_timer);
            timer_destroy(&pot_timer);
        }
    }
    timer_destroy(&bwt_timer);
    if (block->nad == 0x00)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, TOO_LITTLE_DATA);
    }

    // Read fixed length prologue
    uint8_t *binary;
    size_t binary_len;
    status = self->_base->_receive(self->_base, BLOCK_PROLOGUE_LENGTH - 1, &binary, &binary_len);
    if (status != PROTOCOL_RECEIVE_SUCCESS)
    {
        return status;
    }
    if (binary_len != (BLOCK_PROLOGUE_LENGTH - 1))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, TOO_LITTLE_DATA);
    }
    block->pcb = binary[0];
    block->information = NULL;
    block->information_size = 0;
    size_t information_size = (binary[1] << 8) | binary[2];
    free(binary);

    // Read optional dynamic length information field
    if (information_size > 0)
    {
        status = self->_base->_receive(self->_base, information_size, &block->information, &block->information_size);
        if (status != PROTOCOL_RECEIVE_SUCCESS)
        {
            return status;
        }
        if (block->information_size != information_size)
        {
            return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, TOO_LITTLE_DATA);
        }
    }
    block->information_size = information_size;

    // Read fixed length epilogue
    status = self->_base->_receive(self->_base, BLOCK_EPILOGUE_LENGTH, &binary, &binary_len);
    if (status != PROTOCOL_RECEIVE_SUCCESS)
    {
        t1prime_block_destroy(block);
        return status;
    }
    if (binary_len != BLOCK_EPILOGUE_LENGTH)
    {
        t1prime_block_destroy(block);
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, TOO_LITTLE_DATA);
    }
    uint16_t crc = (binary[0] << 8) | binary[1];
    free(binary);

    // Validate CRC
    if (t1prime_validate_crc(block, crc) != T1PRIME_VALIDATE_CRC_SUCCESS)
    {
        t1prime_block_destroy(block);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_DECODE, INVALID_CRC);
    }

    return PROTOCOL_RECEIVE_SUCCESS;
}

/**
 * \brief Sends \ref Block to secure element and reads back response block
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to be send to secure element
 * \param response_buffer Block object to store received data in
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_transceive(Protocol *self, Block *block, Block *response_buffer)
{
    // Get protocol state with information about BWT, WTX, etc
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Send and receive blocks in loop
    Block to_send = *block;
    size_t try = 0;
    do
    {
        // Send block to SE
        status = t1prime_block_transmit(self, &to_send);
        if (status != PROTOCOL_TRANSMIT_SUCCESS)
        {
            return status;
        }

        // Wait if in WTX state
        if (protocol_state->wtx_delay > 0)
        {
            Timer wtx_timer;
            status = timer_set(&wtx_timer, protocol_state->wtx_delay * 1000);
            protocol_state->wtx_delay = 0;
            if (status != TIMER_SET_SUCCESS)
            {
                return status;
            }
            timer_join(&wtx_timer);
            timer_destroy(&wtx_timer);
        }

        // Read response from SE
        status = t1prime_block_receive(self, response_buffer);

        // Validate correct block has been received
        if (status == PROTOCOL_RECEIVE_SUCCESS)
        {
            // Special case S(? request)
            if (T1PRIME_PCB_IS_S(block->pcb) && T1PRIME_PCB_S_IS_REQUEST(block->pcb))
            {
                // S(? response) must match request type
                if (T1PRIME_PCB_IS_S(response_buffer->pcb) && (!T1PRIME_PCB_S_IS_REQUEST(response_buffer->pcb)))
                {
                    if (T1PRIME_PCB_S_GET_TYPE(block->pcb) == T1PRIME_PCB_S_GET_TYPE(response_buffer->pcb))
                    {
                        return status;
                    }
                }
                // R(N(R)) must have correct sequence counter
                else if (T1PRIME_PCB_IS_R(response_buffer->pcb))
                {
                    if (T1PRIME_PCB_R_GET_NR(response_buffer->pcb) != protocol_state->send_counter)
                    {
                        t1prime_block_destroy(response_buffer);
                        return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
                    }
                }
                // I(N(S), M) invalid
                else if (T1PRIME_PCB_IS_I(response_buffer->pcb))
                {
                    t1prime_block_destroy(response_buffer);
                    return IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
                }

                // Invalidate read status
                status = IFX_ERROR(LIBT1PRIME, PROTOCOL_TRANSCEIVE, INVALID_BLOCK);
            }
            else
            {
                return status;
            }
        }

        // All blocks besides S(? request) trigger retranmsissions by sending R(N(R))
        if (!T1PRIME_PCB_IS_S(block->pcb) || !T1PRIME_PCB_S_IS_REQUEST(block->pcb))
        {
            to_send.nad = 0x21;
            to_send.pcb = T1PRIME_PCB_R_CRC(protocol_state->receive_counter);
            to_send.information = NULL;
            to_send.information_size = 0;
        }
    } while ((++try) <= T1PRIME_BLOCK_TRANSCEIVE_RETRIES);

    return status;
}

/**
 * \brief Checks that CRC matches for \ref Block object
 *
 * \param block Block object to match CRC against
 * \param expected Expected CRC to be matched.
 * \return int \c T1PRIME_VALIDATE_CRC_SUCCESS if CRC matches, any other value in case of error
 */
int t1prime_validate_crc(Block *block, uint16_t expected)
{
    // Allocate memory for CRC calculation data (prologue + information field)
    uint8_t *binary = malloc(BLOCK_PROLOGUE_LENGTH + block->information_size);
    if (binary == NULL)
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_VALIDATE_CRC, OUT_OF_MEMORY);
    }

    // Put fixed length prologue data
    binary[0] = block->nad;
    binary[1] = block->pcb;
    binary[2] = (block->information_size & 0xff00) >> 8;
    binary[3] = block->information_size & 0xff;

    // Put variable length optional information field
    if (block->information_size > 0)
    {
        memcpy(binary + 4, block->information, block->information_size);
    }

    // Actually Validate CRC
    uint16_t actual = crc16_ccitt_x25(binary, (1 + 1 + 2 + block->information_size));
    free(binary);
    if (actual != expected)
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_VALIDATE_CRC, INVALID_CRC);
    }

    return T1PRIME_VALIDATE_CRC_SUCCESS;
}

/**
 * \brief Encodes \ref Block to its binary representation
 *
 * \param block Block to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c T1PRIME_BLOCK_ENCODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_encode(Block *block, uint8_t **buffer, size_t *buffer_len)
{
    // Allocate memory for binary data
    *buffer_len = BLOCK_PROLOGUE_LENGTH + block->information_size + BLOCK_EPILOGUE_LENGTH;
    *buffer = malloc(*buffer_len);
    if (*buffer == NULL)
    {
        *buffer_len = 0;
        return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_ENCODE, OUT_OF_MEMORY);
    }

    // Encode fixed length prologue
    (*buffer)[0] = block->nad;
    (*buffer)[1] = block->pcb;
    (*buffer)[2] = (block->information_size & 0xff00) >> 8;
    (*buffer)[3] = block->information_size & 0x00ff;

    // Encode variable length optional information field
    if (block->information_size > 0)
    {
        memcpy((*buffer) + 4, block->information, block->information_size);
    }

    // Encode fixed length epilogue
    uint16_t crc = crc16_ccitt_x25(*buffer, (*buffer_len) - 2);
    (*buffer)[(*buffer_len) - 2] = (crc & 0xff00) >> 8;
    (*buffer)[(*buffer_len) - 1] = crc & 0x00ff;

    return T1PRIME_BLOCK_ENCODE_SUCCESS;
}

/**
 * \brief Decodes binary data to its member representation in \ref Block object
 *
 * \param block Block object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_BLOCK_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_decode(Block *block, uint8_t *data, size_t data_len)
{
    // Minimum length prolouge + epilogue
    if (data_len < (BLOCK_PROLOGUE_LENGTH + BLOCK_EPILOGUE_LENGTH))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_DECODE, TOO_LITTLE_DATA);
    }

    // Clear buffers just to be sure
    block->information_size = 0;
    block->information = NULL;

    // Parse prologue
    block->nad = data[0];
    block->pcb = data[1];
    block->information_size = (data[2] << 8) | data[3];

    // Check that indicated length matches actual length
    if (data_len != (BLOCK_PROLOGUE_LENGTH + (block->information_size) + BLOCK_EPILOGUE_LENGTH))
    {
        t1prime_block_destroy(block);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_DECODE, INFORMATIONSIZE_MISMATCH);
    }

    // Parse variable length optional information field
    if (block->information_size > 0)
    {
        block->information = malloc(block->information_size);
        if (block->information == NULL)
        {
            t1prime_block_destroy(block);
            return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_DECODE, OUT_OF_MEMORY);
        }
        memcpy(block->information, data + BLOCK_PROLOGUE_LENGTH, block->information_size);
    }
    else
    {
        block->information = NULL;
    }

    // Parse epilogue and validate CRC
    uint16_t crc = (data[data_len - 2] << 8) | data[data_len - 1];
    if (t1prime_validate_crc(block, crc) != T1PRIME_VALIDATE_CRC_SUCCESS)
    {
        t1prime_block_destroy(block);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_BLOCK_DECODE, INVALID_CRC);
    }
    return T1PRIME_BLOCK_DECODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref Block object (but not object itself)
 *
 * \details Block objects may contain dynamically allocated data (e.g. by \ref t1prime_block_decode(Block*, uint8_t *, size_t)).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param block Block object whose data shall be freed
 */
void t1prime_block_destroy(Block *block)
{
    if ((block->information_size != 0) && (block->information != NULL))
    {
        free(block->information);
    }
    block->information = NULL;
    block->information_size = 0;
}

/**
 * \brief Decodes binary data to its member representation in \ref CIP object
 *
 * \param cip CIP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1_CIP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_cip_decode(CIP *cip, uint8_t *data, size_t data_len)
{
    // Minimum length check
    if (data_len < (1 + 1 + 1 + 1 + 1 + 1))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, TOO_LITTLE_DATA);
    }

    // (Re-) Initialize values for safe calls to cip_destory(CIP*)
    cip->iin_len = 0;
    cip->iin = NULL;
    cip->plp_len = 0;
    cip->plp = NULL;
    cip->dllp_len = 0;
    cip->dllp = NULL;
    cip->hb_len = 0;
    cip->hb = NULL;

    // Version (1 byte)
    size_t offset = 0;
    cip->version = data[offset++];

    // Issuer identifaction number (variable length)
    cip->iin_len = data[offset++];
    if ((offset + cip->iin_len + 1 + 1 + 1 + 1) > data_len)
    {
        t1prime_cip_destroy(cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, TOO_LITTLE_DATA);
    }
    if (cip->iin_len > 0)
    {
        cip->iin = malloc(cip->iin_len);
        if (cip->iin == NULL)
        {
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, OUT_OF_MEMORY);
        }
        memcpy(cip->iin, data + offset, cip->iin_len);
        offset += cip->iin_len;
    }

    // Physical layer identifier (1 byte)
    cip->plid = data[offset++];

    // Physical layer parameters (variable length)
    cip->plp_len = data[offset++];
    if ((offset + cip->plp_len + 1 + 1) > data_len)
    {
        t1prime_cip_destroy(cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, TOO_LITTLE_DATA);
    }
    if (cip->plp_len > 0)
    {
        cip->plp = malloc(cip->plp_len);
        if (cip->plp == NULL)
        {
            t1prime_cip_destroy(cip);
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, OUT_OF_MEMORY);
        }
        memcpy(cip->plp, data + offset, cip->plp_len);
        offset += cip->plp_len;
    }

    // Data-link layer parameters (variable length)
    cip->dllp_len = data[offset++];
    if ((offset + cip->dllp_len + 1) > data_len)
    {
        t1prime_cip_destroy(cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, TOO_LITTLE_DATA);
    }
    if (cip->dllp_len > 0)
    {
        cip->dllp = malloc(cip->dllp_len);
        if (cip->dllp == NULL)
        {
            t1prime_cip_destroy(cip);
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, OUT_OF_MEMORY);
        }
        memcpy(cip->dllp, data + offset, cip->dllp_len);
        offset += cip->dllp_len;
    }

    // Historical bytes (variable length)
    cip->hb_len = data[offset++];
    if ((offset + cip->hb_len) != data_len)
    {
        t1prime_cip_destroy(cip);
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, INVALID_LENGTH);
    }
    if (cip->hb_len > 0)
    {
        cip->hb = malloc(cip->hb_len);
        if (cip->hb == NULL)
        {
            t1prime_cip_destroy(cip);
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_DECODE, OUT_OF_MEMORY);
        }
        memcpy(cip->hb, data + offset, cip->hb_len);
    }

    // Validate CIP that it matches specification
    int status = t1prime_cip_validate(cip);
    if (status != T1PRIME_CIP_VALIDATE_SUCCESS)
    {
        t1prime_cip_destroy(cip);
        return status;
    }

    return T1PRIME_CIP_DECODE_SUCCESS;
}

/**
 * \brief Validates \ref CIP object by checking all member values against GP specification
 *
 * \param cip CIP object to be validated
 * \return int \c T1PRIME_CIP_VALIDATE_SUCCESS if valid CIP, any other value in case of error
 */
int t1prime_cip_validate(CIP *cip)
{
    // Issuer identification number must be 3 or 4 bytes long
    if ((cip->iin_len < 3) || (cip->iin_len > 4))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, INVALID_LENGTH);
    }

    // SPI physical layer parameters must be at least 12 bytes long
    if (cip->plid == PLID_SPI)
    {
        if (cip->plp_len < (1 + 1 + 2 + 1 + 1 + 2 + 2 + 2))
        {
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, TOO_LITTLE_DATA);
        }
    }
    // I2C physical layer parameters must be at least 8 bytes long
    else if (cip->plid == PLID_I2C)
    {
        if (cip->plp_len < (1 + 1 + 2 + 1 + 1 + 2))
        {
            return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, TOO_LITTLE_DATA);
        }
    }
    // Unkown physical layer identifier
    else
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, INVALID_PLID);
    }

    // Data-link layer parameters must be at least 4 bytes long
    if (cip->dllp_len < (2 + 2))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_CIP_VALIDATE, TOO_LITTLE_DATA);
    }

    return T1PRIME_CIP_VALIDATE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref CIP object (but not object itself)
 *
 * \details CIP objects will most likely be populated by \ref t1prime_cip_decode(CIP*, uint8_t*, size_t).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \example
 *      uint8_t encoded[] = {...};
 *      size_t encoded_len = sizeof(encoded);
 *      CIP cip;
 *      t1prime_cip_decode(&cip, encoded, encoded_len); // Might allocate several buffers
 *      t1prime_cip_destroy(&cip);
 *
 * \param cip CIP object whose data shall be freed
 */
void t1prime_cip_destroy(CIP *cip)
{
    // Issuer identification number
    if ((cip->iin_len > 0) && (cip->iin != NULL))
    {
        free(cip->iin);
    }
    cip->iin_len = 0;
    cip->iin = NULL;

    // Physical layer parameters
    if ((cip->plp_len > 0) && (cip->plp != NULL))
    {
        free(cip->plp);
    }
    cip->plp_len = 0;
    cip->plp = NULL;

    // Data-link layer parameters
    if ((cip->dllp_len > 0) && (cip->dllp != NULL))
    {
        free(cip->dllp);
    }
    cip->dllp_len = 0;
    cip->dllp = NULL;

    // Historical bytes
    if ((cip->hb_len > 0) && (cip->hb != NULL))
    {
        free(cip->hb);
    }
    cip->hb_len = 0;
    cip->hb = NULL;
}

/**
 * \brief Decodes binary data to its member representation in \ref DLLP object
 *
 * \param dllp DLLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_DLLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_dllp_decode(DLLP *dllp, uint8_t *encoded, size_t encoded_len)
{
    if (encoded_len < (2 + 2))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_DLLP_DECODE, TOO_LITTLE_DATA);
    }
    dllp->bwt = (encoded[0] << 8) | encoded[1];
    dllp->ifsc = (encoded[2] << 8) | encoded[3];

    return T1PRIME_DLLP_DECODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref DLLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param dllp DLLP object whose data shall be freed
 */
void t1prime_dllp_destroy(DLLP *dllp)
{
}

#ifdef INTERFACE_I2C
/**
 * \brief Decodes binary data to its member representation in \ref I2CPLP object
 *
 * \param plp I2CPLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_PLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_i2c_plp_decode(I2CPLP *plp, uint8_t *encoded, size_t encoded_len)
{
    if (encoded_len < (1 + 1 + 2 + 1 + 1 + 2))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_PLP_DECODE, TOO_LITTLE_DATA);
    }
    plp->configuration = encoded[0];
    plp->pwt = encoded[1];
    plp->mcf = (encoded[2] << 8) | encoded[3];
    plp->pst = encoded[4];
    plp->mpot = encoded[5];
    plp->rwgt = (encoded[6] << 8) | encoded[7];

    return T1PRIME_PLP_DECODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref I2CPLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param plp I2CPLP object whose data shall be freed
 */
void t1prime_i2c_plp_destroy(I2CPLP *plp)
{
}
#else
/**
 * \brief Decodes binary data to its member representation in \ref SPIPLP object
 *
 * \param plp SPIPLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_PLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_spi_plp_decode(SPIPLP *plp, uint8_t *encoded, size_t encoded_len)
{
    if (encoded_len < (1 + 1 + 2 + 1 + 1 + 2 + 2 + 2))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_PLP_DECODE, TOO_LITTLE_DATA);
    }
    plp->configuration = encoded[0];
    plp->pwt = encoded[1];
    plp->mcf = (encoded[2] << 8) | encoded[3];
    plp->pst = encoded[4];
    plp->mpot = encoded[5];
    plp->segt = (encoded[6] << 8) | encoded[7];
    plp->seal = (encoded[8] << 8) | encoded[9];
    plp->wut = (encoded[10] << 8) | encoded[11];

    return T1PRIME_PLP_DECODE_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref SPIPLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param plp SPIPLP object whose data shall be freed
 */
void t1prime_spi_plp_destroy(SPIPLP *plp)
{
}
#endif

/**
 * \brief Encodes information field size (IFS) to its binary representation
 *
 * \param ifs IFS value to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Number of bytes in \p buffer
 * \return int \c T1PRIME_IFS_ENCODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_ifs_encode(size_t ifs, uint8_t **buffer, size_t *buffer_len)
{
    // Check that desired IFS value is in range
    if ((ifs == 0) || (ifs > T1PRIME_MAX_IFS))
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_SETPROPERTY, ILLEGAL_ARGUMENT);
    }

    // Allocate buffer for binary encoding
    *buffer_len = (ifs <= 0xfe) ? 1 : 2;
    *buffer = malloc(*buffer_len);
    if (*buffer == NULL)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_SETPROPERTY, OUT_OF_MEMORY);
    }

    // Actually encode data
    if ((*buffer_len) == 1)
    {
        (*buffer)[0] = ifs & 0xff;
    }
    else
    {
        (*buffer)[0] = (ifs & 0x0f00) >> 8;
        (*buffer)[1] = ifs & 0x00ff;
    }

    return T1PRIME_IFS_ENCODE_SUCCESS;
}

/**
 * \brief Decodes binary information field size (IFS)
 *
 * \param ifs_buffer Buffer to store IFS value in
 * \param data Binary IFS data
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_IFS_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_ifs_decode(size_t *ifs_buffer, uint8_t *data, size_t data_len)
{
    // Check that length of binary data is valid
    if ((data_len < 1) || (data_len > 2))
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_IFS_DECODE, ILLEGAL_ARGUMENT);
    }

    // Decode data
    if (data_len == 1)
    {
        *ifs_buffer = data[0];
    }
    else
    {
        *ifs_buffer = (data[0] << 8) | data[1];
    }

    // Validate data
    if ((*ifs_buffer) > T1PRIME_MAX_IFS)
    {
        return IFX_ERROR(LIBT1PRIME, T1PRIME_IFS_DECODE, ILLEGAL_ARGUMENT);
    }
    return T1PRIME_IFS_DECODE_SUCCESS;
}

/**
 * \brief Returns maximum information field size of the secure element (IFSC)
 *
 * \param self T=1' protocol stack to get IFSC for
 * \param ifs_buffer Buffer to store IFSC value in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_ifsc(Protocol *self, size_t *ifsc_buffer)
{
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    *ifsc_buffer = protocol_state->ifsc;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Sets maximum information field size of the host device (IFSD)
 *
 * \details Sends S(IFS request) to the secure element and expects matching S(IFS response)
 *
 * \param self T=1' protocol stack to set IFSD for
 * \param ifsd IFS value to be used
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_set_ifsd(Protocol *self, size_t ifsd)
{
    // Encode IFS information
    Block request = {
        .nad = 0x21,
        .pcb = T1PRIME_PCB_S_IFS_REQ,
        .information_size = 0,
        .information = NULL};
    int status = t1prime_ifs_encode(ifsd, &(request.information), &(request.information_size));
    if (status != T1PRIME_IFS_ENCODE_SUCCESS)
    {
        return status;
    }

    // Send S(IFS request) to secure element and read back response
    Block response;
    status = t1prime_block_transceive(self, &request, &response);
    t1prime_block_destroy(&request);
    if (status != PROTOCOL_TRANSCEIVE_SUCCESS)
    {
        return status;
    }

    // Validate response is S(IFS response) -> also implicitely checked by t1prime_block_transceive
    if (response.pcb != T1PRIME_PCB_S_IFS_RESP)
    {
        t1prime_block_destroy(&response);
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    // Decode IFS response
    size_t response_ifs;
    status = t1prime_ifs_decode(&response_ifs, response.information, response.information_size);
    t1prime_block_destroy(&response);
    if (status != T1PRIME_IFS_DECODE_SUCCESS)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    // Check that negotiated value matches
    if (response_ifs != ifsd)
    {
        return IFX_ERROR(LIBT1PRIME, PROTOCOL_RECEIVE, INVALID_BLOCK);
    }

    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Returns current block waiting time (BWT) in [ms]
 *
 * \param self T=1' protocol stack to get BWT for
 * \param bwt_buffer Buffer to store BWT value in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_bwt(Protocol *self, uint16_t *bwt_buffer)
{
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    *bwt_buffer = protocol_state->bwt;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Sets block waiting time (BWT) in [ms]
 *
 * \param self T=1' protocol stack to set BWT for
 * \param bwt BWT value to be used
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_set_bwt(Protocol *self, uint16_t bwt)
{
    T1PrimeProtocolState *protocol_state;
    int status = t1prime_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->bwt = bwt;
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Returns current protocol state for Global Platform T=1' protocol
 *
 * \param self T=1' protocol stack to get protocol state for
 * \param protocol_state_buffer Buffer to store protocol state in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_protocol_state(Protocol *self, T1PrimeProtocolState **protocol_state_buffer)
{
    // Verify that correct protocol layer called this function
    if (self->_layer_id != T1PRIME_PROTOCOLLAYER_ID)
    {
        if (self->_base == NULL)
        {
            return IFX_ERROR(LIBT1PRIME, PROTOCOL_GETPROPERTY, INVALID_PROTOCOLSTACK);
        }
        return t1prime_get_protocol_state(self->_base, protocol_state_buffer);
    }

    // Check if protocol state has been initialized
    if (self->_properties == NULL)
    {
        // Lazy initialize properties
        self->_properties = malloc(sizeof(T1PrimeProtocolState));
        if (self->_properties == NULL)
        {
            return IFX_ERROR(LIBT1PRIME, PROTOCOL_GETPROPERTY, OUT_OF_MEMORY);
        }
        T1PrimeProtocolState *properties = (T1PrimeProtocolState *)self->_properties;
        properties->bwt = T1PRIME_DEFAULT_BWT;
        properties->ifsc = T1PRIME_MAX_IFS;
        properties->send_counter = 0x00;
        properties->receive_counter = 0x00;
        properties->wtx_delay = 0x00;
#ifdef INTERFACE_I2C
        properties->mpot = T1PRIME_DEFAULT_I2C_MPOT;
#else
        properties->mpot = T1PRIME_DEFAULT_SPI_MPOT;
#endif
    }

    *protocol_state_buffer = (T1PrimeProtocolState *)self->_properties;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}
