# Infineon Error Codes

This library offers utilities to create and decode C style `int` error codes.

In complex library chains it is often difficult to track exactly where errors come from.
All **Infineon** libraries therefore encode their errors according to the following schema:

| Bits    | Information                                                                                                                                     |
|---------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| `31`    | Error indicator set to `1` in case of error                                                                                                     |
| `30-24` | **RFU**                                                                                                                                         |
| `23-16` | Module identifier (e.g. `0x01` for `apdu` library)                                                                                              |
| `15-8`  | Function identifier in module (e.g. `0x03` for `apdu_encode` in `apdu` libary)                                                                  |
| `7-0`   | Function-specific error reason (e.g. `0xff` out of memory).<br>Values between 0xf0 and 0xff are reserved for reusable error codes defined here. |

So a return value of `0x801003fe` indicates:
  * Bit `31` is set -> **error**.
  * Bits `23-16` are set to `0x10` -> `apdu` library
  * Bits `15-8` are set to `0x03` -> `apdu_encode` function
  * Bits `7-0` are set to `0xfe` -> **Out of memory**

## Example

```c
#include "ifx/error.h"
#include "ifx/apdu.h"

int error_code = IFX_ERROR(LIBAPDU, APDU_ENCODE, OUT_OF_MEMORY);
if (is_error(error_code))
{
    switch(get_module(error_code))
    {
    case LIBAPDU:
        switch (get_function(error_code))
        {
            case APDU_ENCODE:
                switch (get_reason(error_code))
                {
                case OUT_OF_MEMORY:
                    printf("Out of memory during APDU encoding\n");
                    break;
                default:
                    printf("Unkown error during APDU encoding\n");
                    break;
                }
                break;
            default:
                printf("Error in unknown APDU function\n");
                break;
        }
        break;
    default:
        printf("Error in unkown module\n");
        break;
    }
}
```

## Reusable Error Reasons

This library offers a set of reusable error reasons similar to standard exceptions in other languages.

The following table gives an overview of these values and a quick description for each.

| Value  | Name                | Description                                                           |
|--------|---------------------|-----------------------------------------------------------------------|
| `0xff` | `UNSPECIFIED_ERROR` | Error without any further information                                 |
| `0xfe` | `OUT_OF_MEMORY`     | Could not allocate memory                                             |
| `0xfd` | `ILLEGAL_ARGUMENT`  | Illegal argument given to function call                               |
| `0xfc` | `TOO_LITTLE_DATA`   | Too little data available (e.g. in decoding functions)                |
| `0xfb` | `INVALID_STATE`     | Program (currently) in invalid state                                  |
| `0xfa` | `PROGRAMMING_ERROR` | Error that should have already been caught by the programmer occurred |

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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/ifx-error" ..
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