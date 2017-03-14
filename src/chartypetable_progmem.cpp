/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * chartypetable_progmem.cpp - avr-specific char type table stored in progmem
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The COJSON Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The COJSON Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include <stdint.h>
#include <avr/pgmspace.h>
#include "cojson.hpp"

using namespace cojson;
using namespace details;
static const short chartypes[128] __attribute__((progmem)) = {
/* size considerations
 * 1. __attribute__((progmem)) saves 256 bytes of Data memory
 * 2. chartypetable contains 28 unique values,
 * if necessary, it can further be compacted to save
 * 128-(2*28) 72 bytes of Program memory */
#include "chartypetable.inc"
};

namespace cojson {
namespace details {
ctype chartype(char_t c) noexcept {
	if(static_cast<size_t>(c) >= countof(chartypes)) return ctype::string;
	return static_cast<ctype>(pgm_read_word(chartypes+(size_t)c));
} /* avr: 284 bytes (Program/Data) */
}
}




