# Copyright (C) 2016 Eugene Hutorny <eugene@hutorny.in.ua>
#
# esp8266.mk - make script to build COJSON Library tests for ESP8266 
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

PREFIX ?= xtensa-lx106-elf-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
ASM := @$(PREFIX)gcc$(SUFFIX)
OBJ := @esptool.py
OFLAGS := elf2image
SIZE:= @$(PREFIX)size
BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)
FIND = find $(SDK-DIR)/.. \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1


MAKEFLAGS += --no-builtin-rules

#SDK-DIR = /opt/esp-open-sdk/sdk
#SYSROOT = /opt/esp-open-sdk/xtensa-lx106-elf/xtensa-lx106-elf/sysroot
CXX-DEFS += __ets__ ICACHE_FLASH

-include $(BASE-DIR)/esp8266.vars
export PATH

ESP-INCLUDES = $(SDK-DIR)/include $(SDK-DIR)/driver_lib/include

CPPFLAGS += 																\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -std=c++1y  																\
  -Wall  																	\
  -pedantic																	\
  -Os 																		\
  -fno-exceptions															\
  -fno-threadsafe-statics													\
  -ffunction-sections														\
  -fdata-sections															\
  -fno-exceptions															\
  -fno-rtti																	\
  -fno-use-cxa-atexit														\
  -fno-check-new															\
  -fenforce-eh-specs														\
  -nostdlib																	\
  -mlongcalls																\
  -mtext-section-literals													\


CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES) $(ESP-INCLUDES))								\
  $(addprefix -D,$(CXX-DEFS))												\
  -std=gnu11																\
  -ffunction-sections 														\
  -fdata-sections 															\
  -fsigned-char																\
  -ffreestanding															\
  -g																		\
  -nostdlib																	\
  -mlongcalls																\
  -mtext-section-literals													\
  -Os																		\

LIBS += c gcc hal pp phy net80211 lwip m wpa main

LDFLAGS +=																	\
  -static																	\
  -nostdlib																	\
  -u call_user_start														\
  -ffreestanding															\
  --sysroot=$(SYSROOT)														\
  -Xlinker --gc-sections													\
  -Xlinker --no-check-sections												\
  -T $(BASE-DIR)/eagle.app.v6.mod.ld										\
  -L$(SDK-DIR)/lib															\

SFLAGS := 																	\
  --format=berkeley															\

COJSON-OBJS :=																\
  avrcppfix.o																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\

OBJS := 																	\
  $(COJSON-OBJS)															\
  esp8266_init.o															\
  esp8266_user.o															\
  esp8266_uart.o															\
  esp8266.o																	\

esp8266-OBJS :=																\
  001.o																		\
  002.o																		\
  003.o																		\
  004.o																		\
  004.cpp.o																	\
  005.o																		\
  006.o																		\

esp8266a-OBJS :=															\
  030.o																		\
  031.o																		\
  032.o																		\
  033.o																		\
  034.o																		\
  035.o																		\
  036.o																		\
  080.o																		\

# use of floats blows up size of executable
esp8266b-OBJS :=															\
  100.o																		\
  101.o																		\

100.o : FILE-FLAGS:=-Wno-overflow

esp8266_uart.o : FILE-FLAGS:=-Wno-implicit-function-declaration
esp8266_user.o : FILE-FLAGS:=-Wno-implicit-function-declaration

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
METRICS     := $(METRIC-SRCS:.cpp=.size)
METRIC-OBJS :=																\
  $(COJSON-OBJS)															\
  esp8266_metrics.o															\
  esp8266_init.o															\

vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS))

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
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $(METRIC-FLAGS) $(LDFLAGS) -o $@ 		\
		-Wl,--start-group $(filter-out $@o,$^) $(addprefix -l,$(LIBS))		\
		-Wl,--end-group

$(TARGET-DIR)/%.elf: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(addprefix -l,$(LIBS)) -Wl,--end-group
	@chmod a-x $@

$(TARGET-DIR)/%.bin: $(TARGET-DIR)/%.elf $(BASE-DIR)/esp8266.vars
	@echo "$(BOLD)esptool$(NORM)" $(notdir $@)
	$(OBJ)	$(OFLAGS) $< -o $(basename $@)-
	$(SIZE) $(SFLAGS) $<

$(TARGET): $(TARGET-DIR)/$(TARGET).bin

$(BASE-DIR)/esp8266.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

$(BASE-DIR)/esp8266.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(SDK-DIR),													\
			echo SDK-DIR:=$(SDK-DIR) > $@;									\
			echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;				\
			$(if $(shell which $(PREFIX)g++$(SUFFIX)),						\
				echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX));,\
				echo "# found in $(SDK-DIR)"								\
				"\n"PATH:=$(realpath $(dir $(shell $(FIND)))):$$PATH >> $@;),	\
			$(error $(BOLD)SDK-DIR$(NORM) not set expected path to ESP8266_NONOS_SDK))\
		$(if $(SYSROOT),													\
			echo SYSROOT:=$(SYSROOT);,										\
			echo "# lookup for sysroot" 									\
				"\n"SYSROOT:=$(realpath $(shell find $(dir $(shell 			\
					$(FIND)))/.. -type d -name sysroot)) >> $@;))			\

metrics: $(BASE-DIR)/esp8266.metrics.txt

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map *.size $(TARGET-DIR)/$(TARGET).hex 					\
	$(TARGET-DIR)/$(TARGET).elf $(TARGET-DIR)/$(TARGET)*.bin

