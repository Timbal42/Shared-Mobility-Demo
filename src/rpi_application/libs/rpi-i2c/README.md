# Protocol wrapper for Raspberry Pi BCM2835 I2C driver

This library implements a reusable layer for building generic [protocol](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/protocol/browse) stacks using the [bcm2835](https://www.airspayce.com/mikem/bcm2835/) library on Raspberry Pi. It is not meant to be used as a standalone layer but rather as the physical layer for stacks built on top.

Additionally it implements the [C I2C API](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/i2c/browse) required by protocols relying on I2C specific functionality.

## Example

```c
#include "ifx/rpi-i2c.h"

// Protocol initialization is only part NOT reusable accross different protocol stacks
Protocol driver;
rpi_i2c_initialize(&driver);

// Use in existing protocol stacks
Protocol protocol;
some_protocol_initialize(&protocol, &driver)
protocol_activate(&protocol, &response, &response_len);
protocol_transceive(&protocol, &data, data_len, &response, &response_len);;
protocol_destroy(&protocol);
```

## Build and Installation

For the full functionality you need to have the [bcm2835](https://www.airspayce.com/mikem/bcm2835/) library installed. This project delivers a mock implementation for testing purposes but will show a warning if the library is not found.

The build uses `cmake` and only uses standard configuration parameters. To simplify version control in-source builds are disallowed.

```
mkdir build
cd build
cmake ..
cmake --build .
sudo cmake --build . --target install
```