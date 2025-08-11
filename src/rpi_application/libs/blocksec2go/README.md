# Blockchain  Security 2Go Starterkit R2 C-API

This library implements the Blockchain Security 2Go Starterkit R2 command set.

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters.

```
mkdir build
cd build
cmake ..
cmake --build . --target install
```

If you are building on Windows with Visual Studio you need to set up a correct environment before calling `cmake`. Visual Studio offers a utility script called *VsDevComd.bat*  located in its *Common7/Tools* directory (e.g. *C:/Program Files (x86)/Microsoft  Visual Studio/2017/Professional/Common7/Tools*) to set everything up.

Further Windows does not have a standard location where to install libraries to. You will need to specify where to put the installation using the `CMAKE_INSTALL_PREFIX` configuration parameter.

```
"C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat"
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/blocksec2go" ..
cmake --build .
cmake --build . --target install
```

### Example (Unprotected Mode)

 See [Blockchain Security 2Go Starterkit R2 C-API example](https://bitbucket.vih.infineon.com/projects/ORION/repos/blocksec2go-c-api-example/browse) for a more detailed example of use.
 
```c
#include "ifx/blocksec2go.h"
#include "ifx/error.h"
#include "ifx/t1prime.h"
#include "ifx/rpi-i2c.h"

//Initialize driver and protocol
Protocol driver;
int status = rpi_i2c_initialize(&driver);

Protocol protocol;
status = t1prime_initialize(&protocol, &driver);


// Select Card
uint8_t *version = 0;
uint8_t id[BLOCK2GO_ID_LEN];

status = block2go_select(&protocol, id, &version);

// Generate Permanent Key
uint8_t key;
status = block2go_generate_key_permanent(&protocol,BLOCK2GO_CURVE_NIST_P256, &key);

// Get Key Info
uint32_t global_counter = 0;
uint32_t counter = 0;
uint8_t *public_key = 0;
block2go_curve curve = BLOCK2GO_CURVE_NIST_P256; 
status = block2go_get_key_info_permanent(
        &protocol, key, &curve, &global_counter, &counter, &public_key);

```

### Example (Protected Mode)
```c
#include "ifx/blocksec2go.h"
#include "ifx/error.h"
#include "ifx/scp03.h"
#include "ifx/t1prime.h"
#include "ifx/rpi-i2c.h"

//Initialize driver and protocol
Protocol driver;
int status = rpi_i2c_initialize(&driver);

Protocol protocol;
status = t1prime_initialize(&protocol, &driver);

// Select Card
uint8_t *version = 0;
uint8_t id[BLOCK2GO_ID_LEN];

status = block2go_select(&protocol, id, &version);

// Initialize SCP03 protocol
Protocol scp03_protocol;
status = scp03_initialize(&scp03_protocol, &protocol);

Scp03InitializeUpdateResponse init_update_resp;
status = scp03_initialize_update(&scp03_protocol, 0, 0, 0, &init_update_resp);
```
The static keys for initializing SCP03 can be taken from the specification.
```c
uint8_t key_enc[] = {0xAF, 0x5F, 0x77, 0x47, 0x03, 0xCC, 0xBB, 0x69,
                     0x0C, 0xED, 0x79, 0x4B, 0x41, 0xDE, 0x82, 0xC6,
                     0x28, 0xE6, 0xF3, 0x55, 0xF2, 0xA9, 0x4D, 0x78,
                     0x1E, 0x26, 0xAC, 0xEF, 0x4D, 0x2D, 0xF8, 0xB4};

uint8_t key_mac[] = {0x9E, 0xC4, 0x8A, 0x64, 0xA3, 0xD1, 0x49, 0xC0,
                     0x13, 0x56, 0x1F, 0x5D, 0x75, 0x1D, 0xBD, 0x5F,
                     0x3D, 0x23, 0xBE, 0x7A, 0x06, 0xE0, 0xC9, 0xB3,
                     0x0F, 0xCE, 0x0A, 0x6D, 0x51, 0x42, 0xEC, 0x5E};

Scp03StaticKeys keys = {key_enc, sizeof(key_enc), key_mac, sizeof(key_mac), 0, 0};
```
The security level can be chosen:
```c
uint8_t security_level = SCP03_SECURITY_LEVEL_C_ENCRYPTION; 

status = scp03_external_authenticate(&scp03_protocol, &keys,
                                       security_level);

status = block2go_enable_protected_mode(&scp03_protocol);
```
Now communication via secure chanel is enabled and the SCP03 protocol `scp03_protocol` can be used for the commands:
```c
// Generate Permanent Key
uint8_t key;
status = block2go_generate_key_permanent(&scp03_protocol,BLOCK2GO_CURVE_NIST_P256, &key); 
```
