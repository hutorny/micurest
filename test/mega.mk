# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# mega.mk - make script to build COJSON Library tests for Arduino Mega
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

-include $(BASE-DIR)/mega.vars
export PATH

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
  COJSON_SUITE_SIZE=100														\

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
  -std=c++1y  																\
  -mmcu=atmega2560 															\


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
  -mmcu=atmega2560															\


LDFLAGS +=																	\
  -Wl,-Map,$(TARGET).map,--cref 											\
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


ARDUINO-OBJS := 															\
  HardwareSerial.o															\
  HardwareSerial0.o															\
  Print.o																	\
  hooks.o																	\
  main.o																	\
  wiring.o																	\
  wiring_digital.o															\


COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  cojson_progmem.o															\
  chartypetable_progmem.o													\

OBJS :=																		\
  $(COJSON-OBJS)															\
  $(ARDUINO-OBJS)															\
  arduino.o 																\
  avrcppfix.o 																\

megab-OBJS := 																\
  080.o																		\

megaa-OBJS := 																\
  031.o																		\
  032.o																		\
  033.o																		\
  034.o																		\
  035.o																		\
  036.o																		\
  100.o																		\
  101.o																		\

mega-OBJS := 																\
  001.o																		\
  002.o																		\
  003.o																		\
  004.o																		\
  004.cpp.o																	\
  005.o																		\
  006.o																		\
  030.o																		\

megap-OBJS := 																\
  $(mega-OBJS)																\

megaq-OBJS := 																\
  $(megaa-OBJS)																\

megar-OBJS := 																\
  010.o																		\
  micurest.o																\
  micurest_progmem.o														\

mega-DEFS :=																\
  CSTRING_PROGMEM COJSON_TEST_OMIT_NAMES									\

megaa-DEFS :=																\
  CSTRING_PROGMEM COJSON_TEST_OMIT_NAMES									\

megab-DEFS :=																\
  CSTRING_PROGMEM															\

megap-DEFS :=																\
  CSTRING_PROGMEM															\

megaq-DEFS :=																\
  CSTRING_PROGMEM															\

megar-DEFS :=																\
  CSTRING_PROGMEM															\

megab-INCLUDES :=															\
  $(BASE-DIR)/suites/basic													\
  
megap-INCLUDES :=															\
  $(BASE-DIR)/suites/basic													\

megaq-INCLUDES :=															\
  $(BASE-DIR)/suites/basic													\

megar-INCLUDES :=															\
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

METRIC-FLAGS :=																\
  -Wl,--gc-sections			 												\

vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .hex .elf .o .size .bin

.SECONDARY:

$(BASE-DIR)/mega.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(ARDUINO-DIR),												\
			echo ARDUINO-DIR:=$(ARDUINO-DIR) > $@;,							\
			$(error ARDUINO-DIR is not set))								\
		$(if $(VARIANT-DIR),												\
			echo VARIANT-DIR:=$(VARIANT-DIR) >> $@;,						\
			$(error VARIANT-DIR is not set))								\
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

$(TARGET-DIR)/%.hex: $(TARGET-DIR)/%.elf $(BASE-DIR)/mega.vars
	@echo "$(BOLD)objcopy$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< $@
	$(SIZE) $(SFLAGS) $<

$(BASE-DIR)/mega.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

metrics: $(BASE-DIR)/mega.metrics.txt

$(TARGET): $(TARGET-DIR)/$(TARGET).hex

run: $(TARGET)
	@echo "    $(BOLD)flash$(NORM) " $(TARGET)							\
		$(if $(AVRDUDE_PORT),, "(AVRDUDE_PORT not set, using default port)")
	avrdude $(AVRDUDE_FLAGS) -Uflash:w:@$(TARGET-DIR)/$(TARGET):i

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *.size $(TARGET-DIR)/$(TARGET).hex $(TARGET-DIR)/$(TARGET).elf

