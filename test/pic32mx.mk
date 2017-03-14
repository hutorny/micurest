# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# pic32mx.mk - make script to build COJSON Library tests for PIC32MX130F256B
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

PREFIX ?= xc32-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
ASM := @$(PREFIX)gcc$(SUFFIX)
OBJ := @$(PREFIX)bin2hex
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
FIND = find /opt/microchip \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

MAKEFLAGS += --no-builtin-rules

CXX-FIX := $(realpath $(BASE-DIR)/../include)

-include $(BASE-DIR)/pic32mx.vars
export PATH

CPPFLAGS += 																\
  $(addprefix -I,$(CXX-FIX) $(INCLUDES))									\
  $(addprefix -D,$(CXX-DEFS))												\
  -mprocessor=32MX130F256B													\
  -std=c++1y  																\
  -membedded-data															\
  -Wall  																	\
  -pedantic																	\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -fno-rtti																	\
  -fno-use-cxa-atexit														\
  -fno-check-new															\
  -fenforce-eh-specs														\

CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -mprocessor=32MX130F256B													\
  -std=c11																	\
  -Wall  																	\
  -pedantic																	\
  -ffunction-sections 														\
  -fdata-sections 															\
  -fsigned-char																\
  -ffreestanding															\

ASMFLAGS :=																	\
  -mprocessor=32MX130F256B													\
  -pedantic																	\
  -fsigned-char																\
  -ffunction-sections														\
  -fdata-sections															\
  -ffreestanding															\
  -x assembler-with-cpp														\


LDFLAGS +=																	\
  -ffreestanding															\
  -Xlinker																	\
  --gc-sections																\
  -Wl,--defsym=_min_heap_size=4												\

#  
METRIC-FLAGS +=																\
  -ffreestanding															\
  -Xlinker																	\
  --gc-sections																\
  -Wl,--defsym=_min_heap_size=4												\

#OFLAGS +=																	\

SFLAGS := 																	\
  --format=berkeley															\


COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\

OBJS := 																	\
  $(COJSON-OBJS)															\
  pic32mx.o																	\

#this set of tests probably exceeds ROM capacity of 32MX130F256B
pic32mx-OBJS :=																\
  001.o																		\
  002.o																		\
  003.o																		\
  004.o																		\
  004.cpp.o																	\
  005.o																		\
  030.o																		\
  031.o																		\
  032.o																		\
  033.o																		\
  034.o																		\
  035.o																		\
  036.o																		\
  080.o																		\
  100.o																		\
  101.o																		\

100.o : FILE-FLAGS:=-Wno-overflow

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
METRICS     := $(METRIC-SRCS:.cpp=.size)
METRIC-OBJS := 																\
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

%.: %.cpp 00-base.o $(METRIC-OBJS)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $(METRIC-FLAGS) -o $@ $(filter-out $@o,$^)

$(TARGET-DIR)/%.elf: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@chmod a-x $@

$(TARGET-DIR)/%.hex: $(TARGET-DIR)/%.elf $(BASE-DIR)/pic32mx.vars
	@echo "$(BOLD)bin2hex$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $<
	$(SIZE) $(SFLAGS) $<

$(TARGET): $(TARGET-DIR)/$(TARGET).hex

$(BASE-DIR)/pic32mx.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

$(BASE-DIR)/pic32mx.vars:
	@echo "# lookup for $(PREFIX)g++$(SUFFIX)" > $@
	@echo $(if $(shell which $(PREFIX)g++$(SUFFIX)),						\
		"# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX)),			\
		PATH=$(dir $(shell $(FIND))):$$PATH)  >> $@

metrics: $(BASE-DIR)/pic32mx.metrics.txt

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *.size $(TARGET-DIR)/$(TARGET).hex $(TARGET-DIR)/$(TARGET).elf

