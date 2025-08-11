# Protocol Stack Extension for APDU Exchange

This library is built on top of the generic [protocol](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-protocol/browse) stacks library and offers an extended interface for exchanging [APDU](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit)s.

It can be used on top of any existing protocol stack to fit the needs of interacting with Secure Elements more.


## Example

```c
#include "ifx/apduprotocol.h"

// Protocol can be populated by any concrete implementation
Protocol protocol;

// Build and exchange APDUs
APDU apdu = {
    .cla = 0x00,
    .ins = 0xA4,
    .p1 = 0x04,
    .p2 = 0x00,
    .lc = 0,
    .data = NULL,
    .le = 0
};
APDUResponse response;
apduprotocol_transceive(&protocol, &apdu, &response);

// Verify response
if (response.sw != 0x9000)
{
    // Handle error
}
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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/apduprotocol" ..
cmake --build .
cmake --build . --target install
```
