# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# teensy3.mk - make script to build COJSON Library tests for Teensy 3.1
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

PREFIX ?= arm-none-eabi-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
ASM := @$(PREFIX)gcc$(SUFFIX)
OBJ := @$(PREFIX)objcopy
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
FIND = find /opt/gcc$(PREFIX)* \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

MAKEFLAGS += --no-builtin-rules

-include $(BASE-DIR)/teensy3.vars
export PATH

CXX-DEFS := 																\
  ARDUINO=164																\
  F_CPU=72000000UL															\
  __MK20DX256__																\
  TEENSYDUINO=124															\
  USB_SERIAL																\
  LAYOUT_US_ENGLISH															\
  COJSON_SUITE_SIZE=200														\


CPPFLAGS += 																\
  $(addprefix -I,$(CXX-FIX) $(INCLUDES) $(ARDUINO-DIR))						\
  $(addprefix -D,$(CXX-DEFS))												\
  -Wall  																	\
  -pedantic																	\
  -std=c++1y  																\
  -mcpu=cortex-m4															\
  -mthumb																	\
  -Os																		\
  -fabi-version=6															\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -fsigned-char																\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -fno-rtti																	\
  -fno-use-cxa-atexit														\


CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES) $(ARDUINO-DIR))								\
  $(addprefix -D,$(CXX-DEFS))												\
  -Wall  																	\
  -pedantic																	\
  -std=c11																	\
  -mcpu=cortex-m4															\
  -mthumb																	\
  -Os																		\
  -ffunction-sections 														\
  -fdata-sections 															\
  -fsigned-char																\
  -ffreestanding															\

ASMFLAGS :=																	\
  -pedantic																	\
  -mcpu=cortex-m4															\
  -mthumb																	\
  -Os																		\
  -fsigned-char																\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -x assembler-with-cpp														\

LDFLAGS +=																	\
  -mcpu=cortex-m4															\
  -mthumb																	\
  -Os																		\
  -fsigned-char																\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -Wl,-T"$(ARDUINO-DIR)/mk20dx256.ld"										\
  -Xlinker																	\
  --gc-sections																\
  -s																		\
  -Wl,-Map,$(TARGET).map,--cref												\

OFLAGS +=																	\
  -O ihex																	\

METRIC-FLAGS +=																\
  -Wl,-T"$(ARDUINO-DIR)/mk20dx256.ld"										\
  -Xlinker																	\
  --gc-sections																\

SFLAGS := 																	\
  --format=sysv																\
  --totals																	\

#it does not seem possible to build an app with a smaller subset of features
ARDUINO-OBJS := 															\
  analog.o																	\
  DMAChannel.o																\
  HardwareSerial1.o 														\
  HardwareSerial2.o 														\
  HardwareSerial3.o 														\
  Print.o 																	\
  avr_emulation.o 															\
  main.o 																	\
  mk20dx128.o 																\
  new.o 																	\
  nonstd.o 																	\
  pins_teensy.o 															\
  ser_print.o 																\
  serial1.o 																\
  serial2.o 																\
  serial3.o 																\
  usb_desc.o 																\
  usb_dev.o 																\
  usb_inst.o 																\
  usb_mem.o 																\
  usb_serial.o 																\
  yield.o 																	\

yield.o usb_desc.o DMAChannel.o: FILE-FLAGS:=-Wno-pedantic
nonstd.o: FILE-FLAGS:=-w

COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\


OBJS := 																	\
  $(COJSON-OBJS)															\
  $(ARDUINO-OBJS)															\
  teensy.o 																	\

TESTS-BASIC := $(wildcard $(addprefix $(BASE-DIR)/suites/basic/, *.c *.cpp))
TESTS-BENCH := $(wildcard $(addprefix $(BASE-DIR)/suites/bench/, *.c *.cpp))
TESTS-HOST  := $(wildcard $(addprefix $(BASE-DIR)/suites/host/,  *.c *.cpp))
TESTS-REST  := $(wildcard $(addprefix $(BASE-DIR)/suites/http/,  *.c *.cpp))
TESTS-ALL   := $(notdir $(TESTS-BASIC) $(TESTS-BENCH))

teensy3-OBJS := $(patsubst %.c,%.o,$(TESTS-ALL:.cpp=.o))
teensy3a-OBJS := $(patsubst %.c,%.o,$(TESTS-HOST:.cpp=.o))
teensy3r-OBJS := $(patsubst %.c,%.o,$(TESTS-REST:.cpp=.o))					\
  micurest.o																\

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
METRICS     := $(METRIC-SRCS:.cpp=.size)
METRIC-OBJS := 																\
  $(ARDUINO-OBJS)															\
  $(COJSON-OBJS)															\


vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .hex .elf .o .size

.SECONDARY:

$(BASE-DIR)/teensy3.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(ARDUINO-DIR),												\
			echo ARDUINO-DIR:=$(ARDUINO-DIR) > $@;,							\
			$(error ARDUINO-DIR is not set))								\
		echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;					\
		$(if $(shell which $(PREFIX)g++$(SUFFIX)),							\
		  echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX)) >>$@;,\
		  echo PATH=$(dir $(shell $(FIND))):$$PATH >> $@))

%.o: %.c
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) $(FILE-FLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) -c -o $@ $<

%.o: %.S
	@echo "    $(BOLD)asm$(NORM)" $(notdir $<)
	$(ASM) $(ASMFLAGS) $(FILE-FLAGS) -c -o $@ $<

%.size: %.
	$(SIZE) $< > $@

%.: %.cpp 00-base.o $(METRIC-OBJS)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $(METRIC-FLAGS) -o $@ $(filter-out $@o,$^)

$(TARGET-DIR)/%.elf: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@chmod a-x $@

$(TARGET-DIR)/%.hex: $(TARGET-DIR)/%.elf $(BASE-DIR)/teensy3.vars
	@echo "$(BOLD)objcopy$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< $@
	$(SIZE) $(SFLAGS) $<

$(TARGET): $(TARGET-DIR)/$(TARGET).hex

$(BASE-DIR)/teensy3.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

metrics: $(BASE-DIR)/teensy3.metrics.txt

run: $(TARGET)
	@echo "    $(BOLD)flash$(NORM) " $(TARGET)
	teensy_loader_cli -mmcu=mk20dx256 -w $(TARGET-DIR)/$(TARGET)

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *.size $(TARGET-DIR)/$(TARGET).hex $(TARGET-DIR)/$(TARGET).elf

