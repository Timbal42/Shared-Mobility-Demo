/**
 * \file ifx/i2c.h
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Generic API for I2C drivers
 */
#ifndef _IFX_I2C_H_
#define _IFX_I2C_H_

#include <stdint.h>
#include "ifx/protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error code module identifer
 */
#define LIBI2C 0x23

/**
 * \brief Getter for I2C clock frequency in [Hz]
 * 
 * \param self Protocol object to get clock frequency for
 * \param frequency_buffer Buffer to store clock frequency in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_get_clock_frequency(Protocol *self, uint32_t *frequency_buffer);

/**
 * \brief Sets I2C clock frequency in [Hz]
 * 
 * \param self Protocol object to set clock frequency for
 * \param frequency Desired clock frequency in [Hz]
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_set_clock_frequency(Protocol *self, uint32_t frequency);

/**
 * \brief Getter for I2C slave address
 * 
 * \param self Protocol object to get I2C slave address for
 * \param address_buffer Buffer to store I2C address in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_get_slave_address(Protocol *self, uint16_t *address_buffer);

/**
 * \brief Sets I2C slave address
 * 
 * \param self Protocol object to set I2C slave address for
 * \param address Desired I2C slave address
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_set_slave_address(Protocol *self, uint16_t address);

#ifdef __cplusplus
}
#endif

#endif // _IFX_I2C_H_
