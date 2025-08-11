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
 * \file console-logger.c
 * \brief Logger API implementation logging to STDOUT
 */
#include <stdio.h>
#include "ifx/logger.h"
#include "ifx/console-logger.h"
#include "console-logger.h"

/**
 * \brief Initializes \ref Logger object to be used as console logger
 *
 * \param self \ref Logger object to be initialized.
 * \return int \c LOGGER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
int consolelogger_initialize(Logger *self)
{
    // Initialize object with default values
    int status = logger_initialize(self);
    if (status != LOGGER_INITIALIZE_SUCCESS)
    {
        return status;
    }

    // Populate member functions
    self->_log = consolelogger_log;

    return LOGGER_INITIALIZE_SUCCESS;
}

/**
 * \brief \ref logger_logfunction_t for console logger
 *
 * \see logger_logfunction_t
 */
int consolelogger_log(Logger* self, const char *source, LogLevel level, const char* formatter)
{
    // Get log level tag as string
    const char *level_tag;
    switch (level)
    {
    case LOG_DEBUG:
        level_tag = "DEBUG";
        break;
    case LOG_INFO:
        level_tag = "INFO";
        break;
    case LOG_WARN:
        level_tag = "WARNING";
        break;
    case LOG_ERROR:
        level_tag = "ERROR";
        break;
    case LOG_FATAL:
        level_tag = "FATAL";
        break;
    default:
        // Should not occur
        level_tag = "UNKNOWN";
        break;
    }
    printf("[%s] [%-7s] -> %s\n", source, level_tag, formatter);
    return LOGGER_LOG_SUCCESS;
}