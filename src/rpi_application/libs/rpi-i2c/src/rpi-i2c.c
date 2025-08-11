/**
 * \file rpi-i2c.c
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Raspberry Pi I2C driver implementation
 */
#include <stdlib.h>
#include <stdio.h>
#include "bcm2835.h"
#include "ifx/timer.h"
#include "ifx/protocol.h"
#include "ifx/i2c.h"
#include "ifx/rpi-i2c.h"
#include "rpi-i2c.h"

/**
 * \brief Simple instance counter to deinitialize Raspberry Pi SPI driver layer if not used anymore
 */
static int instance_counter = 0;

/**
 * \brief Simple flag to check if the Raspberry Pi SPI driver has been initialized
 */
static bool driver_initialized = false;

/**
 * \brief Returns current protocol state for of I2C driver layer
 * 
 * \param self I2C driver layer to get protocol state for
 * \param protocol_state_buffer Buffer to store protocol state in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int i2c_get_protocol_state(Protocol *self,ProtocolState **protocol_state_buffer)
{
    // Verify that correct protocol layer called this function
    if (self->_layer_id != RPI_I2C_PROTOCOLLAYER_ID)
    {
        if (self->_base == NULL)
        {
            return IFX_ERROR(LIBRPII2C, PROTOCOL_GETPROPERTY, INVALID_PROTOCOLSTACK);
        }
        return i2c_get_protocol_state(self->_base, protocol_state_buffer);
    }

    // Check if protocol state has been initialized
    if (self->_properties == NULL)
    {
        // Lazy initialize properties
        self->_properties = malloc(sizeof(ProtocolState));
        if (self->_properties == NULL)
        {
            return IFX_ERROR(LIBRPII2C, PROTOCOL_GETPROPERTY, OUT_OF_MEMORY);
        }
        ProtocolState *properties = (ProtocolState*) self->_properties;
        properties->slave_address = (uint16_t)I2C_DEFAULT_SLAVE_ADDRESS;
        properties->clock_frequency = (uint32_t)I2C_DEFAULT_CLOCK_FREQUENCY;
    }

    *protocol_state_buffer = (ProtocolState*) self->_properties;
    return PROTOCOL_GETPROPERTY_SUCCESS;
}

/**
 * \brief Initializes \ref Protocol object for Raspberry Pi I2C driver layer
 * 
 * \param self \ref Protocol object to be initialized.
 * \return int \c PROTOCOLLAYER_INITIALIZE_SUCCESS if successful, any other value in case of error.
 */
 
int rpi_i2c_initialize(Protocol* self)
{
    // Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBRPII2C, PROTOCOLLAYER_INITIALIZE, ILLEGAL_ARGUMENT);
    }

    // Populate object
    int status = protocollayer_initialize(self);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        return status;
    }
    self->_layer_id = RPI_I2C_PROTOCOLLAYER_ID;
    self->_activate = rpi_i2c_activate;
    self->_transmit = rpi_i2c_transmit;
    self->_receive = rpi_i2c_receive;
    self->_destructor = rpi_i2c_destroy;
    
    // Increment instance counter to be able to clean up later on
    instance_counter++;

    // 0 if successful, any other value in case of error.
    if( bcm2835_init() != 1)
    {
        return -1;
    }
    if(bcm2835_i2c_begin() != 1)
    {
        return -1;
    }
    return PROTOCOLLAYER_INITIALIZE_SUCCESS;
}

/**
 * \brief \ref protocol_destroyfunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_destroyfunction_t 
 */
void rpi_i2c_destroy(Protocol *self)
{
    // TODO: Implement
    if (self != NULL)
    {
        // Free properties
        if (self->_properties != NULL)
        {
            free(self->_properties);
            self->_properties = NULL;
        }
        
        // Decrement instance counter and check if driver needs to be shut down
        instance_counter--;
        if (instance_counter <= 0)
        {
            instance_counter = 0;
            if (driver_initialized)
            {
                bcm2835_i2c_end();
                bcm2835_close();
            }
            driver_initialized = false;
        }
    }
}

/**
 * \brief \ref protocol_activatefunction_t for Raspberry Pi I2C driver layer
 *
 * \see protocol_activatefunction_t
 */
int rpi_i2c_activate(Protocol *self, uint8_t **response, size_t *response_len)
{
    // TODO: Pull reset line
    //       RESET_PIN set to HIGH
    //       Wait 100ms
    //       RESET_PIN set to LOW
    bcm2835_gpio_fsel(RPI_GPIO_P1_08, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RPI_GPIO_P1_08, HIGH);
    bcm2835_delay(100);
    bcm2835_gpio_write(RPI_GPIO_P1_08, LOW);
    *response = NULL;
    *response_len = 0;
    return PROTOCOL_ACTIVATE_SUCCESS;
}


/**
 * \brief \ref protocol_transmitfunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_transmitfunction_t 
 */
int rpi_i2c_transmit(Protocol *self, uint8_t *data, size_t data_len)
{
	// TODO: Implement
	// Validate parameters
	if ((self == NULL) || (data == NULL) || (data_len == 0) || (data_len > 0xffffffff))
    {
        return IFX_ERROR(LIBRPII2C, PROTOCOL_TRANSMIT, ILLEGAL_ARGUMENT);
    }

    // Actually send data to reader
    bcm2835_delayMicroseconds(100);
    int status = bcm2835_i2c_write(data,data_len);
    bcm2835_delayMicroseconds(100);
	if(status)
    {
        return status;
    }
	return PROTOCOL_TRANSMIT_SUCCESS;
}

/**
 * \brief \ref protocol_receivefunction_t for Raspberry Pi I2C driver layer
 * 
 * \see protocol_receivefunction_t 
 */
int rpi_i2c_receive(Protocol *self, size_t expected_len, uint8_t **response, size_t *response_len)
{
    // TODO: Implement
	// Validate parameters
    if ((self == NULL) || (expected_len == 0) || (expected_len > 0xffffffff) || (response == NULL) || (response_len == NULL))
    {
        return IFX_ERROR(LIBRPII2C, PROTOCOL_RECEIVE, ILLEGAL_ARGUMENT);
    }

    // Allocate buffer for I2C receive 
    *response = (char *)malloc(expected_len);
    if ((*response) == NULL)
    {
        return IFX_ERROR(LIBRPII2C, PROTOCOL_RECEIVE, OUT_OF_MEMORY);
    }
    bcm2835_delayMicroseconds(100);
    int status = bcm2835_i2c_read(*response,(uint32_t)expected_len);
    bcm2835_delayMicroseconds(100);
    if(status)
    {
        free(*response);
        *response = NULL;
        *response_len = 0;
        return status;
    }

    *response_len = expected_len;
    return PROTOCOL_RECEIVE_SUCCESS;
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
    int status = i2c_get_protocol_state(self, &protocol_state);
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
    int status = i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->clock_frequency = frequency;
    bcm2835_i2c_set_baudrate(frequency);
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
    // TODO: Implement
    ProtocolState *protocol_state;
    int status = i2c_get_protocol_state(self, &protocol_state);
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
    // TODO: Implement
	// Validate parameters
    if (self == NULL)
    {
        return IFX_ERROR(LIBRPII2C, PROTOCOLLAYER_INITIALIZE, ILLEGAL_ARGUMENT);
    }

    ProtocolState *protocol_state;
    int status = i2c_get_protocol_state(self, &protocol_state);
    if (status != PROTOCOL_GETPROPERTY_SUCCESS)
    {
        return status;
    }
    protocol_state->slave_address = address;
    bcm2835_i2c_setSlaveAddress(address);
    return PROTOCOL_SETPROPERTY_SUCCESS;
}
