# CRC implementations

This library offers reusable Cyclic Redundancy Check [CRC] implementations.

## Example

```c
#include "ifx/crc.h"

// Generate some data
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
size_t data_len = sizeof(data);

// Calculate different CRC values
uint16_t x25 = crc16_ccitt_x25(data, data_len);
uint16_t mcrf4xx = crc16_mcrf4xx(data, data_len);
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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/crc" ..
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
