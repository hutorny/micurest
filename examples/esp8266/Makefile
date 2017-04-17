# Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
#
# Makefile - make script to build µcuREST example for ESP8266 
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
# You should have received a copy of the GNU General Public License v2
# along with the COJSON Library; if not, see
# <http://www.gnu.org/licenses/gpl-2.0.html>.
 

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
BUILD-DIR ?= /tmp/$(USER)/micurest/example/esp8266/
FIND = find $(SDK-DIR)/.. \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1


MAKEFLAGS += --no-builtin-rules

-include esp8266.vars
export PATH

INCLUDES +=																	\
  user																		\
  $(realpath ../../src) 													\
  $(SDK-DIR)/include 														\
  $(SDK-DIR)/driver_lib/include												\


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
  $(addprefix -I,$(INCLUDES))												\
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
  -Wno-implicit-function-declaration										\
  -Wno-parentheses															\
  -Wno-unused-value															\


LIBS += c gcc hal pp phy net80211 lwip m wpa main ssl

LDFLAGS +=																	\
  -static																	\
  -nostdlib																	\
  -u call_user_start														\
  -ffreestanding															\
  --sysroot=$(SYSROOT)														\
  -Xlinker --gc-sections													\
  -Xlinker --no-check-sections												\
  -T user/eagle.app.v6.mod.ld												\
  -L$(SDK-DIR)/lib															\

SFLAGS := 																	\
  --format=berkeley															\

MICUREST-OBJS :=															\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\
  micurest.o																\
  miculog_esp.o																\
  network_esp8266.o															\

ESP8266-OBJS :=																\
  avrcppfix.o																\
  esp_gpio.o																\
  esp_persist.o																\
  esp_setup.o																\
  esp_sysinfo.o																\
  user_main.o																\
  user_server.o																\
  uart.o																	\

$(BUILD-DIR)esp_gpio.o : FILE-FLAGS := -Wno-parentheses
  
OBJS += $(addprefix $(BUILD-DIR),$(MICUREST-OBJS) $(ESP8266-OBJS))  

vpath %.cpp user:../../src/:
vpath %.c   user:$(SDK-DIR)/driver_lib/driver

.DEFAULT:

.SUFFIXES:
.SUFFIXES: .0.inc .1.inc .2.inc .0 .1 .2 .inc .elf .o .size .bin .hex .elf .o

all: firmware bin

firmware:
	@mkdir -p $@
	
bin: esp8266.vars firmware/0x10000.bin firmware/0x00000.bin 

$(BUILD-DIR):
	@mkdir -p $@

firmware/0x00000.bin firmware/0x10000.bin: firmware/esp8266.elf | esp8266.vars
	@echo "    $(BOLD)obj$(NORM)" $@
	$(OBJ) $(OFLAGS) $< -o firmware/ > /dev/null
  
firmware/esp8266.elf: $(OBJS) | $(BUILD-DIR) inc esp8266.vars
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(addprefix -l,$(LIBS)) -Wl,--end-group		

$(BUILD-DIR)%.o: %.c | $(BUILD-DIR)
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) $(FILE-FLAGS) -c -o $@ $<

$(BUILD-DIR)%.o: %.cpp | $(BUILD-DIR)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) -c -o $@ $<
 	
esp8266.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(SDK-DIR),													\
			echo SDK-DIR:=$(SDK-DIR) > $@;									\
			echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@;				\
			$(if $(shell which $(PREFIX)g++$(SUFFIX)),						\
				echo "# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX));,\
				echo "# found in $(SDK-DIR)"								\
				"\n"PATH:=$(realpath $(dir $(shell $(FIND)))):$$PATH >> $@;),	\
			$(error $(BOLD)SDK-DIR$(NORM) not set. Expected path to ESP8266_NONOS_SDK))\
		$(if $(SYSROOT),													\
			echo SYSROOT:=$(SYSROOT);,										\
			echo "# lookup for sysroot" 									\
				"\n"SYSROOT:=$(realpath $(shell find $(dir $(shell 			\
					$(FIND)))/.. -type d -name sysroot)) >> $@;))			\
	
help:
	@echo $$PATH	
	@echo "Usage:\n\t $(BOLD)make$(NORM) SDK-DIR=/path-to-esp-sdk" \
	"SYSROOT=/path-to-esp-sysroot" 

inc:																		\
	user/index.html.0.inc													\
	user/index.html.1.inc													\
	user/index.html.2.inc													\
	user/meta.json.inc														\

tools/node_modules/json-minify:
	tools/install.sh

%.json.inc: %.json | tools/node_modules/json-minify
	@tools/json-minify $^ | file2c > $@

%.inc: %
	@file2c < $^ > $@
	@rm $^

%.0 %.1 %.2: %
	@csplit -s --suppress-matched --prefix=user/index.html --suffix-format=.%d $^ '/^<!--SPLIT-->/' '{2}'
	@rm user/index.html.3
    
clean:
	@rm -f $(BUILD-DIR)/*.o $(BUILD-DIR)/*.map $(BUILD-DIR)/*.size 			\
	firmware/esp8266.elf firmware/0x00000.bin  firmware/0x10000.bin
