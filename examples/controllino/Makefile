# Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
#
# Makefile - make script to build µcuREST example for Controllino MAXI 
#
# This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
#
# The µcuREST Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License v2
# as published by the Free Software Foundation;
#
# The µcuREST Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the COJSON Library; if not, see
# <http://www.gnu.org/licenses/gpl-2.0.html>.

PREFIX ?= avr-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
ASM := @$(PREFIX)gcc$(SUFFIX)
OBJ := @$(PREFIX)objcopy
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
BUILD-DIR ?= /tmp/$(USER)/micurest/example/arduino/
PACKAGE-VER ?= 1.1.0
VARIANT ?= Controllino_maxi
ARDUINO-DIRS ?= /home/$(USER)/.arduino* /home/$(USER)/Arduino*
FIND = find /opt/arduino* \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

# make can't handle names with spaces http://savannah.gnu.org/bugs/?712
# Controllino packages dir has them: 'CONTROLLINO Boards'
# To make this example buildable, 'CONTROLLINO Boards' is linked as 'package'

LOOKUP-PKG = $(shell find $(ARDUINO-DIRS) \( -readable -or \! -prune \) \
  \( -type d -o -type l \) -name $(VARIANT) | tail -1)
LINK-PKG = "`realpath "$(LOOKUP-PKG)/../../../../.."`"
LOOKUP-LIB = $(shell find $(ARDUINO-DIRS) \( -readable -or \! -prune \) \
  \( -type d -o -type l \) -name CONTROLLINO | tail -1)
LINK-LIB = "`realpath "$(LOOKUP-LIB)"`"

MAKEFLAGS += --no-builtin-rules

-include arduino.vars
export PATH

ARDUINO-DIR  ?=package/hardware/avr/$(PACKAGE-VER)/cores/arduino
VARIANT-DIR  ?=package/hardware/avr/$(PACKAGE-VER)/variants/$(VARIANT)
ETHERNET-DIR ?=package/hardware/avr/$(PACKAGE-VER)/libraries/Ethernet/src
SPI-DIR      ?=package/hardware/avr/$(PACKAGE-VER)/libraries/SPI

AVRDUDE_FLAGS +=															\
  -D																		\
  -C/etc/avrdude.conf														\
  -patmega2560																\
  -cwiring																	\
  -b115200																	\
  $(if $(AVRDUDE_PORT),-P $(AVRDUDE_PORT))									\

CXX-DEFS := 																\
  ARDUINO=164																\
  F_CPU=16000000UL															\
  ARDUINO_ARCH_AVR															\
  ARDUINO_AVR_MEGA2560														\

INCLUDES +=																	\
  user																		\
  library																	\
  $(realpath ../../include) 												\
  $(realpath ../../src) 													\
  $(VARIANT-DIR)															\
  $(ARDUINO-DIR)															\
  $(ETHERNET-DIR)															\
  $(SPI-DIR)																\

CPPFLAGS += 																\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -std=c++1y  																\
  -Wall  																	\
  -pedantic																	\
  -Os 																		\
  -fshort-enums  															\
  -ffunction-sections														\
  -fdata-sections															\
  -funsigned-bitfields  													\
  -fno-rtti																	\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -fno-use-cxa-atexit														\
  -std=c++1y  																\
  -mmcu=atmega2560 															\


CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -std=gnu11																\
  -Os																		\
  -fpack-struct 															\
  -fshort-enums 															\
  -ffunction-sections 														\
  -fdata-sections 															\
  -funsigned-bitfields 														\
  -mmcu=atmega2560															\

LIBS += c gcc hal pp phy net80211 lwip m wpa main ssl

LDFLAGS +=																	\
  -Wl,-Map,bin/micurest-example.map,--cref 									\
  -mrelax 																	\
  -Wl,--gc-sections			 												\
  -mmcu=atmega2560															\

OFLAGS +=																	\
  -R .eeprom 																\
  -R .fuse 																	\
  -R .lock 																	\
  -R .signature 															\
  -O ihex																	\


SFLAGS := 																	\
  --format=avr																\
  --mcu=atmega2560															\

EXAMPLE-OBJS :=																\
  user_server.o																\
  user.o																	\
  utils.o																	\
  avrcppfix.o 																\

MICUREST-OBJS :=															\
  cojson.o																	\
  cojson_libdep.o															\
  cojson_progmem.o															\
  micurest.o																\
  micurest_progmem.o														\
  miculog_arduino.o															\
  network_arduino.o															\
  chartypetable_progmem.o													\

ARDUINO-OBJS := 															\
  Controllino.o																\
  HardwareSerial.o															\
  HardwareSerial0.o															\
  SPI.o																		\
  IPAddress.o																\
  Dns.o																		\
  Dhcp.o																	\
  Ethernet.o																\
  EthernetClient.o															\
  EthernetServer.o															\
  EthernetUdp.o																\
  Print.o																	\
  hooks.o																	\
  main.o																	\
  wiring.o																	\
  wiring_digital.o															\
  wiring_analog.o															\
  socket.o																	\
  w5100.o																	\
  WMath.o																	\

OBJS += $(addprefix $(BUILD-DIR),$(MICUREST-OBJS) $(ARDUINO-OBJS) $(EXAMPLE-OBJS))

# prerequisites
PREREQ :=																	\
  $(BUILD-DIR)																\
  bin																		\
  arduino.vars																\
  package																	\
  library																	\
  verify-prereq																\

$(BUILD-DIR)Controllino.o	: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)SPI.o			: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)EthernetServer.o: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)EthernetClient.o: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)Ethernet.o		: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)EthernetUdp.o	: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)socket.o		: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)w5100.o			: FILE-FLAGS:=-Wno-pedantic
$(BUILD-DIR)Dns.o			: FILE-FLAGS:=-Wno-pedantic -Wno-strict-aliasing
$(BUILD-DIR)Dhcp.o			: FILE-FLAGS:=-Wno-pedantic

vpath %.cpp user:library:../../src/:$(VARIANT-DIR):$(ARDUINO-DIR):$(ETHERNET-DIR):$(ETHERNET-DIR)/utility:$(SPI-DIR)
vpath %.c   user:$(VARIANT-DIR):$(ARDUINO-DIR):$(ETHERNET-DIR):$(SPI-DIR)

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .0.inc .1.inc .2.inc .0 .1 .2 .inc .elf .o .size .bin .hex .elf .o

.SECONDARY:

.PHONY: bin

all: $(PREREQ) bin/micurest-example.hex

library:
	@ln -s $(if $(LIBRARY-DIR),$(LIBRARY-DIR),$(LINK-LIB)) library

package:
	@ln -s $(if $(PACKAGE-DIR),$(PACKAGE-DIR),$(LINK-PKG)) package

bin:
	@mkdir -p $@

$(BUILD-DIR):
	@mkdir -p $@

bin/%.elf: $(OBJS) | $(PREREQ)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@chmod a-x $@

bin/%.hex: bin/%.elf | $(PREREQ)
	@echo "$(BOLD)objcopy$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< $@
	$(SIZE) $(SFLAGS) $<

$(BUILD-DIR)%.o: %.c | $(PREREQ)
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) $(FILE-FLAGS) -c -o $@ $<

$(BUILD-DIR)%.o: %.cpp | $(PREREQ)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) -c -o $@ $<
 	
arduino.vars:
	@$(if $(filter distclean clean help inc,$(MAKECMDGOALS)),,				\
		$(if $(PACKAGE-DIR),												\
			echo PACKAGE-DIR:=$(PACKAGE-DIR) > $@;,							\
			echo "#PACKAGE-DIR is not set, default will be used"> $@;)		\
		echo	"\n"> $@;													\
		$(if $(LIBRARY-DIR),												\
			echo LIBRARY-DIR:=$(LIBRARY-DIR) >> $@;,						\
			echo "#LIBRARY-DIR is not set, default will be used">> $@;)		\
		echo	"\n"> $@;													\
		echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;					\
		$(if $(shell which $(PREFIX)g++$(SUFFIX)),							\
			echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX))>>$@;,\
			echo PATH=$(dir $(shell $(FIND))):$$PATH >> $@))

verify-prereq: $(VARIANT-DIR)/pins_arduino.h library/Controllino.h

$(VARIANT-DIR)/pins_arduino.h: package
	@if [ ! $@ ]; then echo "$@ is not found\n";								\
		@echo "Install Controllino package or rerun with other PACKAGE-DIR and VARIANT"; fi;

library/Controllino.h: library
	@if [ ! $@ ]; then echo "$@ is not found\n";								\
		@echo "Install Controllino package or rerun with other LIBRARY-DIR"; fi;

help:
	@echo 																	\
	"\nUsage:\n\t$(BOLD)make$(NORM) PACKAGE-DIR=/path-to-controllino-package"\
	" LIBRARY-DIR=/path-to-controllino-library\n"\
	"\nNote: first run resolves dependencies and may fail with error"\
	"\n\tNo rule to make target"\
	"\n\nJust run it again. If PACKAGE-DIR is not set, makefile will "\
	"attempt to locate controllino\npackage and library in well know places:"\
	"$(addprefix \n\r,$(ARDUINO-DIRS))"\
	"\n\nwith the following results:"
	@realpath "$(LOOKUP-PKG)/../../../../.."
	@realpath "$(LOOKUP-LIB)"
	@echo

inc:																		\
	user/index.html.0.inc													\
	user/index.html.1.inc													\
	user/index.html.2.inc													\

%.inc: %
	@file2c < $^ > $@
	@rm $^

%.0 %.1 %.2: %
	@csplit -s --suppress-matched --prefix=user/index.html --suffix-format=.%d $^ '/^<!--SPLIT-->/' '{2}'
	@rm user/index.html.3

run: bin/micurest-example.hex
	@echo "    $(BOLD)flash$(NORM) " $^										\
		$(if $(AVRDUDE_PORT),, "(AVRDUDE_PORT not set, using default port)")
	avrdude $(AVRDUDE_FLAGS) -Uflash:w:@$^:i

clean:
	@rm -f $(BUILD-DIR)/*.o $(BUILD-DIR)/*.map $(BUILD-DIR)/*.size 			\
	bin/*.hex bin/*.elf

distclean: clean
	@rm -f package library arduino.vars
	@rm -rf bin $(BUILD-DIR)

