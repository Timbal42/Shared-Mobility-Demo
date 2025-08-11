/**
 * \file ifx/aardvark-i2c.h
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief I2C driver implementation for TotalPhase Aardvark I2C reader on Windows
 */
#ifndef _IFX_AARDVARK_I2C_H_
#define _IFX_AARDVARK_I2C_H_

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
#define LIBAARDVARKI2C 0x34

/**
 * \brief Initializes \ref Protocol object for TotalPhase Aardvark I2C driver.
 * 
 * \param self \ref Protocol object to be initialized.
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
int aardvark_i2c_initialize(Protocol* self);

/**
 * \brief Getter for I2C bus timeout (BTO) in [ms]
 * 
 * \param self Aardvark I2C driver layer to get BTO for
 * \param bto_buffer Buffer to store BTO in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_get_bto(Protocol *self, uint16_t *bto_buffer);

/**
 * \brief Sets I2C bus timeout (BTO) in [ms]
 * 
 * \param self Aardvark I2C driver layer to set BTO for
 * \param bto Desired BTO
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_bto(Protocol *self, uint16_t bto);

#ifdef __cplusplus
}
#endif

#endif // _IFX_AARDVARK_I2C_H_