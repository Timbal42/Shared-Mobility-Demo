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
 * \file ifx/logger.h
 * \brief Generic logging API
 */
#ifndef _IFX_LOGGER_H_
#define _IFX_LOGGER_H_

#include <stdarg.h>
#include "ifx/error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief IFX error encoding  module identifer
 */
#define LIBLOGGER 0x90

typedef struct Logger Logger;

/**@enum LogLevel
 * \brief Log level for filtering messages to actually be logged
 */
typedef enum
{
    /**
     * \brief Debug information for finding problems in library
     */
    LOG_DEBUG = 0x00,

    /**
     * \brief Information that help trace the program's normal execution flow
     */
    LOG_INFO = 0x01,

    /**
     * \brief Information that warns of potential problems
     */
    LOG_WARN = 0x02,

    /**
     * \brief Information about (recoverable) errors
     */
    LOG_ERROR = 0x03,

    /**
     * \brief Information about non-recoverable errors
     */
    LOG_FATAL = 0x04
} LogLevel;

/**
 * \brief Reusable IFX error encoding function identifier for any function initializing a \ref Logger
 */
#define LOGGER_INITIALIZE 0x90

/**
 * \brief Return code for successful calls to any function initializing a \ref Logger
 */
#define LOGGER_INITIALIZE_SUCCESS SUCCESS

/**
 * \brief Initializes \ref Logger object by setting all members to valid (but potentially unusable) values
 *
 * \details Concrete implementations can call this function to ensure all members are set to their correct default values before populating the required fields.
 *
 * \param self \ref Logger object to be initialized
 * \return int \c LOGGER_INITIALIZE_SUCCESS if successful, any other value in case of error
 */
int logger_initialize(Logger *self);

/**
 * \brief Reusable IFX error encoding function identifier for all functions logging data
 */
#define LOGGER_LOG 0x91

/**
 * \brief Reusable return code for successful calls to functions logging data
 */
#define LOGGER_LOG_SUCCESS SUCCESS

/**
 * \brief IFX error encoding error reason if error occured formatting string in \ref logger_log()
 */
#define FORMAT_ERROR 0x01

/**
 * \brief Log formatted message
 *
 * \details Uses printf syntax for message formatting.
 *
 * \code
 *     logger_log(&logger, "example", LOG_INFO, "The answer to life, the universe, and everything is: %d", 42)
 * \endcode
 *
 * \param self \ref Logger object holding concrete implementation
 * \param source String with information where the log originated from
 * \param level \ref LogLevel of message (used for filtering)
 * \param formatter Format string (printf syntax)
 * \param ... String format arguments (printf sytnax)
 * \return int \c LOGGER_LOG_SUCCESS if successful, any other value in case of error
 */
int logger_log(Logger* self, const char *source, LogLevel level, const char* formatter, ...);

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
int logger_log_bytearray(Logger *self, const char *source, LogLevel level, const char *msg, uint8_t *data, size_t data_len, const char *delimiter);

/**
 * \brief Implementation specific log function
 *
 * \details Used by \ref logger_log() to call concrete implementation.
 *
 * \param self \ref Logger object the function was called for
 * \param source String with information where the log originated from
 * \param level \ref LogLevel of message (used for filtering)
 * \param message Formatted string to be logged
 * \return int \c LOGGER_LOG_SUCCESS if successful, any other value in case of error
 */
typedef int (*logger_logfunction_t)(Logger *self, const char *source, LogLevel level, const char* message);

/**
 * \brief IFX error encoding function identifier for \ref logger_set_level(Logger*, LogLevel)
 */
#define LOGGER_SET_LEVEL 0x01

/**
 * \brief Return code for successful calls to \ref logger_set_level(Logger*, LogLevel)
 */
#define LOGGER_SET_LEVEL_SUCCESS SUCCESS

/**
 * \brief Sets minimum log level of interest
 *
 * \details All messages with level lower than \p level shall be discarded by the logger.
 *
 * \param self \ref Logger object to set log level for
 * \param level Mimimum log level of interest
 * \return int \c LOGGER_SET_LEVEL_SUCCESS if successful, any other value in case of error
 */
int logger_set_level(Logger* self, LogLevel level);

/**
 * \brief Frees memory associated with \ref Logger object (but not object itself)
 *
 * \details Logger objects might contain data that needs to be released (e.g. open file handles).
 *          Users would need to manually check which members have been dynamically allocated and free them themselves.
 *          Calling this function will ensure that all dynamically allocated members have been freed.
 *
 * \param self \ref Logger object whose data shall be freed
 */
void logger_destroy(Logger *self);

/**
 * \brief Implementation specific destructor
 *
 * \details Different loggers may need different clean-up functionality.
 *          This function type gives a generic interface for performing clean-up.
 *
 * \param self \ref Logger object being destroyed
 */
typedef void (*logger_destroyfunction_t)(Logger *self);

/** \struct Logger
 * \brief Generic Logger object used to decapsulate concrete implementation from interface
 */
struct Logger
{
    /**
     * \brief Private logging function for concrete implementation
     *
     * \details Set by implementation's initialization function, do **NOT** set manually.
     */
    logger_logfunction_t _log;

    /**
     * \brief Private destructor if further cleanup is necessary
     *
     * \details Set by implementation's initialization function, do **NOT** set manually.
     *          \ref logger_destroy(Logger*) will call `free()` for \ref Logger._data .
     *          If any further cleanup is necessary implement it in this function.
     *          Otherwise use \c NULL.
     */
    logger_destroyfunction_t _destructor;

    /**
     * \brief Private member for minimum log level used for filtering messages
     *
     * \details Set by \ref logger_set_level(Logger*, LogLevel), do **NOT** set manually
     */
    LogLevel _level;

    /**
     * \brief Private member for generic logger data as \c void*
     *
     * \details Only used internally, do **NOT** set manually.
     *          Might be \c NULL.
     */
    void *_data;
};

#ifdef __cplusplus
}
#endif

#endif // _IFX_LOGGER_H_
