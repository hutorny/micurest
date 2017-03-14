# ESP8266 Example

This is an example of implementing REST services for an ESP8266 platform, or 
more specifically - `NodeMCU V3` SOM. The intent of this example is to 
demonstrate `μcuREST` capabilities and show technics for implementing various 
REST resources with `μcuREST`: plain variables, complex objects, 
custom-stringified variables (such as IP and MAC addresses), 
web pages and binary data. 

Please see [about.html](web/about.html) for a list of published URIs

## Build Steps

### 1. Install Toolchain

Please follow [this link](https://github.com/esp8266/esp8266-wiki/wiki/Toolchain)
for instructions on building the toolchain.

### Install buildutils

Install freebsd-buildutils: `sudo apt-get install freebsd-buildutils`

### Build the firmware

To build the example, run `make` in the example/esp8266 directory as the 
following:
```
 make SDK-DIR=/path-to-esp-sdk SYSROOT=/path-to-esp-sysroot
```
where `/path-to-esp-sdk` and `/path-to-esp-sysroot` points to 
`ESP-NONOS-SDK` directory Xtensa sysroot respectively. These paths will 
be stored in a local file `esp8266.vars` and further builds may be started 
simply with `make`.

Upon build completion, binaries will be placed in `firmware` directory.

## Flashing

Please follow [this link](https://nodemcu.readthedocs.io/en/master/en/flash/)
for instructions on flashing the built firmware to ESP8266 SOM.

### Addresses

The firmware build for this example should be flashed to ROM addresses 
as the following:

   `firmware/0x00000.bin` to `0x00000`
   `firmware/0x10000.bin` to `0x10000`

## Known Issues

### Short responses issue
For some reason, short responses, such as 204 No Content are not received
by Firefox - a connection seem to be reset before the data is received.
To avoid this problem, this example uses POST, which on the `µcuREST` side is 
implemented as a combination of `PUT` and `GET`.
In Chrome, this behaviour differs - it sends two packets per request and 
in some cases gets the response in some others - not. 

### Random response loses
When requests are coming too frequent Crome and Opera loses part responses.
Firefox somehow manages to avoid losses

### Fragile TLS
TLS stack on ESP8266 is corrupted quickly. Not sure why.

 