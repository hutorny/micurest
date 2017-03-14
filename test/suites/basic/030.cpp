/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 030.cpp - cojson tests, reading plain values
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
#include <math.h>

struct Test030 : Test {
	static Test030 tests[];
	inline Test030(cstring name, cstring desc, runner func) noexcept
		: Test(name, desc, func) {}
	int index() const noexcept  {
		return (this-tests);
	}
};


template<typename T>
struct plain {
	static T value;
	static void set(T val) noexcept { value = val; }
	static result_t run(const Environment& env, cstring data,
			T answer, error_t expected = error_t::noerror)
		noexcept  {
		value = 0;
		bool r = V<T,plain::set>().read(test::json(data)) xor
				(expected != error_t::noerror);
		bool m = eq(answer,value) && (test::json().error() == expected);
		env.out(r && m, fmt<T>(), value);
		return combine2(r, m, test::json().error() xor expected);
	}
	static result_t run2(const Environment& env, cstring data,
			T answer, error_t expected = error_t::noerror)
		noexcept  {
		value = 0;
		bool r = V<T,&plain::value>().read(test::json(data)) xor
				(expected != error_t::noerror);
		bool m = eq(answer,value) && (test::json().error() == expected);
		env.out(r && m, fmt<T>(), value);
		return combine2(r, m, test::json().error() xor expected);
	}

	static constexpr bool eq(const T& a, const T& b) noexcept {
		return a == b;
	}
};

template<typename T>
T plain<T>::value;

inline constexpr int odif(float a, float b) noexcept {
	return a == b ? -8 * (int)sizeof(float) : log(fabs((a-b)/(a+b)))/log(2.);
}

template<>
constexpr bool plain<float>::eq(const float& a, const float& b) noexcept {
	return odif(a,b) <= -24; /* last bit error */
}


#define RUN(name, body) Test030(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test030 Test030::tests[] = {
	RUN("parsing plain values: char", {
		return plain<signed char>::run(env, CSTR("125"), 125);				}),
	RUN("parsing plain values: unsigned char", {
		return plain<unsigned char>::run(env, CSTR("255"), 255);			}),
	RUN("parsing plain values: short", {
		return plain<short>::run2(env, CSTR("-32000"), -32000);				}),
	RUN("parsing plain values: unsigned short", {
		return plain<unsigned short>::run(env, CSTR("65000"), 65000);		}),
	RUN("parsing plain values: int", {
		return plain<long>::run(env, CSTR("1000000"), 1000000L);			}),
	RUN("parsing plain values: unsigned long long", {
		return plain<unsigned long long>::run(
				env, CSTR("10900900900900900900"), 10900900900900900900ULL);}),
	RUN("parsing plain values: float", {
			return plain<float>::run(env, CSTR("1.5"), 1.5);				}),
	RUN("parsing plain values: float with exp", {
			return plain<float>::run(env, CSTR("1.5e3"), 1.5e3);			}),
	RUN("parsing plain values: float with -exp", {
			return plain<float>::run(env, CSTR("2.7E-3"), 2.7e-3);			}),
	RUN("parsing plain values: negative float", {
			return plain<float>::run(env, CSTR("-3.141528"), -3.141528);	}),
	RUN("parsing plain values: int + trailing spaces", {
		return plain<int>::run(env, CSTR("4\n "), 4);						}),
	RUN("parsing plain values: short w/ excessive element", {
		return plain<short>::run(env, CSTR("400 500"), 400);				}),
	RUN("parsing plain values: long w/ excessive null", {
		return plain<long>::run(env, CSTR("800 null"), 800);				}),
	RUN("parsing plain values: long mismatching to null", {
		return plain<long>::run(env, CSTR("null 900"), 0, error_t::mismatch);}),
	RUN("parsing plain values: int mismatching to string", {
		return plain<int>::run(env, CSTR("\"null\" 900"), 0,
													error_t::mismatch);		}),
};


