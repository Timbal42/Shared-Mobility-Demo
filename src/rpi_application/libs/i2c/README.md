# C Interface for I2C Drivers

This is a header-only C interface for reusable [I2C](https://en.wikipedia.org/wiki/I%C2%B2C) drivers. No actual implementation is provided rather developers can use the header to have the same code running on different platforms by linking to concrete implementations. The interface is designed to be used in combination with the [Infineon Protocol Library](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/protocol).

## Example

```c
#include "ifx/i2c.h"

// Protocol initialized depending on actual stack
Protcol protocol;

// Set protocol parameters for I2C communication
i2c_set_clock_frequency(&protocol, 400000);
i2c_set_slave_address(&protocol, 0x10);

// Call protocol functionality like any other stack
protocol_transceive(&protocol, &data, sizeof(data), &response, &response_len);
```

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters. No actual build steps are required but rather `cmake` is used to distribute the interface.

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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/i2c" ..
cmake --build .
cmake --build . --target install
```
