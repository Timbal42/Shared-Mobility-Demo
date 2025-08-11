/**
 * \file aardvark-i2c.c
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief I2C driver layer implementation for TotalPhase Aardvark I2C reader on Windows
 */
#include <stdlib.h>
#include <windows.h>
#include "ifx/timer.h"
#include "ifx/aardvark-i2c.h"
#include "aardvark-i2c.h"

/**
 * \brief Initializes \ref Protocol object for TotalPhase Aardvark I2C driver layer
 * 
 * \param self \ref Protocol object to be initialized
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_initialize(Protocol* self)
{
    // Populate object
    int status = protocollayer_initialize(self);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        return status;
    }
    self->_layer_id = AARDVARK_I2C_PROTOCOLLAYER_ID;
    self->_activate = aardvark_i2c_activate;
    self->_transmit = aardvark_i2c_transmit;
    self->_receive = aardvark_i2c_receive;
    self->_destructor = aardvark_i2c_destroy;

    return PROTOCOLLAYER_INITIALIZE_SUCCESS;
}

/**
 * \brief \ref protocol_activatefunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_activatefunction_t 
 */
int aardvark_i2c_activate(Protocol *self, uint8_t **response_buffer, size_t *response_len)
{
    // Verify that reader is configured correctly
    int status = aardvark_verify_interface_configuration(self);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Perform "cold" reset
    // TODO: Check status of calls
    // TODO: Make configurable
    c_aa_target_power(aardvark_interface_state.aardvark_handle, 0x00);
    Timer reset_timer;
    status = timer_set(&reset_timer, 100 * 1000);
    timer_join(&reset_timer);
    c_aa_target_power(aardvark_interface_state.aardvark_handle, 0x03);

    // Do not send any response
    *response_buffer = NULL;
    *response_len = 0;
    return PROTOCOL_ACTIVATE_SUCCESS;
}

/**
 * \brief \ref protocol_transmitfunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_transmitfunction_t 
 */
int aardvark_i2c_transmit(Protocol *self, uint8_t *data, size_t data_len)
{
    // Validate parameters
    if ((data == NULL) || (data_len == 0) || (data_len > 0xffff))
    {
        return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_TRANSMIT, ILLEGAL_ARGUMENT);
    }

    // Verify that reader is configured correctly
    int status = aardvark_verify_interface_configuration(self);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Get current state of protocol for I2C slave address
    ProtocolState *protocol_state;
    status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Actually send data to reader
    size_t bytes_written = c_aa_i2c_write(aardvark_interface_state.aardvark_handle, protocol_state->slave_address, 0x00, data_len & 0xffff, data);
    if (bytes_written != data_len)
    {
        return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_TRANSMIT, TOO_LITTLE_DATA);
    }
    return PROTOCOL_TRANSMIT_SUCCESS;
}

/**
 * \brief \ref protocol_receivefunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_receivefunction_t 
 */
int aardvark_i2c_receive(Protocol *self, size_t expected_len, uint8_t **response, size_t *response_len)
{
    // Validate parameters
    if ((expected_len == 0) || (expected_len > 0xffff) || (expected_len == PROTOCOL_RECEIVE_LENGTH_UNKOWN))
    {
        return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_RECEIVE, ILLEGAL_ARGUMENT);
    }

    // Verify that reader is configured correctly
    int status = aardvark_verify_interface_configuration(self);
    if (status != PROTOCOL_SETPROPERTY_SUCCESS)
    {
        return status;
    }
    
    // Get current state of protocol for I2C slave address
    ProtocolState *protocol_state;
    status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Actually read data from reader
    *response = malloc(expected_len);
    if ((*response) == NULL)
    {
        return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_RECEIVE, OUT_OF_MEMORY);
    }
    *response_len = c_aa_i2c_read(aardvark_interface_state.aardvark_handle, protocol_state->slave_address, 0x00, expected_len & 0xffff, *response);
    if ((*response_len) != expected_len)
    {
        free(*response);
        *response = NULL;
        return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_RECEIVE, TOO_LITTLE_DATA);
    }
    return PROTOCOL_RECEIVE_SUCCESS;
}

/**
 * \brief \ref protocol_destroyfunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_destroyfunction_t 
 */
void aardvark_i2c_destroy(Protocol *self)
{
    if (self->_properties != NULL)
    {
        free(self->_properties);
        self->_properties = NULL;
    }
}

/**
 * \brief Getter for I2C clock frequency in [Hz]
 * 
 * \param self Protocol object to get clock frequency for
 * \param frequency_buffer Buffer to store clock frequency in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_get_clock_frequency(Protocol *self, uint32_t *frequency_buffer)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    *frequency_buffer = protocol_state->clock_frequency;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Sets I2C clock frequency in [Hz]
 * 
 * \param self Protocol object to set clock frequency for
 * \param frequency Desired clock frequency in [Hz]
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_set_clock_frequency(Protocol *self, uint32_t frequency)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->clock_frequency = frequency;
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Getter for I2C slave address
 * 
 * \param self Protocol object to get I2C slave address for
 * \param address_buffer Buffer to store I2C address in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_get_slave_address(Protocol *self, uint16_t *address_buffer)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    *address_buffer = protocol_state->slave_address;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Sets I2C slave address
 * 
 * \param self Protocol object to set I2C slave address for
 * \param address Desired I2C slave address
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_set_slave_address(Protocol *self, uint16_t address)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->slave_address = address;
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Getter for I2C bus timeout (BTO) in [ms]
 * 
 * \param self Aardvark I2C driver layer to get BTO for
 * \param bto_buffer Buffer to store BTO in
 * \return int \c PROTOCOL_GETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_get_bto(Protocol *self, uint16_t *bto_buffer)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    *bto_buffer = protocol_state->bus_timeout;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Sets I2C bus timeout (BTO) in [ms]
 * 
 * \param self Aardvark I2C driver layer to set BTO for
 * \param bto Desired BTO
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_bto(Protocol *self, uint16_t bto)
{
    ProtocolState *protocol_state;
    int status = aardvark_i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->bus_timeout = bto;
    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Returns current protocol state for of Aardvark I2C driver layer
 * 
 * \param self Aardvark I2C driver layer to get protocol state for
 * \param protocol_state_buffer Buffer to store protocol state in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_get_protocol_state(Protocol *self, ProtocolState **protocol_state_buffer)
{
    // Verify that correct protocol layer called this function
    if (self->_layer_id != AARDVARK_I2C_PROTOCOLLAYER_ID)
    {
        if (self->_base == NULL)
        {
            return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_GETPROPERTY, INVALID_PROTOCOLSTACK);
        }
        return aardvark_i2c_get_protocol_state(self->_base, protocol_state_buffer);
    }

    // Check if protocol state has been initialized
    if (self->_properties == NULL)
    {
        // Lazy initialize properties
        self->_properties = malloc(sizeof(ProtocolState));
        if (self->_properties == NULL)
        {
            return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_GETPROPERTY, OUT_OF_MEMORY);
        }
        ProtocolState *properties = (ProtocolState*) self->_properties;
        properties->slave_address = AARDVARK_I2C_DEFAULT_SLAVE_ADDRESS;
        properties->clock_frequency = AARDVARK_I2C_DEFAULT_CLOCK_FREQUENCY;
        properties->bus_timeout = AARDVARK_I2C_DEFAULT_BUS_TIMEOUT;
    }

    *protocol_state_buffer = (ProtocolState*) self->_properties;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Verifies that interface is configured according to current state of \ref Protocol object
 * 
 * \param driver Aardvark I2C driver layer to verify interface configuration for
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_verify_interface_configuration(Protocol *driver)
{
    // Verify that DLL is loaded
    if (!aardvark_dll_loaded())
    {
        int status = aardvark_load_dll();
        if (status != AARDVARK_LOAD_DLL_SUCCESS)
        {
            return status;
        }
    }

    // Assure that connected to reader
    if (aardvark_interface_state.aardvark_handle <= 0)
    {
        // Find reader
        uint16_t reader_port;
        int readers_found = c_aa_find_devices(1, &reader_port);
        if (readers_found != 1)
        {
            return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_ACTIVATE, NO_READER_FOUND);
        }

        // Verify that device is not busy
        if ((reader_port & 0x8000) != 0x0000)
        {
            return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_ACTIVATE, DEVICE_BUSY);
        }

        // Actually connect to reader
        aardvark_interface_state.aardvark_handle = c_aa_open(reader_port);
        if (aardvark_interface_state.aardvark_handle <= 0)
        {
            return IFX_ERROR(LIBAARDVARKI2C, PROTOCOL_ACTIVATE, CANNOT_CONNECT_TO_READER);
        }

        // Set fixed configuration parameters
        // TODO: Check status of calls
        int status = c_aa_configure(aardvark_interface_state.aardvark_handle, 0x02);
        status = c_aa_i2c_pullup(aardvark_interface_state.aardvark_handle, 0x03);
        status = c_aa_target_power(aardvark_interface_state.aardvark_handle, 0x03);
    }

    // Verify dynamic parameters
    ProtocolState *driver_state;
    int status = aardvark_i2c_get_protocol_state(driver, &driver_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }

    // Check that frequency is set correctly
    if (driver_state->clock_frequency != aardvark_interface_state.i2c_clock_frequency)
    {
        c_aa_i2c_bitrate(aardvark_interface_state.aardvark_handle, driver_state->clock_frequency / 1000);
        aardvark_interface_state.i2c_clock_frequency = driver_state->clock_frequency;
    }

    // Check that bus timeout is set correctly
    if (driver_state->bus_timeout != aardvark_interface_state.bus_timeout)
    {
        c_aa_i2c_bus_timeout(aardvark_interface_state.aardvark_handle, driver_state->bus_timeout);
        aardvark_interface_state.bus_timeout = driver_state->bus_timeout;
    }

    return PROTOCOL_SETPROPERTY_SUCCESS;
}

/**
 * \brief Verifies that Aardvark DLL has successfully been loaded
 * 
 * \return bool \c true if all required DLL functionality has been loaded
 */
bool aardvark_dll_loaded()
{
    return (c_aa_find_devices != NULL) && \
           (c_aa_open != NULL) && \
           (c_aa_close != NULL) && \
           (c_aa_configure != NULL) && \
           (c_aa_i2c_read != NULL) && \
           (c_aa_i2c_write != NULL) && \
           (c_aa_i2c_bitrate != NULL) && \
           (c_aa_i2c_bus_timeout != NULL) && \
           (c_aa_i2c_pullup != NULL);
}

/**
 * \brief Loads Aardvark DLL along with all required functions
 * 
 * \return int \c AARDVARK_LOAD_DLL_SUCCESS if successful, any other value in case of error
 */
int aardvark_load_dll()
{
    // Check if DLL is already loaded
    if (aardvark_dll_loaded())
    {
        return AARDVARK_LOAD_DLL_SUCCESS;
    }

    // Actually load DLL into memory
    HINSTANCE dll_handle = LoadLibraryA("aardvark.dll");
    if (dll_handle == NULL)
    {
        return !SUCCESS;
    }

    // Load all required functions
    c_aa_find_devices = (int (*)(int, uint16_t*)) GetProcAddress(dll_handle, "c_aa_find_devices");
    c_aa_open = (int (*)(int)) GetProcAddress(dll_handle, "c_aa_open");
    c_aa_close = (int (*)(int)) GetProcAddress(dll_handle, "c_aa_close");
    c_aa_configure = (int (*)(int, uint8_t)) GetProcAddress(dll_handle, "c_aa_configure");
    c_aa_target_power = (int (*)(int, uint8_t)) GetProcAddress(dll_handle, "c_aa_target_power");
    c_aa_i2c_bitrate = (int (*)(int, int)) GetProcAddress(dll_handle, "c_aa_i2c_bitrate");
    c_aa_i2c_bus_timeout = (int (*)(int, uint16_t)) GetProcAddress(dll_handle, "c_aa_i2c_bus_timeout");
    c_aa_i2c_read = (int (*)(int, uint16_t, uint8_t, uint16_t, uint8_t *)) GetProcAddress(dll_handle, "c_aa_i2c_read");
    c_aa_i2c_write = (int (*)(int, uint16_t, uint8_t, uint16_t, const uint8_t*)) GetProcAddress(dll_handle, "c_aa_i2c_write");
    c_aa_i2c_pullup = (int (*)(int, uint8_t)) GetProcAddress(dll_handle, "c_aa_i2c_pullup");

    // Verify that functions have successfully been loaded
    if (!aardvark_dll_loaded())
    {
        return !SUCCESS;
    }

    return SUCCESS;
}