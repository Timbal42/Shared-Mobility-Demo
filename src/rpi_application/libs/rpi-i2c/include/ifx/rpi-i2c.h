/**
 * \file ifx/rpi-i2c.h
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Raspberry Pi I2C driver implementation
 */
#ifndef _IFX_RPI_I2C_H_
#define _IFX_RPI_I2C_H_

#include "bcm2835.h"
#include "ifx/error.h"
#include "ifx/protocol.h"
#include "ifx/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error code module identifer
 */
#define LIBRPII2C 0x35

/**
 * \brief Initializes \ref Protocol object for Raspberry Pi I2C driver layer
 * 
 * \param self \ref Protocol object to be initialized.
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
int rpi_i2c_initialize(Protocol* self);

#ifdef __cplusplus
}
#endif

#endif // _IFX_RPI_I2C_H_