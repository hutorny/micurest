/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * cojson_progmem.cpp - progmem storage for string literals and progmem access
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

/*
 * This file provides implementation progmem methods and placements for
 * promem literals
 */
#include <avr/pgmspace.h>
#include "cojson.hpp"
namespace cojson {
namespace details {

template<>
bool match<progmem<char>, char const*>(progmem<char> a, char const* b) noexcept {
	return strcmp_P(b,static_cast<const char*>(a)) == 0;
}

template<>
char progmem<char>::read(const char * ptr) noexcept {
	return pgm_read_byte(ptr);
}
template<>
bool ostream::puts<progmem<char>>(progmem<char> i) noexcept {
	while(*i && put(*i++));
	return *i == 0;
}

bool writer<progmem<char>>::write(progmem<char> str, ostream& out) noexcept {
	bool r = true;
	if( ! out.put(literal::quotation_mark) ) return false;
	while( *str && writer<const char_t*>::write(*str++, out) );
	return r && out.put(literal::quotation_mark);
}

constexpr const char literal_strings<progmem<char>>::_null_l[] __attribute__((progmem));
constexpr const char literal_strings<progmem<char>>::_true_l[] __attribute__((progmem));
constexpr const char literal_strings<progmem<char>>::_false_l[] __attribute__((progmem));
constexpr const char literal_strings<progmem<char>>::_bom[] __attribute__((progmem));


}}
