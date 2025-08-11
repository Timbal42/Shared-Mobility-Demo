# C Interface for Joinable Timers

This is a header-only C interface for timer objects. No actual implementation is provided rather developers can use the header to have the same code running on different platforms by linking to concrete implementations.

## Example

```c
#include "ifx/timer.h"

// Set a timer for 100us
Timer timer;
timer_set(&timer, 100);

// Check if timer has elapsed
if (!timer_has_elapsed(&timer))
{
    // Wait for timer to finish
    timer_join(&timer);
}

// Clear timer
timer_destroy(&timer);
```

## Build and Installation

The build uses `cmake` and only uses standard configuration parameters. No actual build steps are required but rather `cmake` is used to distribute the interface. To simplify version control in-source builds are disallowed.

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
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX="%APPDATA%/cmake/timer" ..
cmake --build .
cmake --build . --target install
```
