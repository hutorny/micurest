/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * json_via_arduino_serial.cpp - example for using cojson with Arduiono Serial
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
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
 * You should have received a copy of the GNU General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "cojson.hpp"
#include "Arduino.h"
#ifdef CSTRING_PROGMEM
#	include <avr/pgmspace.h>
#	define NAME(s) static inline progmem<char> s() noexcept { 			\
		static const char l[] __attribute__((progmem)) = #s; 			\
		return progmem<char>(l);}
#else
#	define NAME(s) static inline constexpr const char_t* s() noexcept {	\
		return #s;}
#endif

using namespace cojson;
using cojson::details::progmem;

class jsonw : public details::ostream {
	Print& out;
public:
	inline jsonw(Print& o) noexcept : out(o) {}
	bool put(char_t c) noexcept {
		return out.write(c) == 1;
	}
};

class jsonr : public details::lexer, private details::istream {
	Stream& in;
private:
	bool get(char_t& c) noexcept {
		int r = in.read();
		if( r == -1 ) {
			istream::error(details::error_t::eof);
			c = iostate::eos_c;
			return false;
		}
		c = static_cast<char_t>(r);
		return true;
	}
public:
	inline jsonr(Stream& i) noexcept
	  :	lexer(static_cast<istream&>(*this)), in(i) {}
};

template<int id>
struct pin {
	static bool get() noexcept {
		return digitalRead(id) == HIGH;
	}
	static void set(bool val) noexcept {
		digitalWrite(id, val);
	}
};


struct led : pin<13> {
	struct name {
		NAME(led)
	};
	static const details::value& json() noexcept {
		return V<M<name::led,accessor::functions<bool, led::get,led::set>>>();
	}
};

void json_read_write() {
	jsonr in(Serial);
	jsonw out(Serial);
	led::json().read(in);
	led::json().write(out);
}

