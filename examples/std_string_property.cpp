/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * std_string_property.cpp - example for using cojson with std strings
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
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include <string>
#include "cojson.hpp"

namespace cojson {
namespace details {
template<>
bool reader<std::string>::read(std::string& dst, lexer& in) noexcept  {
	std::string tmp;
	bool first = true;
	ctype ct;
	char chr;
	while( (ct=in.string(chr, first)) == ctype::string ) {
		tmp += chr;
		first = false;
	}
	if( chr ) {
		in.error(error_t::bad);
		return false;
	}
	dst = tmp;
	return true;
}

template<>
bool writer<std::string>::write(const std::string& str, ostream& out) noexcept {
	return writer<const char*>::write(str.c_str(), out);
}
}}

using namespace cojson;
using namespace std;

class Pdo {
	string str;
	static constexpr const char* name() noexcept { return "str"; }
	inline const details::clas<Pdo>& json() noexcept {
		return O<Pdo, P<Pdo, name, string, &Pdo::str>>();
	}
public:
	inline bool read(details::istream& in) noexcept {
		details::lexer lex(in);
		return json().read(*this, lex);
	}
	inline bool write(details::ostream& out) noexcept {
		return json().write(*this, out);
	}
};

void std_string_example(details::iostream& inout) {
	Pdo pdo;
	pdo.read(inout);
	pdo.write(inout);
}

