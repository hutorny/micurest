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

CC  := @$(PREFIX)gcc$(SUFFIX)
CXX := @$(PREFIX)g++$(SUFFIX)
LD  := @$(PREFIX)g++$(SUFFIX)

BOLD:=$(shell tput bold)
NORM:=$(shell tput sgr0)


MAKEFLAGS += --no-builtin-rules


COJSON-OBJS :=																\
  common.o 																	\
  cojson.o																	\
  cojson_libdep.o															\
  chartypetable_runtime.o													\
  coop.o																	\
  host.o																	\
  printf_char16_char32.o													\


TESTS-BASIC := $(wildcard $(addprefix $(BASE-DIR)/suites/basic/, *.c *.cpp))
TESTS-BENCH := $(wildcard $(addprefix $(BASE-DIR)/suites/bench/, *.c *.cpp))
TESTS-HOST  := $(wildcard $(addprefix $(BASE-DIR)/suites/host/,  *.c *.cpp))
TESTS-REST  := $(wildcard $(addprefix $(BASE-DIR)/suites/http/,  *.c *.cpp))
TESTS-ALL   := $(sort $(notdir 													\
  $(TESTS-BASIC)															\
  $(TESTS-BENCH)    														\
  $(TESTS-HOST)     														\
  $(TESTS-REST)     														\
))

micurest-OBJS := 															\
  micurest.o																\
  micurpc.o																	\

host-OBJS   := $(patsubst %.c,%.o,$(TESTS-ALL:.cpp=.o) $(micurest-OBJS))

wchar-DEFS        := TEST_WCHAR_T
char16-DEFS       := TEST_CHAR16_T
char32-DEFS       := TEST_CHAR32_T
overflow-DEFS     := TEST_OVERFLOW_ERROR
saturate-DEFS     := TEST_OVERFLOW_SATURATE
sprintf-DEFS      := TEST_WITH_SPRINTF

wchar-INCLUDES    := $(BASE-DIR)/suites/wchar
char16-INCLUDES   := $(BASE-DIR)/suites/wchar
char32-INCLUDES   := $(BASE-DIR)/suites/wchar
overflow-INCLUDES := $(BASE-DIR)/suites/basic
saturate-INCLUDES := $(BASE-DIR)/suites/basic
sprintf-INCLUDES  := $(BASE-DIR)/suites/basic

uchar-OBJS        := $(host-OBJS)
sprintf-OBJS      := $(host-OBJS)
wchar-OBJS        := 070.o
char16-OBJS	      := 071.o
char32-OBJS	      := 072.o
overflow-OBJS     := 034.o
saturate-OBJS     := 034.o

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


.DEFAULT:

.SUFFIXES:
.SUFFIXES: .hex .elf .o

vpath %.cpp $(subst $(eval) ,:,$(SRC-DIRS))
vpath %.c   $(subst $(eval) ,:,$(SRC-DIRS))

%.o: %.c
	@echo "     $(BOLD)cc$(NORM)" $(notdir $<)
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "    $(BOLD)c++$(NORM)" $(notdir $<)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

.SECONDARY:

$(TARGET-DIR)/%: $(OBJS) $($(TARGET)-OBJS)
	@echo "    $(BOLD)ld$(NORM) " $(notdir $@)
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET-DIR):
	@mkdir -p $(TARGET-DIR)

uchar: CPPFLAGS += -funsigned-char

$(TARGET): $(TARGET-DIR)/$(TARGET)

run: $(TARGET)
	@echo "    $(BOLD)run$(NORM) " $(TARGET)
	@$(TARGET-DIR)/$(TARGET)

rebuild: clean $(TARGET)

clean:
	@rm -f *.o *.map $(TARGET-DIR)/$(TARGET)


