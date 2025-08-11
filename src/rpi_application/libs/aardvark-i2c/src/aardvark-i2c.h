/**
 * \file aardvark-i2c.h
 * \author Martin Kloesch (martin.kloesch-ee@infineon.com)
 * \brief Internal definitions for Total Phase Aardvark I2C driver layer
 */
#ifndef _AARDVARK_I2C_H_
#define _AARDVARK_I2C_H_

#include "ifx/protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Protocol Layer ID for Aardvark I2C driver layer
 * 
 * \details Used to verify that correct protocol layer has called member functionality
 */
#define AARDVARK_I2C_PROTOCOLLAYER_ID 0x03

/**
 * \brief \ref protocol_activatefunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_activatefunction_t 
 */
int aardvark_i2c_activate(Protocol *self, uint8_t **response_buffer, size_t *response_len);

/**
 * \brief \ref protocol_transmitfunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_transmitfunction_t 
 */
int aardvark_i2c_transmit(Protocol *self, uint8_t *data, size_t data_len);

/**
 * \brief \ref protocol_receivefunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_receivefunction_t 
 */
int aardvark_i2c_receive(Protocol *self, size_t expected_len, uint8_t **response, size_t *response_len);

/**
 * \brief \ref protocol_destroyfunction_t for TotalPhase Aardvark I2C driver layer
 * 
 * \see protocol_destroyfunction_t 
 */
void aardvark_i2c_destroy(Protocol *self);

#pragma region PROTOCOL_PROPERTIES
/**
 * \brief State of Aardvark I2C driver layer
 */
typedef struct ProtocolState
{
    uint16_t slave_address; /**< I2C address currently in use */
    uint32_t clock_frequency; /**< I2C clock frequency in [Hz] */
    uint16_t bus_timeout; /**< Current I2C bus timeout in [ms] */
} ProtocolState;

/**
 * \brief Protocol property identifier for current state of G+D T=1 protocol
 */
#define AARDVARK_I2C_PROPERTY_PROTOCOL_STATE ((uint64_t) ((((uint64_t) LIBAARDVARKI2C) << 32) | 0x0000000000000001))

/**
 * \brief Default value for I2C address used
 */
#define AARDVARK_I2C_DEFAULT_SLAVE_ADDRESS ((uint16_t) 0x10)

/**
 * \brief Default value for I2C clock frequency in [Hz]
 */
#define AARDVARK_I2C_DEFAULT_CLOCK_FREQUENCY ((uint32_t) 100000)

/**
 * \brief Default value for I2C bus timeout in [Hz]
 */
#define AARDVARK_I2C_DEFAULT_BUS_TIMEOUT ((uint32_t) 150)

/**
 * \brief Returns current protocol state for of Aardvark I2C driver layer
 * 
 * \param self Aardvark I2C driver layer to get protocol state for
 * \param protocol_state_buffer Buffer to store protocol state in
 * \return int \c PROTOCOL_GETPROTPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_i2c_get_protocol_state(Protocol *self, ProtocolState **protocol_state_buffer);
#pragma endregion PROTOCOL_PROPERTIES

#pragma region READER_INTERACTION
/**
 * \brief Error reason if no reader found during \ref aardvark_verify_interface_configuration(Protocol*)
 */
#define NO_READER_FOUND 0xaa

/**
 * \brief Error reason if reader busy while trying to connect during \ref aardvark_verify_interface_configuration(Protocol*)
 */
#define DEVICE_BUSY 0xab

/**
 * \brief Error reason if no connection to reader could be established during \ref aardvark_verify_interface_configuration(Protocol*)
 */
#define CANNOT_CONNECT_TO_READER 0xac

/**
 * \brief Verifies that interface is configured according to current state of \ref Protocol object
 * 
 * \param driver Aardvark I2C driver layer to verify interface configuration for
 * \return int \c PROTOCOL_SETPROPERTY_SUCCESS if successful, any other value in case of error
 */
int aardvark_verify_interface_configuration(Protocol *driver);

/**
 * \brief Global struct for current state of Aardvark reader
 */
struct
{
    int aardvark_handle; /**< Reader connection handle for reader currently used */
    uint32_t i2c_clock_frequency; /**< Current I2C clock frequency in [Hz] */
    uint16_t bus_timeout; /**< Current I2C bus timeout in [ms] */
} aardvark_interface_state;

int (*c_aa_find_devices)(int max, uint16_t *ports) = NULL;
int (*c_aa_open)(int port) = NULL;
int (*c_aa_close)(int) = NULL;
int (*c_aa_configure)(int, uint8_t) = NULL;
int (*c_aa_target_power)(int, uint8_t) = NULL;
int (*c_aa_i2c_bitrate)(int, int) = NULL;
int (*c_aa_i2c_bus_timeout)(int, uint16_t) = NULL;
int (*c_aa_i2c_read)(int, uint16_t, uint8_t, uint16_t, uint8_t *) = NULL;
int (*c_aa_i2c_write)(int, uint16_t, uint8_t, uint16_t, const uint8_t*) = NULL;
int (*c_aa_i2c_pullup)(int, uint8_t) = NULL;

/**
 * \brief IFX error encoding function identifier for \ref aardvark_load_dll()
 */
#define AARDVARK_LOAD_DLL 0x01

/**
 * \brief Return code for successful calls to \ref aardvark_load_dll()
 */
#define AARDVARK_LOAD_DLL_SUCCESS SUCCESS

/**
 * \brief Loads Aardvark DLL along with all required functions
 * 
 * \return int \c AARDVARK_LOAD_DLL_SUCCESS if successful, any other value in case of error
 */ 
int aardvark_load_dll();

/**
 * \brief Verifies that Aardvark DLL has successfully been loaded
 * 
 * \return bool \c true if all required DLL functionality has been loaded
 */
bool aardvark_dll_loaded();

#pragma endregion

#ifdef __cplusplus
}
#endif

#endif // _AARDVARK_I2C_H_