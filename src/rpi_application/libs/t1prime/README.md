# Global Platform T=1' Protocol

This library implements the Global Platform T=1' protocol as a reusable layer for generic [protocol](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-protocol/browse) stacks.

As the protocol only works over [I2C](https://en.wikipedia.org/wiki/I%C2%B2C) or [SPI](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface) a concrete driver implementation of the Infineon [I2C API](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-i2c/browse) or [SPI API](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-spi/browse) is required when consuming this library.


## Example

```c
#include "ifx/t1prime.h"

// Protocol initialization is only part NOT reusable accross different protocol stacks
Protocol driver;
Protocol protocol;
t1prime_initialize(&protocol, &driver);

// The rest can be reused as any other protocol stack
protocol_activate(&protocol, &response, &response_len);
protocol_transceive(&protocol, &data, data_len, &response, &response_len);;
protocol_destroy(&protocol);
```

## Build and Installation

The build uses `cmake` with standard configuration parameters as well as custom parameters defined here. To simplify version control in-source builds are disallowed.

The custom configuration parameters are:

* `USE_I2C`
    Parameter determining whether to build protocol for I2C or SPI (`1` or `on` means use `I2C`)

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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/t1prime" ..
cmake --build .
cmake --build . --target install
```
