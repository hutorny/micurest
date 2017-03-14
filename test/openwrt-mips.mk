# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# host.mk - make script to build COJSON Library tests for the host
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

PREFIX ?= mips-openwrt-linux-
CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)
SIZE:= @$(PREFIX)size

#STAGING_DIR

BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)


FIND = find $(STAGING_DIR) \( -readable -or \! -prune \) \
  \( -type f -o -type l \) -name  $(PREFIX)g++$(SUFFIX) | tail -1

MAKEFLAGS += --no-builtin-rules

-include $(BASE-DIR)/openwrt-mips.vars
export PATH
export STAGING_DIR

COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\
  coop.o																	\
  host.o																	\

METRIC-OBJS := 																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable.o															\


TESTS-BASIC := $(wildcard $(addprefix $(BASE-DIR)/suites/basic/, *.c *.cpp))
TESTS-BENCH := $(wildcard $(addprefix $(BASE-DIR)/suites/bench/, *.c *.cpp))
TESTS-HOST  := 100.cpp 101.cpp #compilation fails on 102.cpp
TESTS-ALL   := $(notdir $(TESTS-BASIC) $(TESTS-BENCH) $(TESTS-HOST))

openwrt-mips-OBJS  := $(patsubst %.c,%.o,$(TESTS-ALL:.cpp=.o))
openwrt-mips-uchar-OBJS := $(openwrt-mips-OBJS)

OBJS := 																	\
  $(COJSON-OBJS)															\


CPPFLAGS += 																\
  $(addprefix -I,$($(TARGET)-INCLUDES) $(INCLUDES))							\
  $(addprefix -D,$($(TARGET)-DEFS) $(CXX-DEFS))								\
  -Wall																		\
  -Wextra																	\
  -pedantic																	\
  -pedantic-errors															\
  -O3																		\
  -fmessage-length=0														\
  -ffunction-sections  														\
  -fdata-sections															\
  -std=c++1y  																\


CFLAGS += 																	\
  $(addprefix -I,$(INCLUDES))												\
  $(addprefix -D,$(CXX-DEFS))												\
  -Wall																		\
  -O3																		\
  -ffunction-sections 														\
  -fdata-sections 															\
  -std=gnu99 																\


LDFLAGS +=																	\
  -s						 												\
  -L$(LIBPATH)																\

METRIC-FLAGS := 															\
  -Os																		\

METRIC-SRCS := $(notdir $(wildcard $(BASE-DIR)/suites/metrics/*.cpp))
METRICS     := $(METRIC-SRCS:.cpp=.size)

vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS))

.DEFAULT:

.SECONDARY:

.SUFFIXES:
.SUFFIXES: .hex .elf .o

$(TARGET)-uchar: CPPFLAGS += -funsigned-char

$(BASE-DIR)/openwrt-mips.vars:
	@$(if $(filter-out clean,$(MAKECMDGOALS)),								\
		$(if $(STAGING_DIR),												\
			echo STAGING_DIR:=$(STAGING_DIR) > $@;,							\
			$(error STAGING_DIR is not set)))
	@echo "# lookup for $(PREFIX)g++$(SUFFIX)" >> $@
	@echo $(if $(shell which $(PREFIX)g++$(SUFFIX)),						\
		"# found in path\n# "$(shell which $(PREFIX)g++$(SUFFIX)),			\
		PATH=$(dir $(shell $(FIND))):$$PATH) >> $@
	@echo LIBPATH=$(realpath $(dir $(shell $(FIND)))/../usr/lib) >> $@

%.o: %.c
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

%.size: %.
	$(SIZE) $< > $@

%.: %.cpp 00-base.o $(METRIC-OBJS)
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) $(FILE-FLAGS) $(METRIC-FLAGS) -o $@ $(filter-out $@o,$^)


$(TARGET-DIR)/%: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET-DIR):
	@mkdir -p $(TARGET-DIR)

$(TARGET): $(BASE-DIR)/openwrt-mips.vars $(TARGET-DIR)/$(TARGET)

$(BASE-DIR)/openwrt-mips.metrics.txt: $(METRICS)
	@head -1 $< > $@
	@grep -h -v filename  $(sort $^) >> $@
	@cat $@

metrics: $(BASE-DIR)/openwrt-mips.metrics.txt

run: $(TARGET)
	@echo "    $(BOLD)run$(NORM) " $(TARGET)
	@$(TARGET-DIR)/$(TARGET)

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map $(TARGET-DIR)/$(TARGET)


