# APDU en-/decoding

This library offers utilities to en- and decode [APDU](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit) objects as well as their responses.

In smartcard applications a host typically sends binary `APDU` data to a secure element and reads back a response. This binary data may be somewhat difficult to create and understand depending on the length of the data, etc. This library can be used to work with `APDU`s in a more generic and structured way.

## Example

```c
#include "ifx/apdu.h"

// Encode APDU so that it can be send to secure element
APDU apdu = {
    .cla = 0x01,
    .ins = 0x02,
    .p1 = 0x03,
    .p2 = 0x04,
    .lc = 0,
    .data = NULL,
    .le = 0
};
uint8_t *encoded;
size_t encoded_len;
apdu_encode(&apdu, &encoded, &encoded_len);

// Perform some interaction with a secure element (not of importance here)
uint8_t *response;
size_t response_len;
protocol_transceive(protocol, encoded, encoded_len, &response, &response_len);

// Decode APDU response and check status
APDUResponse decoded;
apduresponse_decode(&decoded, response, response_len);
if (decoded.sw != 0x9000)
{
    // Error indicator set
}
```

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters. To simplify version control in-source builds are disallowed.

```
mkdir build
cd build
cmake ..
cmake --build .
cmake --build . --target install
```

If you are building on Windows with Visual Studio you need to set up a correct environment before calling `cmake`. Visual Studio offers a utility script called *VsDevComd.bat*  located in its *Common7/Tools* directory (e.g. *C:/Program Files (x86)/Microsoft  Visual Studio/2017/Professional/Common7/Tools*) to set everything up.

Further Windows does not have a standard location where to install libraries to. You will need to specify where to put the installation using the `CMAKE_INSTALL_PREFIX` configuration parameter.

```
"C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat"
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/apdu" ..
cmake --build .
cmake --build . --target install
```


## Testing

All tests are based on [catch2](https://github.com/catchorg/Catch2). The build assumes that `catch2` is installed and only enables testing if it found the required `cmake` package.

On Windows you can set the `Catch2_DIR` enviroment variable to point to your installation directory and then call `ctest` as usual.

```
SET Catch2_DIR=%APPDATA%/cmake/catch2
cmake --build .
ctest .
```

The `ctest` target `memcheck` can be used to run all tests with a memory check command (by default `valgrind`). You can specify which tool to use by setting the  `MEMORY_CHECK_COMMAND` configuration parameter.

```
cmake --build .
ctest -T memcheck .
ctest -DMEMORY_CHECK_COMMAND=/usr/bin/valgrind -T memcheck .
```
