# Arduino Example

This is an example of implementing REST services for an Arduino platform, or 
more specifically - `Controllino MAXI` PLC. The intent of this example is to 
demonstrate `μcuREST` capabilities and show technics for implementing various 
REST resources with `μcuREST`: plain variables, objects, custom-stringified 
variables (such as IP and MAC addresses), web pages and binary data. 

Please see [about.html](web/about.html) for a list of published URIs

# Build Steps

## Install gcc-avr

`sudo apt-get install gcc-avr`

## Install Arduino IDE

Please follow instructions from this [Guide](https://www.arduino.cc/en/Guide/Linux)

## Install Controllino libraries

Please follow instructions from this [Guide](https://github.com/CONTROLLINO-PLC/CONTROLLINO_Library#installation-guide)

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
