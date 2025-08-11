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
 * \file t1prime.h
 * \brief Internal definitions for Global Platform T=1' protocol
 */
#ifndef _T1PRIME_H_
#define _T1PRIME_H_

#include <stdint.h>
#include "ifx/protocol.h"
#include "t1prime/datastructures.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Protocol Layer ID for Global Platform T=1' protocol
 *
 * \details Used to verify that correct protocol layer has called member functionality
 */
#define T1PRIME_PROTOCOLLAYER_ID 0x01

#pragma region PCB
/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) of given type
 *
 * \param type Type of PCB
 * \return uint8_t PCB to be used in \ref Block
 */
#define T1PRIME_PCB(type)                   ((type) & 0xff)

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for I blocks
 *
 * \param ns Send sequence counter of I block
 * \param m \c true if I block shall have \c more bit set
 * \return uint8_t I block PCB to be used in \ref Block
 */
#define T1PRIME_PCB_I(ns, m)                (T1PRIME_PCB(((ns) ? 0x40 : 0x00) | ((m) ? 0x20 : 0x00)))

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) indicates an I block
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates an I block
 */
#define T1PRIME_PCB_IS_I(pcb)               ((T1PRIME_PCB((pcb)) & 0x80) == 0x00)

/**
 * \brief Gets sequence counter for Global Platform T=1' Protocol Control Byte (PCB) for I blocks
 *
 * \param pcb PCB to be checked
 * \return int Sequence counter from I block
 */
#define T1PRIME_PCB_I_GET_NS(pcb)           ((T1PRIME_PCB((pcb)) & 0x40) >> 6)

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) for I blocks indicates more data will follow
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates more data
 */
#define T1PRIME_PCB_I_HAS_MORE(pcb)         ((T1PRIME_PCB((pcb)) & 0x20) == 0x20)

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for R blocks
 *
 * \param ns Receive sequence counter of R block
 * \param type Type of R block to be indicated by PCB
 * \return uint8_t R block PCB to be used in \ref Block
 */
#define T1PRIME_PCB_R(nr, type)             (T1PRIME_PCB(0x80 | ((nr) ? 0x10 : 0x00) | ((type) & 0x0f)))

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) indicates a R block
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates a R block
 */
#define T1PRIME_PCB_IS_R(pcb)               ((T1PRIME_PCB((pcb)) & 0xC0) == 0x80)

/**
 * \brief Gets sequence counter for Global Platform T=1' Protocol Control Byte (PCB) for R blocks
 *
 * \param pcb PCB to be checked
 * \return int Sequence counter from R block
 */
#define T1PRIME_PCB_R_GET_NR(pcb)           ((T1PRIME_PCB((pcb)) & 0x10) >> 4)

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for error-free acknowledgement R blocks
 *
 * \param nr Receive sequence counter of R block
 * \return uint8_t R block PCB to be used by \ref Block
 */
#define T1PRIME_PCB_R_ACK(nr)               T1PRIME_PCB_R((nr), 0x00)

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) indicates an error free acknowledgement R block
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates an error free acknowledgement R block
 */
#define T1PRIME_PCB_IS_R_ACK(pcb)           (T1PRIME_PCB_IS_R(pcb) && (((pcb) & 0x0f) == 0x00))

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for CRC error R blocks
 *
 * \param nr Receive sequence counter of R block
 * \return uint8_t R block PCB to be used by \ref Block
 */
#define T1PRIME_PCB_R_CRC(nr)               T1PRIME_PCB_R((nr), 0x01)

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for other error R blocks
 *
 * \param nr Receive sequence counter of R block
 * \return uint8_t R block PCB to be used by \ref Block
 */
#define T1PRIME_PCB_R_ERROR(nr)             T1PRIME_PCB_R((nr), 0x02)

/**
 * \brief Creates Global Platform T=1' Protocol Control Byte (PCB) for S blocks
 *
 * \param type Type of S block to be indicated by PCB
 * \param is_response \c true if PCB shall indicate response S block
 * \return uint8_t S block PCB to be used in \ref Block
 */
#define T1PRIME_PCB_S(type, is_response)    T1PRIME_PCB(0xC0 | ((is_response) ? 0x20 : 0x00) | ((type) & 0x0f))

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) indicates a S block
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates a S block
 */
#define T1PRIME_PCB_IS_S(pcb)               ((T1PRIME_PCB((pcb)) & 0xC0) == 0xC0)

/**
 * \brief Checks if Global Platform T=1' Protocol Control Byte (PCB) indicates a request S block
 *
 * \param pcb PCB to be checked
 * \return bool \c true if PCB indicates a request S block
 */
#define T1PRIME_PCB_S_IS_REQUEST(pcb)       ((T1PRIME_PCB((pcb)) & 0x20) == 0x00)

/**
 * \brief Gets type of S(? ?) block from Global Platform T=1' Protocol Control Byte (PCB)
 *
 * \param pcb PCB to get S block type for
 * \return int S block type
 */
#define T1PRIME_PCB_S_GET_TYPE(pcb)           (T1PRIME_PCB((pcb)) & 0x1f)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(RESYNCH request) blocks
 */
#define T1PRIME_PCB_S_RESYNCH_REQ           T1PRIME_PCB_S(0x0, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(RESYNCH response) blocks
 */
#define T1PRIME_PCB_S_RESYNCH_RESP          T1PRIME_PCB_S(0x0, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(IFS request) blocks
 */
#define T1PRIME_PCB_S_IFS_REQ               T1PRIME_PCB_S(0x1, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(IFS response) blocks
 */
#define T1PRIME_PCB_S_IFS_RESP              T1PRIME_PCB_S(0x1, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(ABORT request) blocks
 */
#define T1PRIME_PCB_S_ABORT_REQ             T1PRIME_PCB_S(0x2, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(ABORT response) blocks
 */
#define T1PRIME_PCB_S_ABORT_RESP            T1PRIME_PCB_S(0x2, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(WTX request) blocks
 */
#define T1PRIME_PCB_S_WTX_REQ               T1PRIME_PCB_S(0x3, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(WTX response) blocks
 */
#define T1PRIME_PCB_S_WTX_RESP              T1PRIME_PCB_S(0x3, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(CIP request) blocks
 */
#define T1PRIME_PCB_S_CIP_REQ               T1PRIME_PCB_S(0x4, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(CIP response) blocks
 */
#define T1PRIME_PCB_S_CIP_RESP              T1PRIME_PCB_S(0x4, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(RELEASE request) blocks
 */
#define T1PRIME_PCB_S_RELEASE_REQ           T1PRIME_PCB_S(0x6, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(RELEASE response) blocks
 */
#define T1PRIME_PCB_S_RELEASE_RESP          T1PRIME_PCB_S(0x6, true)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(SWR request) blocks
 */
#define T1PRIME_PCB_S_SWR_REQ               T1PRIME_PCB_S(0xf, false)

/**
 * \brief Global Platform T=1' Protocol Control Byte (PCB) for S(SWR response) blocks
 */
#define T1PRIME_PCB_S_SWR_RESP              T1PRIME_PCB_S(0xf, true)
#pragma endregion PCB

#pragma region PROTOCOL_MEMBER_FUNCTIONS
/**
 * \brief \ref protocol_activatefunction_t for Global Platform T=1' protocol
 *
 * \see protocol_activatefunction_t
 */
int t1prime_activate(Protocol *self, uint8_t **response, size_t *response_len);

/**
 * \brief Error reason if secure element aborted transmission during \ref t1prime_transceive(Protocol*, uint8_t*, size_t, uint8_t**, size_t*)
 */
#define TRANSCEIVE_ABORTED 0x60

/**
 * \brief \ref protocol_transceivefunction_t for Global Platform T=1' protocol
 *
 * \see protocol_transceivefunction_t
 */
int t1prime_transceive(Protocol *self, uint8_t *data, size_t data_len, uint8_t **response, size_t *response_len);

/**
 * \brief \ref protocol_destroyfunction_t for Global Platform T=1' protocol
 *
 * \see protocol_destroyfunction_t
 */
void t1prime_destroy(Protocol *self);
#pragma endregion PROTOCOL_MEMBER_FUNCTIONS

#pragma region BLOCK_TRANSMISSION
/**
 * \brief Error reason if invalid \ref Block received in any secure element interaction.
 */
#define INVALID_BLOCK 0x61

/**
 * \brief Node address byte (NAD) for transmission from host device to secure element
 */
#define NAD_HD_TO_SE 0x21

/**
 * \brief Sends \ref Block to secure element
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to be send to secure element
 * \return int \c PROTOCOL_TRANSMIT_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_transmit(Protocol *self, Block *block);

/**
 * \brief Reads \ref Block from secure element
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to store received data in
 * \return int \c PROTOCOL_RECEIVE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_receive(Protocol *self, Block *block);

/**
 * \brief Number of read retries after which \ref t1prime_block_transceive(Protocol*, Block*, Block*) shall fail
 */
#define T1PRIME_BLOCK_TRANSCEIVE_RETRIES 2

/**
 * \brief Sends \ref Block to secure element and reads back response block
 *
 * \param self Protocol stack for performing necessary operations
 * \param block Block object to be send to secure element
 * \param response_buffer Block object to store received data in
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_transceive(Protocol *self, Block *block, Block *response_buffer);

/**
 * \brief Performs Global Platform T=1' RESYNCH operation
 *
 * \details Sends S(RESYNCH request) and expects S(RESYNCH response)
 *
 * \param self Protocol stack for performing necessary operations
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_resynch(Protocol *self);

/**
 * \brief Queries Global Platform T=1' Communication Interface Parameters (CIP)
 *
 * \details Sends S(CIP request) and expects S(CIP response)
 *
 * \param self Protocol stack for performing necessary operations
 * \param cip Buffer to store received \ref CIP in
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_cip(Protocol *self, CIP *cip);

/**
 * \brief Performs Global Platform T=1' software reset (SWR)
 *
 * \details Sends S(SWR request) and expects S(SWR response)
 *
 * \param self Protocol stack for performing necessary operations
 * \return int \c PROTOCOL_TRANSCEIVE_SUCCESS if successful, any other value in case of error
 */
int s_swr(Protocol *self);
#pragma endregion BLOCK_TRANSMISSION

#pragma region PROTOCOL_PROPERTIES
#ifdef INTERFACE_I2C
/**
 * \brief Default I2C clock frequency in [Hz]
 */
#define T1PRIME_DEFAULT_I2C_CLOCK_FREQUENCY 400000

/**
 * \brief Default I2C minimum polling time in [multiple of 100us]
 */
#define T1PRIME_DEFAULT_I2C_MPOT 10
#else
/**
 * \brief Default SPI clock frequency in [Hz]
 */
#define T1PRIME_DEFAULT_SPI_CLOCK_FREQUENCY 1000000

/**
 * \brief Default SPI secure element guard time (SEGT) in [us]
 */
#define T1PRIME_DEFAULT_SPI_SEGT 200

/**
 * \brief Default SPI secure element access length (SEAL) in [byte]
 */
#define T1PRIME_DEFAULT_SPI_SEAL 16

/**
 * \brief Default SPI minimum polling time in [multiple of 100us]
 */
#define T1PRIME_DEFAULT_SPI_MPOT 10
#endif

/**
 * \brief Maximum allowed information field size
 */
#define T1PRIME_MAX_IFS 0xff9

/**
 * \brief Default value for current maximum information field size (IFS)
 */
#define T1PRIME_DEFAULT_IFSC 0x08

/**
 * \brief Default value for current block waiting time in [ms]
 */
#define T1PRIME_DEFAULT_BWT ((uint16_t) 300)

/**
 * \brief Returns current protocol state for Global Platform T=1' protocol
 *
 * \param self T=1' protocol stack to get protocol state for
 * \param protocol_state_buffer Buffer to store protocol state in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_protocol_state(Protocol *self, T1PrimeProtocolState **protocol_state_buffer);

/**
 * \brief Returns maximum information field size of the secure element (IFSC)
 *
 * \param self T=1' protocol stack to get IFSC for
 * \param ifs_buffer Buffer to store IFSC value in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_ifsc(Protocol *self, size_t *ifsc_buffer);

/**
 * \brief IFX error code function identifier for \ref t1prime_ifs_encode(size_t, uint8_t**, size_t*)
 */
#define T1PRIME_IFS_ENCODE 0x35

/**
 * \brief Return code for successful calls to \ref t1prime_ifs_encode(size_t, uint8_t**, size_t*)
 */
#define T1PRIME_IFS_ENCODE_SUCCESS SUCCESS

/**
 * \brief Encodes information field size (IFS) to its binary representation
 *
 * \param ifs IFS value to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Number of bytes in \p buffer
 * \return int \c T1PRIME_IFS_ENCODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_ifs_encode(size_t ifs, uint8_t **buffer, size_t *buffer_len);

/**
 * \brief IFX error code function identifier for \ref t1prime_ifs_decode(size_t*, uint8_t*, size_t)
 */
#define T1PRIME_IFS_DECODE 0x34

/**
 * \brief Return code for successful calls to \ref t1prime_ifs_decode(size_t*, uint8_t*, size_t)
 */
#define T1PRIME_IFS_DECODE_SUCCESS SUCCESS

/**
 * \brief Decodes binary information field size (IFS)
 *
 * \param ifs_buffer Buffer to store IFS value in
 * \param data Binary IFS data
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_IFS_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_ifs_decode(size_t* ifs_buffer, uint8_t *data, size_t data_len);

#pragma endregion PROTOCOL_PROPERTIES

#ifdef __cplusplus
}
#endif

#endif // _T1PRIME_H_
