# Arduino JSON-RPC Example

This is an example of implementing JSON-RPC services for an Arduino platform, or 
more specifically - `Arduino Mega` board. The intent of this example is to 
demonstrate `μcuRPC` capabilities and show technics for implementing RPC API 
 resources with `μcuRPC`.


# Build Steps

This section describes how to build this example from command line.
To build with Arduino Studio, please use examples that come with Micurest
Arduino Library. 


## Install gcc-avr

`sudo apt-get install gcc-avr`

## Install Arduino IDE

Please follow instructions from this [Guide](https://www.arduino.cc/en/Guide/Linux)

## Apply a workaround

Make links to <type_traits>, <limits> c++0x_warning.h in ../../includes directory
```
cd ../../include
ln -s /usr/include/c++/4.9/limits   
ln -s /usr/include/c++/4.9/type_traits
ln -s /usr/include/c++/4.9/bits/c++0x_warning.h bits/ 
```
You may need to adjust commands above if you have gcc version other than 4.9

## Build this example

Run the following command

`make PACKAGE-DIR=/path-to-controllino-package LIBRARY-DIR=/path-to-controllino-library`

Succesfully built exaple will be placed in `bin/` directory
