/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * json_via_std_stream.cpp - example for using cojson with std streams
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

#include <iostream>
#include "cojson.hpp"

using namespace cojson;
using namespace std;

class jsonw : public details::ostream {
private:
	std::ostream & out;
public:
	inline jsonw(std::ostream& o) noexcept : out(o) {}
	bool put(char_t c) noexcept {
		if( out.put(c).good() ) return true;
		error(details::error_t::ioerror);
		return false;
	}
};

class jsonr : public details::lexer, details::istream {
	std::istream& in;
private:
	bool get(char_t& c) noexcept {
		if( in.get(c).good() ) return true;
		if( in.eof() ) {
			istream::error(details::error_t::eof);
			c = iostate::eos_c;
		} else {
			istream::error(details::error_t::ioerror);
			c = iostate::err_c;
		}
		return false;
	}
public:
	inline jsonr(std::istream& i) noexcept
	  :	lexer(static_cast<istream&>(*this)), in(i) {}
};

struct led {
	static bool val;
	static bool get() noexcept {
		return val;
	}
	static void set(bool v) noexcept {
		val = v;
	}

	static constexpr const char* name() noexcept { return "led"; }
	static const details::value& json() noexcept {
		return V<M<led::name,accessor::functions<bool, led::get,led::set>>>();
	}
};

bool led::val = false;

void json_read_write() {
	jsonr in(cin);
	jsonw out(cout);
	led::json().read(in);
	led::json().write(out);
}

