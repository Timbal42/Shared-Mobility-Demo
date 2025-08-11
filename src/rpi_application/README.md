# Additonal Setup Notes

This project demonstrates how to use the *Infineon's Blockchain Security 2Go Starterkit R2* using Infineon libraries. 

## Requirements

To run the Python application, at least Python 3.7 is required.
Additionaly, the C library for [bcm2835](https://www.airspayce.com/mikem/bcm2835/index.html) need to be installed. 


```
cd ~                  
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz
tar xvfz bcm2835-1.68.tar.gz
cd bcm2835-1.68
./configure
make
sudo make install

```

## Build

The build uses `cmake` and only uses standard configuration parameters.

```
mkdir build
cd build
cmake -USE_I2C=ON ..
cmake --build .
```

If you are building on Windows with Visual Studio you need to set up a correct environment before calling `cmake`. Visual Studio offers a utility script called *VsDevComd.bat* located in its *Common7/Tools* directory (e.g. *C:/Program Files (x86)/Microsoft  Visual Studio/2017/Professional/Common7/Tools*) to set everything up.

```
"C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/Common7/Tools/VsDevCmd.bat"
mkdir build
cd build
cmake -G "Ninja" ..
cmake --build .
```

## Set up the Raspberry Pi

The example expects an Infineon Blockchain Security 2Go R2 secure element to be connected to the standard I2C pins on a Raspberry Pi (see this [tutorial](https://learn.sparkfun.com/tutorials/raspberry-pi-spi-and-i2c-tutorial/all) for details on setting up the Raspberry for I2C communication):

* `SDA` -> `Pin 3`
* `SCL` -> `Pin 5`
* `VCC` -> `Pin 1`
* `GND` -> `Pin 6`
