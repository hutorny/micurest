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
VARIANT ?= mega
FIND = find /opt/arduino* \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

MAKEFLAGS += --no-builtin-rules

-include arduino.vars
export PATH

#ARDUINO-DIR  ?=/opt/arduino
CORE-DIR	 ?=$(ARDUINO-DIR)/hardware/arduino/avr/cores/arduino
VARIANT-DIR  ?=$(ARDUINO-DIR)/hardware/arduino/avr/variants/$(VARIANT)
ETHERNET-DIR ?=$(ARDUINO-DIR)/libraries/Ethernet/src
SPI-DIR      ?=$(ARDUINO-DIR)/hardware/arduino/avr/libraries/SPI

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
  $(realpath .)																\
  $(realpath ../../include) 												\
  $(realpath ../../src) 													\
  $(VARIANT-DIR)															\
  $(CORE-DIR)																\
  $(ETHERNET-DIR)															\
  $(SPI-DIR)																\

CPPFLAGS += 																\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -std=c++1y  																\
  -Wall  																	\
  -Os 																		\
  -fshort-enums  															\
  -ffunction-sections														\
  -fdata-sections															\
  -funsigned-bitfields  													\
  -fno-rtti																	\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -fno-use-cxa-atexit														\
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
  arduino_mega2560_rpc.o													\
  avrcppfix.o 																\

MICUREST-OBJS :=															\
  cojson.o																	\
  cojson_libdep.o															\
  cojson_progmem.o															\
  micurpc.o																	\
  micurest.o																\
  micurest_progmem.o														\
  miculog_arduino.o															\
  network_arduino.o															\
  chartypetable_progmem.o													\

ARDUINO-OBJS := 															\
  HardwareSerial.o															\
  HardwareSerial0.o															\
  SPI.o																		\
  IPAddress.o																\
  Dns.o																		\
  Dhcp.o																	\
  Tone.o																	\
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

#  arduino.vars																\

$(BUILD-DIR)cojson.o				: FILE-FLAGS:=-pedantic
$(BUILD-DIR)cojson_libdep.o			: FILE-FLAGS:=-pedantic
$(BUILD-DIR)cojson_progmem.o		: FILE-FLAGS:=-pedantic
$(BUILD-DIR)micurpc.o				: FILE-FLAGS:=-pedantic
$(BUILD-DIR)micurest.o				: FILE-FLAGS:=-pedantic
$(BUILD-DIR)micurest_progmem.o		: FILE-FLAGS:=-pedantic
$(BUILD-DIR)miculog_arduino.o		: FILE-FLAGS:=-pedantic
$(BUILD-DIR)network_arduino.o		: FILE-FLAGS:=-pedantic
$(BUILD-DIR)chartypetable_progmem.o	: FILE-FLAGS:=-pedantic
$(BUILD-DIR)Dns.o					: FILE-FLAGS:=-Wno-strict-aliasing

vpath %.cpp user:../../src/:$(CORE-DIR):$(ETHERNET-DIR):$(ETHERNET-DIR)/utility:$(SPI-DIR)
vpath %.c   user:$(CORE-DIR):$(ARDUINO-DIR):$(ETHERNET-DIR):$(SPI-DIR)

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .elf .o .size .bin .hex .elf .o

.SECONDARY:

.PHONY: bin

all: $(PREREQ) bin/arduino_mega_2560.hex

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
	@$(if $(filter-out clean,$(MAKECMDGOALS),$(if $(MAKECMDGOALS),,true)),	\
		$(if $(ARDUINO-DIR),												\
			echo ARDUINO-DIR:=$(ARDUINO-DIR) > $@;,							\
			$(error ARDUINO-DIR is not set))								\
		echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;					\
		$(if $(shell which $(PREFIX)g++$(SUFFIX)),							\
		  echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX)) >>$@;,\
		  echo PATH=$(dir $(shell $(FIND))):$$PATH >> $@))

help:
	@echo $(CORE-DIR):$(ARDUINO-DIR):$(ETHERNET-DIR)
	@echo 																	\
	"\nUsage:\n\t$(BOLD)make$(NORM) ARDUINO-DIR=/path-to-arduino-studio\n"	\

run: bin/arduino_mega_2560.hex
	@echo "    $(BOLD)flash$(NORM) " $^										\
		$(if $(AVRDUDE_PORT),, "(AVRDUDE_PORT not set, using default port)")
	avrdude $(AVRDUDE_FLAGS) -Uflash:w:@$^:i

clean:
	@rm -f $(BUILD-DIR)/*.o $(BUILD-DIR)/*.map $(BUILD-DIR)/*.size 			\
	bin/*.hex bin/*.elf

distclean: clean
	@rm -f package library arduino.vars
	@rm -rf bin $(BUILD-DIR)

