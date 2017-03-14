# Copyright (C) 2016 Eugene Hutorny <eugene@hutorny.in.ua>
#
# smart.mk - make script to build COJSON Library tests for 
#			Mediatek Smart Linkit 7688
#
# This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
#
# The COJSON Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License v2
# as published by the Free Software Foundation;
#
# The COJSON Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the COJSON Library; if not, see
# <http://www.gnu.org/licenses/gpl-2.0.html>.

# This makefile builds unit tests for cojson
# It is not intended to build any other applications
# variable expected from the outer Makefile: 
# BASE-DIR SRC-DIRS INCLUDES TARGET TARGET-DIR

#/opt/arduino-1.6.4/hardware/tools/avr/bin/avr-g++ -c -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega32u4 -DF_CPU=8000000L -DARDUINO=10604 -DARDUINO_AVR_LINKITSMART7688 -DARDUINO_ARCH_AVR -DUSB_VID=0x0E8D -DUSB_PID=0xAB01 -DUSB_MANUFACTURER="MediaTek Labs" -DUSB_PRODUCT="LinkIt Smart 7688 Duo" -I/opt/arduino-1.6.4/hardware/arduino/avr/cores/arduino -I/home/eugene/.arduino15/packages/LinkIt/hardware/avr/0.1.5/variants/smart7688
#/opt/arduino-1.6.4/hardware/tools/avr/bin/avr-objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 /tmp/build2100887461208674711.tmp/sketch_jun20a.cpp.elf
PREFIX ?= avr-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
OBJ := @$(PREFIX)objcopy
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
FIND = find /opt/arduino* \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

CXX-FIX := $(realpath $(BASE-DIR)/../include)
MAKEFLAGS += --no-builtin-rules

-include $(BASE-DIR)/smart.vars
export PATH

AVRDUDE_FLAGS +=															\
  -D																		\
  -C/etc/avrdude.conf														\
  -patmega32u4																\
  -cwiring																	\
  -b115200																	\
  $(if $(AVRDUDE_PORT),-P $(AVRDUDE_PORT))									\

CXX-DEFS := 																\
  ARDUINO_AVR_LINKITSMART7688												\
  ARDUINO_ARCH_AVR															\
  ARDUINO=10604																\
  F_CPU=8000000L															\
  COJSON_SUITE_SIZE=4														\
  USB_VID=0x0E8D															\
  USB_PID=0xAB01															\
  USB_MANUFACTURER="\"MediaTek Labs\""										\
  USB_PRODUCT="\"LinkIt Smart 7688 Duo\""									\

CPPFLAGS += 																\
  $(addprefix -I,															\
      $(CXX-FIX) 															\
      $($(TARGET)-INCLUDES) 												\
      $(INCLUDES) 															\
      $(VARIANT-DIR)														\
      $(ARDUINO-DIR))														\
  $(addprefix -D,$($(TARGET)-DEFS) $(CXX-DEFS))								\
  -Wall  																	\
  -Os  																		\
  -fshort-enums  															\
  -ffunction-sections  														\
  -fdata-sections  															\
  -funsigned-bitfields  													\
  -fno-exceptions  															\
  -fno-threadsafe-statics													\
  -std=c++11  																\
  -mmcu=atmega32u4 															\


CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES) $(VARIANT-DIR) $(ARDUINO-DIR))					\
  $(addprefix -D,$(CXX-DEFS))												\
  -Wall																		\
  -Os 																		\
  -fpack-struct 															\
  -fshort-enums 															\
  -ffunction-sections 														\
  -fdata-sections 															\
  -funsigned-bitfields 														\
  -mmcu=atmega32u4															\


LDFLAGS +=																	\
  -Wl,-Map,$(TARGET).map,--cref 											\
  -mrelax 																	\
  -Wl,--gc-sections			 												\
  -mmcu=atmega32u4															\


OFLAGS +=																	\
  -j .eeprom 																\
  --set-section-flags=.eeprom=alloc,load									\
  --no-change-warnings														\
  --change-section-lma .eeprom=0											\
  -R .fuse 																	\
  -R .lock 																	\
  -R .signature 															\
  -O ihex																	\


SFLAGS := 																	\
  --format=avr																\
  --mcu=atmega32u4															\

ARDUINO-OBJS := 															\
  HardwareSerial.o															\
  HardwareSerial1.o															\
  Print.o																	\
  main.o																	\
  wiring.o																	\
  wiring_digital.o															\
  hooks.o																	\
  WInterrupts.o																\
  abi.o																		\

#  CDC.o																		\

#  HID.o																		\
#  USBCore.o																	\

#  wiring_analog.o															\
#  wiring_pulse.o															\
#  wiring_shift.o															\
#  HardwareSerial1.o															\
#  HardwareSerial2.o															\
#  HardwareSerial3.o															\

#  IPAddress.o
#  new.o
#  Tone.o
#  WMath.o
#  WString.o

COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\

OBJS :=																		\
  $(COJSON-OBJS)															\
  $(ARDUINO-OBJS)															\
  avrcppfix.o 																\
  chartypetable_progmem.o													\
  cojson_progmem.o															\

smart-OBJS := 																\
  smart.o 																	\
  001.o																		\
  002.o																		\

smarta-OBJS := 																\
  smart.o 																	\
  003.o																		\

smartb-OBJS := 																\
  smart.o 																	\
  080.o																		\

smartr-OBJS := 																\
  smart-http.o																\
  micurest.o																\
  cojson_progmem.o															\
  micurest_progmem.o														\
  Print.o																	\
  WString.o																	\
  Stream.o																	\
  CDC.o																		\
  HID.o																		\
  USBCore.o																	\

smart-DEFS :=																\
  COJSON_TEST_OMIT_NAMES													\
  CSTRING_PROGMEM															\


smartr-DEFS :=																\
  CSTRING_PROGMEM															\

smart-INCLUDES :=															\
  $(BASE-DIR)/suites/basic													\

smartr-INCLUDES :=															\
  $(BASE-DIR)/suites/basic													\


100.o : FILE-FLAGS:=-Wno-overflow

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
METRICS     := $(METRIC-SRCS:.cpp=.size)
METRIC-OBJS :=																\
  wiring.o																	\
  main.o																	\
  $(COJSON-OBJS)															\
  avrcppfix.o 																\
  chartypetable_progmem.o													\
  cojson_progmem.o															\
  hooks.o																	\
  CDC.o																		\
  USBCore.o																	\
  HID.o																		\
  Print.o																	\
  wiring.o																	\

METRIC-FLAGS :=																\
  -Wl,--gc-sections			 												\

vpath %.cpp $(subst $(eval) ,:,$(VARIANT-DIR) $(ARDUINO-DIR) $(SRC-DIRS))
vpath %.c   $(subst $(eval) ,:,$(VARIANT-DIR) $(ARDUINO-DIR) $(SRC-DIRS))

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .hex .elf .o .size .bin

.SECONDARY:

$(BASE-DIR)/smart.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(ARDUINO-DIR),												\
			echo ARDUINO-DIR:=$(ARDUINO-DIR) > $@;,							\
			$(error ARDUINO-DIR is not set))								\
		$(if $(VARIANT-DIR),												\
			echo VARIANT-DIR:=$(VARIANT-DIR) >> $@;,						\
			$(error ARDUINO-DIR is not set))								\
		echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;					\
		$(if $(shell which $(PREFIX)g++$(SUFFIX)),							\
			echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX))>>$@;,\
			echo PATH=$(dir $(shell $(FIND))):$$PATH >> $@))

%.o: %.c
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) $(FILE-FLAGS) $($(TARGET)-FLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $($(TARGET)-FLAGS) -c -o $@ $<

%.size: %.
	$(SIZE) $< > $@

%.: %.cpp 00-base.o $(METRIC-OBJS)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $(METRIC-FLAGS) -o $@ $(filter-out $@o,$^)

$(TARGET-DIR)/%.elf: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@chmod a-x $@

$(TARGET-DIR)/%.hex: $(TARGET-DIR)/%.elf $(BASE-DIR)/smart.vars
	@echo "$(BOLD)objcopy$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< $@
	$(SIZE) $(SFLAGS) $<

$(BASE-DIR)/smart.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

metrics: $(BASE-DIR)/smart.metrics.txt

$(TARGET): $(TARGET-DIR)/$(TARGET).hex

run: $(TARGET)
	@echo "    $(BOLD)flash$(NORM) " $(TARGET)							\
		$(if $(AVRDUDE_PORT),, "(AVRDUDE_PORT not set, using default port)")
	avrdude $(AVRDUDE_FLAGS) -Uflash:w:@$(TARGET-DIR)/$(TARGET):i

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *. *.size $(TARGET-DIR)/$(TARGET).hex $(TARGET-DIR)/$(TARGET).elf

