#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ifx/se_interface.h"
#include "ifx/t1prime.h"
#include "ifx/rpi-i2c.h"
#include "ifx/protocol.h"
#include "ifx/blocksec2go.h"

static Protocol protocol;
static Protocol driver;

uint16_t se_interface_init()
{
    // Initialize Raspberry Pi I2C driver
    int status = rpi_i2c_initialize(&driver);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        printf("rpi error: %i\n", status);
        return status;
    }
    
    // Initialize T=1' protocol
    status = t1prime_initialize(&protocol, &driver);
    if (status != PROTOCOLLAYER_INITIALIZE_SUCCESS)
    {
        printf("t1prime error: %i\n", status);
      protocol_destroy(&driver);
        return status;
    }
    
    i2c_set_slave_address(&driver, I2C_ADDRESS);

    // Activate secure element
    uint8_t *response = NULL;
    size_t response_len = 0;
    status = protocol_activate(&protocol, &response, &response_len);
    if (status != PROTOCOL_ACTIVATE_SUCCESS)
    {
        printf("activate error: %i\n", status);
        protocol_destroy(&driver);
        return status;
    }
    free(response);
    
    return SUCCESS;
}

int wrap_block2go_select(uint8_t id[BLOCK2GO_ID_LEN],
                    char **version)
{
    return block2go_select(&protocol, id, version);
}

int wrap_gen_key(uint8_t *key_index)
{
    int status = block2go_generate_key_permanent(&protocol,
    BLOCK2GO_CURVE_SEC_P256K1, key_index); if (status !=
    BLOCK2GO_GENERATE_KEY_SUCCESS) { 
      fprintf(stderr, "GENERATE KEY failed(0x%08x)\n", status);
      protocol_destroy(&driver);
    }
    return status;
}

int wrap_get_pub_key(uint8_t key_index, uint8_t *public_key[65], uint8_t *public_key_len)
{
    block2go_curve curve = BLOCK2GO_CURVE_SEC_P256K1;
    uint32_t global_counter = 0;
    uint32_t counter = 0;
    *public_key_len = BLOCK2GO_PUBLIC_KEY_LEN;
    
    int  status = block2go_get_key_info_permanent(
        &protocol, key_index, &curve, &global_counter, &counter, public_key);
    
    if (status != BLOCK2GO_GET_KEY_INFO_SUCCESS) {
      fprintf(stderr, "GET KEY INFO failed (0x%08x)\n", status);
      protocol_destroy(&driver);
      free(*public_key);
      
    }
    return status;
}

int wrap_sign(uint8_t key_index, uint8_t data_to_sign[32], uint8_t **signature, size_t *signature_len)
{
    uint32_t counter = 0;
    uint32_t global_counter = 0;

    int status = block2go_generate_signature_permanent(&protocol, key_index, data_to_sign,
                                                   &global_counter, &counter,
                                                   signature, signature_len);
    if (status != BLOCK2GO_GENERATE_SIGNATURE_SUCCESS) {
      fprintf(stderr, "\nGENERATE SIGNATURE failed (0x%08x)\n", status);
      protocol_destroy(&driver);
    }
    
    return status;

}
