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
 * \file t1prime/datastructures.h
 * \brief Global Platform T=1' data structure definitions
 */
#ifndef _T1PRIME_DATASTRUCTURES_H_
#define _T1PRIME_DATASTRUCTURES_H_

#include <stdint.h>
#include "ifx/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

#pragma region BLOCK
/**
 * \brief Return code for successful calls to \ref t1prime_block_encode(Block*, uint8_t**, size_t*)
 */
#define T1PRIME_BLOCK_ENCODE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_block_encode(Block*, uint8_t**, size_t*)
 */
#define T1PRIME_BLOCK_ENCODE 0x70

/**
 * \brief Return code for successful calls to \ref t1prime_block_decode(Block*, uint8_t*, size_t)
 */
#define T1PRIME_BLOCK_DECODE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_block_decode(Block*, uint8_t*, size_t)
 */
#define T1PRIME_BLOCK_DECODE 0x60

/**
 * \brief Error reason if information size does not match length of data in \ref t1prime_block_decode(Block*, uint8_t*, size_t)
 */
#define INFORMATIONSIZE_MISMATCH 0x01

/**
 * \brief Error reason if CRC does not match data in \ref t1prime_block_decode(Block*, uint8_t*, size_t)
 */
#define INVALID_CRC 0x02

/**
 * \brief Fixed number of bytes in \ref Block prologue.
 */
#define BLOCK_PROLOGUE_LENGTH (1 + 1 + 2)

/**
 * \brief Fixed number of bytes in \ref Block epilogue.
 */
#define BLOCK_EPILOGUE_LENGTH 2

/**
 * \brief Data storage for a Global Platform T=1' block.
 */
typedef struct Block
{
    uint8_t nad; /**< Node Address (NAD) routing information */
    uint8_t pcb; /**< Protocol Control Byte (PCB) */
    size_t information_size; /**< Number of bytes in \ref Block.information */
    uint8_t *information; /**< Actual block data */
} Block;

/**
 * \brief Decodes binary data to its member representation in \ref Block object
 *
 * \param block Block object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_BLOCK_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_decode(Block *block, uint8_t *data, size_t data_len);

/**
 * \brief Encodes \ref Block to its binary representation
 *
 * \param block Block to be encoded
 * \param buffer Buffer to store encoded data in
 * \param buffer_len Pointer for storing number of bytes in \p buffer
 * \return int \c T1PRIME_BLOCK_ENCODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_block_encode(Block *block, uint8_t **buffer, size_t *buffer_len);

/**
 * \brief Frees memory associated with \ref Block object (but not object itself)
 *
 * \details Block objects may contain dynamically allocated data (e.g. by \ref t1prime_block_decode(Block*, uint8_t *, size_t)).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param block Block object whose data shall be freed
 */
void t1prime_block_destroy(Block *block);

/**
 * \brief IFX error encoding function identifier for \ref t1prime_validate_crc(Block*, uint16_t)
 */
#define T1PRIME_VALIDATE_CRC 0x50

/**
 * \brief Return code for successful calls to \ref t1prime_validate_crc(Block*, uint16_t)
 */
#define T1PRIME_VALIDATE_CRC_SUCCESS SUCCESS

/**
 * \brief Checks that CRC matches for \ref Block object
 *
 * \param block Block object to match CRC against
 * \param expected Expected CRC to be matched.
 * \return int \c T1PRIME_VALIDATE_CRC_SUCCESS if CRC matches, any other value in case of error
 */
int t1prime_validate_crc(Block *block, uint16_t expected);
#pragma endregion BLOCK

#pragma region CIP
/**
 * \brief Return code for successful calls to \ref t1prime_cip_decode(CIP*, uint8_t*, size_t)
 */
#define T1PRIME_CIP_DECODE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_cip_decode(CIP*, uint8_t*, size_t)
 */
#define T1PRIME_CIP_DECODE 0x30

/**
 * \brief Return code for successful calls to \ref t1prime_cip_validate(CIP*)
 */
#define T1PRIME_CIP_VALIDATE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_cip_validate(CIP*)
 */
#define T1PRIME_CIP_VALIDATE 0x31

/**
 * \brief Error reason if any length information does not match during \ref t1prime_cip_decode(CIP*, uint8_t*, size_t)
 */
#define INVALID_LENGTH 0x01

/**
 * \brief Error reason if invalid physical layer identifier detected during t1prime_cip_decode(CIP*, uint8_t*, size_t)
 */
#define INVALID_PLID 0x02

/**
 * \brief Physical layer identifier for SPI in \ref CIP
 */
#define PLID_SPI 0x01

/**
 * \brief Physical layer identifier for SPI in \ref CIP
 */
#define PLID_I2C 0x02

/**
 * \brief Data storage for a Global Platform T=1' Communication Interface Parameters (CIP).
 */
typedef struct CIP
{
    uint8_t version;    /**< Protocol version */
    uint8_t iin_len;    /**< Number of bytes in \ref CIP.iin */
    uint8_t *iin;       /**< Issuer identification number */
    uint8_t plid;       /**< Physical Layer Identifier */
    uint8_t plp_len;    /**< Number of bytes in \ref CIP.plp */
    uint8_t *plp;       /**< Physical layer parameters */
    uint8_t dllp_len;   /**< Number of bytes in \ref CIP.dllp */
    uint8_t *dllp;      /**< Data-link layer parameters */
    uint8_t hb_len;     /**< Number of bytes in \ref CIP.hb */
    uint8_t *hb;        /**< Historical bytes */
} CIP;

/**
 * \brief Decodes binary data to its member representation in \ref CIP object
 *
 * \param cip CIP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_CIP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_cip_decode(CIP *cip, uint8_t *data, size_t data_len);

/**
 * \brief Validates \ref CIP object by checking all member values against GP specification
 *
 * \param cip CIP object to be validated
 * \return int \c T1PRIME_CIP_VALIDATE_SUCCESS if valid CIP, any other value in case of error
 */
int t1prime_cip_validate(CIP *cip);

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
void t1prime_cip_destroy(CIP *cip);

/**
 * \brief Return code for successful calls to \ref t1prime_dllp_decode(DLLP*, uint8_t*, size_t)
 */
#define T1PRIME_DLLP_DECODE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_dllp_decode(CIP*, uint8_t*, size_t)
 */
#define T1PRIME_DLLP_DECODE 0x32

/**
 * \brief Data storage for a Global Platform T=1' data-link layer parameters (DLLP)
 */
typedef struct DLLP
{
    uint16_t bwt;   /**< Block waiting time in [ms] */
    uint16_t ifsc;  /**< Maximum information field size of secure element (initial value) */
} DLLP;

/**
 * \brief Decodes binary data to its member representation in \ref DLLP object
 *
 * \param dllp DLLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_DLLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_dllp_decode(DLLP *dllp, uint8_t *encoded, size_t encoded_len);

/**
 * \brief Frees memory associated with \ref DLLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param dllp DLLP object whose data shall be freed
 */
void t1prime_dllp_destroy(DLLP *dllp);

/**
 * \brief Return code for successful calls to \ref t1prime_i2c_plp_decode(I2CPLP*, uint8_t*, size_t) and \ref t1prime_spi_plp_decode(SPIPLP*, uint8_t*, size_t)
 */
#define T1PRIME_PLP_DECODE_SUCCESS SUCCESS

/**
 * \brief IFX error code function identifier for \ref t1prime_i2c_plp_decode(I2CPLP*, uint8_t*, size_t) and \ref t1prime_spi_plp_decode(SPIPLP*, uint8_t*, size_t)
 */
#define T1PRIME_PLP_DECODE 0x33

#ifdef INTERFACE_I2C
/**
 * \brief Data storage for a Global Platform T=1' I2C phyiscal layer parameters
 */
typedef struct I2CPLP
{
    uint8_t configuration;  /**< RFU */
    uint8_t pwt;            /**< Power wake-up time in [ms] */
    uint16_t mcf;           /**< Maximum clock frequency in [kHz] */
    uint8_t pst;            /**< Power saving timeout in [ms] */
    uint8_t mpot;           /**< Minimum polling time in [multiples of 100us] */
    uint16_t rwgt;          /**< Read / Write guart time in [us] */
} I2CPLP;

/**
 * \brief Decodes binary data to its member representation in \ref I2CPLP object
 *
 * \param plp I2CPLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_PLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_i2c_plp_decode(I2CPLP *plp, uint8_t *encoded, size_t encoded_len);

/**
 * \brief Frees memory associated with \ref I2CPLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param plp I2CPLP object whose data shall be freed
 */
void t1prime_i2c_plp_destroy(I2CPLP *plp);

#else
/**
 * \brief Data storage for a Global Platform T=1' SPI phyiscal layer parameters
 */
typedef struct SPIPLP
{
    uint8_t configuration;  /**< RFU */
    uint8_t pwt;            /**< Power wake-up time in [ms] */
    uint16_t mcf;           /**< Maximum clock frequency in [kHz] */
    uint8_t pst;            /**< Power saving timeout in [ms] */
    uint8_t mpot;           /**< Minimum polling time in [multiples of 100us] */
    uint16_t segt;          /**< Secure element guard time in [ms] */
    uint16_t seal;          /**< Maximum secure element access length in [byte] */
    uint16_t wut;           /**< Wake-up time */
} SPIPLP;

/**
 * \brief Decodes binary data to its member representation in \ref SPIPLP object
 *
 * \param plp SPIPLP object to store values in
 * \param data Binary data to be decoded
 * \param data_len Number of bytes in \p data
 * \return int \c T1PRIME_PLP_DECODE_SUCCESS if successful, any other value in case of error
 */
int t1prime_spi_plp_decode(SPIPLP *plp, uint8_t *encoded, size_t encoded_len);

/**
 * \brief Frees memory associated with \ref SPIPLP object (but not object itself)
 *
 * \details This function does nothing at the moment but keeps the interface uniform accross all datastructures.
 *
 * \param plp SPIPLP object whose data shall be freed
 */
void t1prime_spi_plp_destroy(SPIPLP *plp);
#endif
#pragma endregion CIP

/**
 * \brief State of T=1' protocol keeping track of sequence counters, information field sizes, etc.
 */
typedef struct
{
    uint16_t bwt; /**< Current Block Waiting Time in [ms] */
    uint8_t mpot; /**< Minimum Polling Time in [multiple of 100us] */
    size_t ifsc; /**< Current maximum size of SE information field in [byte] */
    uint8_t send_counter; /**< Current sequence counter of transmitted I blocks */
    uint8_t receive_counter; /**< Current sequence counter of received I blocks */

    size_t wtx_delay; /**< Waiting time extension delay between transmission and reception */
} T1PrimeProtocolState;

#ifdef __cplusplus
}
#endif

#endif // _T1PRIME_DATASTRUCTURES_H_
