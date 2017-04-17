/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * utils.cpp - implementation of utility methods
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "utils.hpp"

bool ip_addr::write(cojson::details::ostream& out) const noexcept {
	using cojson::details::writer;
	return
		out.put('"') &&	writer<uint8_t>::write(0xFF & (addr>>24), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>>16), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>> 8), out) &&
		out.put('.') && writer<uint8_t>::write(0xFF & (addr>> 0), out) &&
		out.put('"');
}

bool ip_addr::read(cojson::details::lexer& in) noexcept {
	using namespace cojson;
	using namespace cojson::details;
	char_t buffer[sizeof(ip_addr)*4];
	if(!cojson::details::reader<char_t*>::read(buffer, sizeof(buffer), in))
		return false;
	char_t* curr = buffer;
	unsigned val = 0;
	uint_fast8_t shft = 0;
	uint_fast8_t cnt = 0;

	decltype(addr) adr = 0;
	for(;curr<buffer+sizeof(buffer);++curr) {
		if( +(chartype(*curr) & ctype::digit) ) {
			if( ! tenfold<decltype(val)>(val,*curr-'0') ) return false;
			if( val > 0xFF ) return false;
			++cnt;
		} else {
			if( *curr == 0 || *curr == '.' ) {
				if( cnt == 0 ) return false;
				adr |= static_cast<decltype(addr)>(val) << shft;
				if( *curr == 0 ) {
					addr = adr;
					return true;
				}
				shft += 8;
				cnt = 0;
				val = 0;
			}
		}
	}
	return false;
}

static inline bool ashex(uint8_t v, cojson::details::ostream& out) noexcept {
	return out.put(cojson::details::ashex(v >> 4)) &&
		   out.put(cojson::details::ashex(v & 0xF));
}

bool mac_addr::write(cojson::details::ostream& out) const noexcept {
	char dlm = '"';
	for(size_t i=0; i<cojson::details::countof(addr); ++i) {
		out.put(dlm);
		ashex(addr[i], out);
		dlm = ':';
	}
	return out.put('"');
}

bool mac_addr::read(cojson::details::lexer& in) noexcept {
	using namespace cojson;
	using namespace cojson::details;
	char_t buffer[sizeof(mac_addr)*3];
	if(!reader<char_t*>::read(buffer, sizeof(buffer), in))
		return false;
	char_t* curr = buffer;
	unsigned val = 0;
	uint_fast8_t cnt = 0;
	uint_fast8_t pos = 0;
	ctype ctp;

	uint8_t adr[len] = {};
	for(;curr<buffer+sizeof(buffer);++curr) {
		if(+(ctp = chartype(*curr) & (ctype::digit|ctype::hex|ctype::heX))){
			val <<= 4;
			val |= *curr - (ctp == ctype::digit
					? '0' : ctp == ctype::hex
					? 'a'-0xA : 'A'-0xA);
			if( val > 0xFF ) return false;
			++cnt;
		} else {
			if( *curr == 0 || *curr == ':' ) {
				if( cnt != 2 ) return false;
				adr[pos++] = val;
				if( *curr == 0 ) {
					assign(addr, adr);
					return true;
				}
				cnt = 0;
				val = 0;
			}
		}
	}
	return false;
}
