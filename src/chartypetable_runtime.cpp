/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * chartypetable_runtime.cpp - char type table populated at run-time
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

#include "cojson.hpp"
namespace cojson {
namespace runtime {
using namespace details;

inline ctype& operator|=(ctype& a, int b) noexcept {
	*reinterpret_cast<int*>(&a) |= b;
	return a;
}

struct chartypetable {
	chartypetable() noexcept {
		table = this;
		lexer::char_typify(add);
		string(' ', 127);
	}
	inline ctype chartype(char_t c) const noexcept {
		if(static_cast<size_t>(c) >= countof(chartypes)) return ctype::string;
		return chartypes[(size_t)c];
	}
	static void add(const char * str,ctype traits) noexcept {
		while( *str )
			table->chartypes[(size_t)*str++] |= (int)traits;
	}
	inline void string(size_t from, size_t to) noexcept {
		while( from <= to ) chartypes[from++] |= (int) ctype::string;
	}
private:
	ctype chartypes[128];
	static chartypetable* table;
};
chartypetable* chartypetable::table;
}
namespace details {
ctype chartype(char_t c) noexcept __attribute__((weak));
ctype chartype(char_t c) noexcept {
	static const runtime::chartypetable table;
	return table.chartype(c);
} /* avr: 360 bytes for chartable,
	 or additional 112/86 comparing to the build-time chartable  */
}
}




