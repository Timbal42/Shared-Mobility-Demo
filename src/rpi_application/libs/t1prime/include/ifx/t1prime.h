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
 * \file ifx/t1prime.h
 * \brief Global Platform T=1' protocol
 */
#ifndef _IFX_T1PRIME_H_
#define _IFX_T1PRIME_H_

#include "ifx/error.h"
#include "ifx/protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error code module identifer
 */
#define LIBT1PRIME 0x21

/**
 * \brief Initializes Protocol object for Global Platform T=1' protocol.
 *
 * \param self Protocol object to be initialized.
 * \param driver Physical (driver) layer used for communication.
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
int t1prime_initialize(Protocol *self, Protocol *driver);

/**
 * \brief Sets maximum information field size of the host device (IFSD)
 *
 * \param self T=1' protocol stack to set IFSD for
 * \param ifsd IFS value to be used
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_set_ifsd(Protocol *self, size_t ifsd);

/**
 * \brief Returns current block waiting time (BWT) in [ms]
 *
 * \param self T=1' protocol stack to get BWT for
 * \param bwt_buffer Buffer to store BWT value in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_get_bwt(Protocol *self, uint16_t *bwt_buffer);

/**
 * \brief Sets block waiting time (BWT) in [ms]
 *
 * \param self T=1' protocol stack to set BWT for
 * \param bwt BWT value to be used
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int t1prime_set_bwt(Protocol *self, uint16_t bwt);

#ifdef __cplusplus
}
#endif

#endif // _IFX_T1PRIME_H_
