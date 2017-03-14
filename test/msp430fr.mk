# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# msp430fr.mk - make script to build COJSON Library tests for MSP430FR6989
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

PREFIX ?= msp430-elf-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
ASM := @$(PREFIX)gcc$(SUFFIX)
OBJ := @$(PREFIX)objcopy
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
FIND = find /opt/ti \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1


MAKEFLAGS += --no-builtin-rules

CXX-FIX := $(realpath $(BASE-DIR)/../include)

-include $(BASE-DIR)/msp430fr.vars
export PATH

LDPATH  := $(realpath $($(PREFIX)-PATH)/..)/include
INCLDIR := $(realpath $($(PREFIX)-PATH)/..)/include

MCU ?= msp430fr6989
MSP430FLAGS :=																\
  -mmcu=$(MCU)																\
  -mhwmult=f5series															\
  -msmall																	\
  -mcode-region=none														\
  -mdata-region=none														\


CXX-DEFS := 																\
  COJSON_TEST_OMIT_NAMES													\
  BENCH_DATA_ATTR='__attribute__((section(".text")))'						\


CPPFLAGS += 																\
  $(addprefix -I,$(INCLDIR) $(CXX-FIX) $(INCLUDES))							\
  $(addprefix -D,$(CXX-DEFS))												\
  $(MSP430FLAGS)															\
  -std=c++1y  																\
  -Wall  																	\
  -pedantic																	\
  -Os 																		\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -fno-rtti																	\
  -fno-use-cxa-atexit														\
  -fno-check-new															\
  -fsingle-precision-constant												\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\

CFLAGS += 																	\
  $(addprefix -I,$(INCLDIR) $(INCLUDES))									\
  $(addprefix -D,$(CXX-DEFS))												\
  $(MSP430FLAGS)															\
  -std=c11																	\
  -Os 																		\
  -Wall  																	\
  -pedantic																	\
  -fsingle-precision-constant												\
  -ffunction-sections 														\
  -fdata-sections 															\
  -ffreestanding															\

ASMFLAGS :=																	\
  -pedantic																	\
  -fsigned-char																\
  -fsingle-precision-constant												\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -x assembler-with-cpp														\

LDFLAGS +=																	\
  $(CPPFLAGS)																\
  -Xlinker --gc-sections													\
  -Wl,-s,-relax,-L$(LDPATH),-Map,$@.map										\

#METRIC-FLAGS +=															\

OFLAGS +=																	\
  -O ihex																	\

SFLAGS := 																	\
  --format=berkeley															\


COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\

OBJS := 																	\
  $(COJSON-OBJS)															\
  msp430cppfix.o															\
  msp430.o																	\
  msp430fr.o																\

msp430fr-OBJS :=															\
  080.o																		\

msp430fr.001-OBJS :=														\
  001.o																		\

msp430fr.002-OBJS :=														\
  002.o																		\

msp430fr.003-OBJS :=															\
  003.o																		\

msp430fr.004-OBJS :=														\
  004.o																		\
  004.cpp.o																	\

msp430fr.005-OBJS :=														\
  005.o																		\

msp430fr.030-OBJS :=														\
  030.o																		\

msp430fr.031-OBJS :=														\
  031.o																		\

msp430fr.032-OBJS :=														\
  032.o																		\

msp430fr.033-OBJS :=														\
  033.o																		\

msp430fr.034-OBJS :=														\
  034.o																		\

msp430fr.035-OBJS :=														\
  035.o																		\

msp430fr.036-OBJS :=														\
  036.o																		\

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
# 09-complex-object metric does not fit ROM
METRICS     := $(METRIC-SRCS:.cpp=.size)
METRIC-OBJS := 																\
  avrcppfix.o																\
  $(COJSON-OBJS)															\

vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS) $(ARDUINO-DIR))

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .hex .elf .o

.SECONDARY:

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

%.: %.o 00-base.o $(METRIC-OBJS)
	@echo "    $(BOLD)ld$(NORM)" $(notdir $<)
	$(LD) $(LDFLAGS) $(METRIC-FLAGS) -o $@ $< $(filter-out $<,$^)

$(TARGET-DIR)/%.elf: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@chmod a-x $@

$(TARGET-DIR)/%.hex: $(TARGET-DIR)/%.elf $(BASE-DIR)/msp430fr.vars
	@echo "$(BOLD)objcopy$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< $@
	$(SIZE) $(SFLAGS) $<

$(TARGET): $(TARGET-DIR)/$(TARGET).hex

$(TARGET-DIR)/$(TARGET): $(TARGET-DIR)/$(TARGET).hex

$(BASE-DIR)/msp430fr.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

$(BASE-DIR)/msp430fr.vars:
	@echo "Looking for $(BOLD)$(PREFIX)g++$(SUFFIX)$(NORM)..."
	@echo "# lookup for $(PREFIX)g++$(SUFFIX)" > $@
	@echo $(if $(shell which $(PREFIX)g++$(SUFFIX)),						\
		"# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX)),			\
		$(PREFIX)-PATH=$(dir $(shell $(FIND)))"\n"PATH=$(dir $(shell $(FIND))):$$PATH) >> $@

metrics:: $(BASE-DIR)/msp430fr.metrics.txt

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *.size													\
	$(TARGET-DIR)/$(TARGET).*.hex											\
	$(TARGET-DIR)/$(TARGET).*.elf											\
	$(TARGET-DIR)/$(TARGET).*.map											\


