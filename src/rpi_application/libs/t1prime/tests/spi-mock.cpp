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
 * \file spi-mock.cpp
 * \brief Mock implementation of SPI interface
 */
#include "ifx/spi.h"

/**
 * Mock implementation of SPI interface function.
 */
int spi_get_clock_frequency(Protocol *self, uint32_t *frequency_buffer)
{
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_set_clock_frequency(Protocol *self, uint32_t frequency)
{
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_set_clock_polarity(Protocol *self, bool cpol)
{
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_set_clock_phase(Protocol *self, bool cpha)
{
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_get_buffer_size(Protocol *self, size_t *buffer_size_buffer)
{
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_set_buffer_size(Protocol *self, size_t buffer_size)
{
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_get_guard_time(Protocol *self, uint32_t *guard_time_buffer)
{
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * Mock implementation of SPI interface function.
 */
int spi_set_guard_time(Protocol *self, uint32_t guard_time)
{
    return PROTOCOL_SETPROPERTY_SUCCESS;
}
