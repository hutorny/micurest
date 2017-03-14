/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * chartypetable.cpp - char type table populated at build-time
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
#include "cojson.hpp"

using namespace cojson;
using namespace details;

static const int_fast16_t chartypes[128] = {
#include "chartypetable.inc"
};

namespace cojson {
namespace details {
ctype chartype(char_t c) noexcept {
	if(static_cast<size_t>(c) >= countof(chartypes)) return ctype::string;
	return static_cast<ctype>(chartypes[(size_t)c]);
}
}
}




