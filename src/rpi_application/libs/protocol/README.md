# Generic Protocol Stacks

This library offers a C interface for reusable protocol stacks similar to the [ISO/OSI Model](https://en.wikipedia.org/wiki/OSI_model).

It offers generic `Protocol` structs that can be populated by concrete implementations. These `Protocol` structs can be layered to achieve independence between them and have reusable components (e.g. I2C driver, Global Platform T=1').


## Example

```c
#include "ifx/protocol.h"

// Protocol can be populated by any concrete implementation
Protocol protocol;


// This code can be reused across all protocol stacks
protocol_activate(&protocol, &response, &response_len);
protocol_transceive(&protocol, &data, data_len, &response, &response_len);;
protocol_destroy(&protocol);
```

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters. To simplify version control in-source builds are disallowed.

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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/protocol" ..
cmake --build .
cmake --build . --target install
```
