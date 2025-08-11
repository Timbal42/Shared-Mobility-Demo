# Logger API Implementation logging to STDOUT

This library implements the Infineon [logger](https://bitbucket.vih.infineon.com/projects/V2XSYS/repos/hsw-logger/browse) API.

## Example

```c
#include "ifx/logger.h"
#include "ifx/console-logger.h"

// Logger initialization is only part NOT reusable
Logger logger;
consolelogger_initialize(&logger);

// Log source information reusable accross e.g. library
#define LOG_TAG "example"

// The rest can be reused as any other logger
logger_set_level(&logger, LOG_INFO);
logger_log(&logger, LOG_TAG, LOG_WARN, "Something happened");
logger_log(&logger, LOG_TAG, LOG_INFO, "The answer to life, the universe and everything is: %d", 42);
logger_destroy(&logger);
```

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters. To simplify version control in-source builds are disallowed.

```
mkdir build
cd build
cmake ..
cmake --build . --target install
```

If you are building on Windows with Visual Studio you need to set up a correct environment before calling `cmake`. Visual Studio offers a utility script called *VsDevComd.bat* located in its *Common7/Tools* directory (e.g. *C:/Program Files (x86)/Microsoft  Visual Studio/2017/Professional/Common7/Tools*) to set everything up.

Further Windows does not have a standard location where to install libraries to. You will need to specify where to put the installation using the `CMAKE_INSTALL_PREFIX` configuration parameter.

```
"C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat"
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/console-logger" ..
cmake --build .
cmake --build . --target install
```
