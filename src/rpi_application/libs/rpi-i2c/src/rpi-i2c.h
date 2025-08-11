/**
 * \file rpi-i2c.h
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Internal definitions for Raspberry Pi I2C driver layer
 */
#ifndef _RPI_I2C_H_
#define _RPI_I2C_H_

#include <stdint.h>
#include "bcm2835.h"
#include "ifx/protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief State of I2C driver layer
 */
typedef struct ProtocolState
{
    uint16_t slave_address; /**< I2C address currently in use */
    uint32_t clock_frequency; /**< I2C clock frequency in [Hz] */
} ProtocolState;

/**
 * \brief Protocol Layer ID for Raspberry Pi I2C driver layer
 * 
 * \details Used to verify that correct protocol layer has called member functionality
 */
#define RPI_I2C_PROTOCOLLAYER_ID 0x34

/**
 * \brief Default value for I2C address used
 */
#define I2C_DEFAULT_SLAVE_ADDRESS ((uint16_t) 0x10)

/**
 * \brief Default value for I2C clock frequency in [Hz]
 */
#define I2C_DEFAULT_CLOCK_FREQUENCY ((uint32_t) 100000)

/**
 * \brief \ref protocol_activatefunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_activatefunction_t
 */
int rpi_i2c_activate(Protocol *self, uint8_t **response, size_t *response_len);

/**
 * \brief \ref protocol_transmitfunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_transmitfunction_t 
 */
int rpi_i2c_transmit(Protocol *self, uint8_t *data, size_t data_len);

/**
 * \brief \ref protocol_receivefunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_receivefunction_t 
 */
int rpi_i2c_receive(Protocol *self, size_t expected_len, uint8_t **response, size_t *response_len);

/**
 * \brief \ref protocol_destroyfunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_destroyfunction_t 
 */
void rpi_i2c_destroy(Protocol *self);


#ifdef __cplusplus
}
#endif

#endif // _RPI_I2C_H_
