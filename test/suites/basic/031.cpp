/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 031.cpp - cojson tests, reading strings
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 *
 * The COJSON Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License v2
 * as published by the Free Software Foundation;
 *
 * The COJSON Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "test.hpp"
namespace cojson {
namespace test {

struct Test031 : Test {
	static Test031 tests[];
	inline Test031(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc, func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

#define RUN(name, body) Test031(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test031 Test031::tests[] = {
	RUN("parsing string: plain string", {
		return sstr<31>::run(env, CSTR("\"qwerty\""), "qwerty");			}),
	RUN("parsing string: exact length string", {
		return sstr<31>::run(env, CSTR("\"0123456789012345678901234567891\""),
					"0123456789012345678901234567891");						}),
	RUN("parsing string: escaped to a r/w string", {
		return sstr<31>::run(env, CSTR("\"quotes: \\\"\""), "quotes: \"");	}),
	RUN("parsing string: plain to a r/o string", {
		return sstr<31>::run0(env, CSTR("\"qwerty\""),
				"_______________________________");							}),
	RUN("parsing string: leading spaces", {
		return sstr<31>::run(env, CSTR("\n\t\t  \"NORMAL\""), "NORMAL"); 	}),
	RUN("parsing string: common escapes", {
		return sstr<31>::run(env, CSTR("\"\\b\\f\\n\\r\\tFIVE\""),
				"\b\f\n\r\tFIVE"); 											}),
	RUN("parsing string: hex codes", {
		return sstr<31>::run(env, CSTR("\"\\u0040\\u004F\\u00A2\\u00C3\""),
				"\x40\x4F\xA2\xC3"); }),
	RUN("parsing string: UTF-8", {
		return sstr<31>::run(env, CSTR("\"Йцуке Ğä Ψσμα\""), "Йцуке Ğä Ψσμα");}),
	RUN("parsing string: string + trailing spaces", {
		return sstr<31>::run(env, CSTR("\"qwerty\" \n"), "qwerty");			}),
	RUN("parsing string: string + excessive string", {
		return sstr<31>::run(env, CSTR("\"string\" \"excessive\"\n"),
				"string");													}),
	RUN("parsing string: null", {
		return sstr<31>::run(env, CSTR("null"), "");						}),
	RUN("parsing string: mismatching true", {
		return sstr<31>::run(env, CSTR("true"),
				"_______________________________", error_t::mismatch);		}),
	RUN("parsing string: mismatching array", {
		return sstr<31>::run(env, CSTR("[\"string\"]"),
				"_______________________________", error_t::mismatch);		}),
	RUN("parsing string: mismatching object", {
		return sstr<31>::run(env, CSTR("{\"a\":\"string\"}"),
				"_______________________________", error_t::mismatch);		}),
	RUN("parsing string: length overrun", {
		return sstr<31>::run(env, CSTR("\"01234567890123456789012345678912\""),
				"0123456789012345678901234567891", error_t::overrun);		}),
	RUN("parsing string: with BOM", {
		return sstr<31>::run(env, CSTR(
				"\xEF\xBB\xBF\"0123456789012345678901234567891\""),
				"0123456789012345678901234567891");							}),

};
}}

