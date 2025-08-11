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
 * \file logger.c
 * \brief Generic logging API
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "ifx/logger.h"

/**
 * \brief Initializes \ref Logger object by setting all members to valid (but potentially unusable) values
 *
 * \details Concrete implementations can call this function to ensure all members are set to their correct default values before populating the required fields.
 *
 * \param self \ref Logger object to be initialized
 * \return int \c LOGGER_INITIALIZE_SUCCESS if successful, any other value in case of error
 */
int logger_initialize(Logger *self)
{
    // Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_INITIALIZE, ILLEGAL_ARGUMENT);
    }

    // Populate values
    self->_log = NULL;
    self->_destructor = NULL;
    self->_level = LOG_FATAL;
    self->_data = NULL;

    return LOGGER_INITIALIZE_SUCCESS;
}

/**
 * \brief Log formatted message
 *
 * \details Uses printf syntax for message formatting.
 *
 * \example logger_log(&logger, "example", LOG_INFO, "The answer to life, the universe, and everything is: %d", 42)
 *
 * \param self \ref Logger object holding concrete implementation
 * \param source String with information where the log originated from
 * \param level \ref LogLevel of message (used for filtering)
 * \param formatter Format string (printf syntax)
 * \param ... String format arguments (printf sytnax)
 * \return int \c LOGGER_LOG_SUCCESS if successful, any other value in case of error
 */
int logger_log(Logger* self, const char *source, LogLevel level, const char* formatter, ...)
{
    // Validate parameters
    if ((self == NULL) || (self->_log == NULL) || (formatter == NULL))
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_LOG, ILLEGAL_ARGUMENT);
    }

    // Pre-check level
    if (level < self->_level)
    {
        return LOGGER_LOG_SUCCESS;
    }

    // Format string
    va_list args;
    va_start(args, formatter);
    size_t output_length = vsnprintf(NULL, 0, formatter, args);
    char *output = malloc(output_length + 1);
    if (output == NULL)
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_LOG, OUT_OF_MEMORY);
    }

    // Disable vsprintf warning because msbuild warning not valid in C99
    int bytes_written = -1;
    #pragma warning(push)
    #pragma warning(disable : 4996)
    bytes_written = vsprintf(output, formatter, args);
    #pragma warning(pop)

    if (bytes_written < 0)
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_LOG, FORMAT_ERROR);
    }

    // Actually dispatch call
    int status = self->_log(self, source, level, output);

    // Clean up
    free(output);
    va_end(args);
    return status;
}

/**
 * \brief Extension of \ref logger_log() for logging byte arrays
 *
 * \code
 *     uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
 *     size_t data_len = sizeof(data);
 *     logger_log_bytearray(&logger, TAG, LOG_INFO, ">> ", data, data_len, " ");
 *     // >> 01 02 03 04
 * \endcode
 *
 * \param self \ref Logger object holding concrete implementation
 * \param source String with information where the log originated from
 * \param msg Optional message to be prepended to byte array (might be \c NULL)
 * \param data Data to be logged
 * \param data_len Number of bytes in \p data
 * \param delimiter Delimiter to be used between bytes (might be \c NULL for empty string)
 * \return int \c LOGGER_LOG_SUCCESS if successful, any other value in case of error
 */
int logger_log_bytearray(Logger *self, const char *source, LogLevel level, const char *msg, uint8_t *data, size_t data_len, const char *delimiter)
{
    // Validate parameters
    if ((self == NULL) || (self->_log == NULL) || (data == NULL) || (data_len == 0))
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_LOG, ILLEGAL_ARGUMENT);
    }

    // Pre-check level
    if (level < self->_level)
    {
        return LOGGER_LOG_SUCCESS;
    }

    // Manually format string
    size_t msg_len = (msg != NULL) ? strlen(msg) : 0;
    size_t delimiter_len = (delimiter != NULL) ? strlen(delimiter) : 0;
    size_t formatted_len = msg_len + (data_len * 2) + ((data_len - 1) * delimiter_len);
    char *formatted = malloc(formatted_len + 1);
    if (msg_len > 0)
    {
        memcpy(formatted, msg, msg_len);
    }
    for (size_t i=0; i < data_len; i++)
    {
        sprintf(formatted + msg_len + (i * (2 + delimiter_len)), "%02x", data[i]);
        if ((delimiter_len > 0) && ((i + 1) != data_len))
        {
            memcpy(formatted + msg_len + (i * (2 + delimiter_len)) + 2, delimiter, delimiter_len);
        }
    }
    formatted[formatted_len] = '\x00';

    // Actually log message
    int status = self->_log(self, source, level, formatted);
    free(formatted);
    return status;
}

/**
 * \brief Sets minimum log level of interest
 *
 * \details All messages with level lower than \p level shall be discarded by the logger.
 *
 * \param self \ref Logger object to set log level for
 * \param level Mimimum log level of interest
 * \return int \c LOGGER_SET_LEVEL_SUCCESS if successful, any other value in case of error
 */
int logger_set_level(Logger* self, LogLevel level)
{
    // Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBLOGGER, LOGGER_SET_LEVEL, ILLEGAL_ARGUMENT);
    }

    // Actually set level
    self->_level = level;
    return LOGGER_SET_LEVEL_SUCCESS;
}

/**
 * \brief Frees memory associated with \ref Logger object (but not object itself)
 *
 * \details Logger objects might contain data that needs to be released (e.g. open file handles).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param self \ref Logger object whose data shall be freed
 */
void logger_destroy(Logger *self)
{
    if (self != NULL)
    {
        // Check if logger has custom cleanup function
        if (self->_destructor != NULL)
        {
            self->_destructor(self);
        }

        // Check if properties have been missed by concrete implementation
        if (self->_data != NULL)
        {
            free(self->_data);
            self->_data = NULL;
        }
    }
}
