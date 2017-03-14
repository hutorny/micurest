## micurest
 Application server for constrained platforms

### Introduction

`μcuREST` is a C++11 library for implementing REST services on constrained 
platforms, such as bare metal applications on low-end MCUs. The library is
platform-agnostic, zero-allocation, and has almost no external dependencies. 
It allows mapping hierarchically organized URIs to C/C++ variables, 
constants and methods, and exposing the URIs via a HTTP protocol.

`μcuREST` is a continuation of `cojson` project - it includes `cojson` as its 
integral part and uses many feature, itroduced in cojson.

### HTTP standard

`μcuREST` implements a minimal subset of `HTTP/1.1` specification, sufficient for
serving `GET` requests from browsers, `GET/PUT/POST` requests from JavaScript via
`XMLHttpRequest` and simple machine-to-machine communications.

### Transport Layer

An application, implementing HTTP RESTful services with `μcuREST` needs a 
platform specific transport layer and a mediator (Session Layer in terms of 
OSI model) between `μcuREST` and the transport layer. 
The mediator must handle sessions and implement two `μcuREST` interfaces: 
`istream` and `ostream`.

### Limitations

Query strings, cookies, absolute URLs, cache control, transfer encoding, 
and many other HTTP features are not supported.
Depending on the transport layer in use, and mediator implementation, 
the HTTP message size could be limited to size of one MTU (~1460 bytes).

### Requirements

* **Compiler:** `μcuREST` sources need a `C++11` enabled compiler, such as 
   `g++-4.8` and up.
* **Library:** `libstdc++ v3` highly desirable. But if not available a 
    workaround exists
* **Transport layer:** A platform specific transport layer, such as TCP/IP, is
    required for implementing HTTP REST services.
* **ROM/RAM space:** on AVR platform μcuREST core takes 6K of ROM and 100 bytes 
    of RAM. Size of the resource map starts with 3K/200 and grows with its 
    complexity, taking in average 750 bytes of ROM and 250 bytes of RAM per 
    entry.

### Tested On
* **Debian** `i686`, `g++-4.9.2`
* **Controllino MAXI** `ATmega2560`, `avr-g++-4.8.1`
* **NodeMCU V3** `ESP8266` `xtensa-lx106-elf-g++-4.8.5`

Please visit project's [`home page`](http://hutorny.in.ua/projects/micurest) 
and [`tutorial`](http://hutorny.in.ua/projects/micurest-tutorial) for more 
details


## cojson
 C++ pull-type `JSON` parser/generator for constrained platforms

### COJSON Introduction

`cojson` is a C++ pull-type `JSON` parser/serializer for constrained platforms,
such as bare metal applications on low-end MCUs. It does not use memory 
allocation and has almost no external dependencies. It is not intrusive - it 
neither forces nor implies any particular design of the application. 
Instead it adapts to fit any existing application code. 
It is tolerant to data type mismatching. When such occurs, parser just skips 
mismatching data and makes best efforts to continue parsing. 

The parser is recursive, e.g. nested `JSON` elements are handled with the 
recursion. However, this recursion is driven by the structure definition, not by
the input data, which prevents stack faults on malformed input data.

`cojson` is character type neutral - it can work with signed or unsigned 
character, as well as with standard wide character types: 
*`wchar_t`*, *`char16_t`* and *`char32_t`*. 

It is also transparent for `UTF8` and properly handles `BOM` sequence.

`cojson` works against a user-defined structure which specifies hierarchy, 
data types, and data storage access methods. Thus, when parsing is complete, 
the data already delivered to the application and no further processing needed.

The same structure definition is also used for writing `JSON`.
The `JSON` structure is defined with a set of templetized function. 

Please visit project's [`home page`](http://hutorny.in.ua/projects/cojson) 
and [`tutorial`](http://hutorny.in.ua/projects/cojson-tutorial) for more details

### Requirements

* **Compiler:** cojson sources need a `C++11` enabled compiler, such as 
   `g++-4.8` and up.
* **Library:** `libstdc++ v3` highly desirable. But if not available a 
    workaround exists
* **Code space:** Depending on the platform and `JSON` structure complexity 
    varies from 4kB to 20kB.
* **RAM space:** 20-80 bytes per entry in the defined `JSON` structure

### Tested On
* **Debian** `i686`, `g++-4.9.2`
* **Arduino Mega** `ATmega2560`, `avr-g++-4.8.1`
* **Teensy 3.1** `ARM Cortex-M4` `arm-none-eabi-g++-4.8.3`
* **Carambola2** `Atheros AR9331` `mips-openwrt-linux-g++-4.8.3`
* **MSP430FR6989** `MSP430FR6989` `msp430-elf-g++-4.9.1`
* **NodeMCU V3** `ESP8266` `xtensa-lx106-elf-g++-4.8.5`

