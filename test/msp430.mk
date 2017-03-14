# Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
#
# msp430.mk - make script to build COJSON Library tests for MSP430FR6989
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

.DEFAULT:

.SUFFIXES:

.SECONDARY:

msp430fr:																	\
  msp430fr.001																\
  msp430fr.002																\
  msp430fr.003																\
  msp430fr.004																\
  msp430fr.005																\
  msp430fr.030																\
  msp430fr.031																\
  msp430fr.032																\
  msp430fr.033																\
  msp430fr.034																\
  msp430fr.035																\
  msp430fr.036																\

rebuild: clean msp430fr

clean:
	@rm -f *. *.o *.map *.size												\
	$(TARGET-DIR)/msp430fr*.hex												\
	$(TARGET-DIR)/msp430fr*.elf												\
	$(TARGET-DIR)/msp430fr*.map												\

%::
	@$(if $(filter clean $(TARGET),$(MAKECMDGOALS)),,echo "$(BOLD)make$(NORM) $@")
	@$(MAKE) $(filter clean,$(MAKECMDGOALS)) $(filter-out $(TARGET),$@)		\
		--no-print-directory 												\
		-f $(BASE-DIR)/msp430fr.mk											\
		TARGET=$(filter-out metrics, $@)									\

