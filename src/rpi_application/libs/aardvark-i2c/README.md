# Protocol wrapper for TotalPhase Aardvark I2C reader

This library implements a reusable layer for building generic [protocol](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/protocol/browse) using the TotalPhase Aardvark I2C Reader. It is not meant to be used as a standalone layer but rather as the physical layer for stacks built on top.

Additionally it implements the [C I2C API](https://bitbucket.vih.infineon.com/projects/GOLDENEYE_EVALBOARD/repos/i2c/browse) required by protocols relying on I2C specific functionality.

## Example

```c
#include "ifx/aardvark-i2c.h"

// Protocol initialization is only part NOT reusable accross different protocol stacks
Protocol driver;
aardvark_i2c_initialize(&driver);

// Use in existing protocol stacks
Protocol protocol;
some_protocol_initialize(&protocol, &driver)
protocol_activate(&protocol, &response, &response_len);
protocol_transceive(&protocol, &data, data_len, &response, &response_len);;
protocol_destroy(&protocol);
```

## Build and Installation

The build uses `cmake` with standard configuration parameters as well as custom parameters defined here. To simplify version control in-source builds are disallowed.

The custom configuration parameters are:

* `AARDVARK_DLL_OUTPUT_PATH`
    Parameter determining if the provided `aardvark.dll` should be copied to a non-standard location.
    This can be useful for embedding this library in applications without the need to manually copy the required dll somewhere.

As this code is intended to be built on Windows with Visual Studio you need to set up a correct environment before calling `cmake`. Visual Studio offers a utility script called *VsDevComd.bat*  located in its *Common7/Tools* directory (e.g. *C:/Program Files (x86)/Microsoft  Visual Studio/2017/Professional/Common7/Tools*) to set everything up.

Further Windows does not have a standard location where to install libraries to. You will need to specify where to put the installation using the `CMAKE_INSTALL_PREFIX` configuration parameter.

```
"C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat"
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/aardvark-i2c" -AARDVARK_DLL_OUTPUT_PATH="..\..\build" ..
cmake --build .
cmake --build . --target install
```