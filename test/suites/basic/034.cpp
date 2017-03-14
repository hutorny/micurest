/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 034.cpp - cojson tests, numerics with overflow
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

template<typename T>
inline constexpr T _(T a, T b, T c) noexcept {
	return
		config::config::overflow == config::config::overflow_is::saturated ? a
	  : config::config::overflow == config::config::overflow_is::error	   ? b
	  : c;
}

static inline constexpr error_t expect() noexcept {
	return config::config::overflow == config::config::overflow_is::error ?
		error_t::overflow : error_t::noerror;
}

struct Test034 : Test {
	static Test034 tests[];
	inline Test034(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc, func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

template<typename T>
inline constexpr int odif(T a, T b) noexcept {
	return a == b ? -8 * (int)sizeof(T) : log(fabs((a-b)/(a+b)))/log(2.);
}


template<typename T>
struct plain {
	static T value;
	static void set(const T val) noexcept { value = val; }
	static result_t run(const Environment& env, cstring data,
			T answer, error_t expect = expect()) noexcept {
		value = 0;
		bool r = V<T,plain::set>().read(test::json(data));
		bool m = eq(answer,value);
		env.out(r && m, fmt<T>(), value);
		return combine2(r, m, Test::expected(test::json().error(), expect));
	}
	static constexpr bool eq(const T& a, const T& b) noexcept {
		return a == b;
	}
};

template<typename T>
T plain<T>::value;

template<>
constexpr bool plain<float>::eq(const float& a, const float& b) noexcept {
	return odif(a,b) <= -24; /* last bit error */
}

#define RUN(name, body) Test034(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test034 Test034::tests[] = {
	RUN("parsing plain values: positive char", {
		return plain<signed char>::run(env, CSTR("100"),
							_<signed char>(100,100,100), error_t::noerror);	}),
	RUN("parsing plain values: positive char with overflow", {
		return plain<signed char>::run(env, CSTR("300"),
											_<signed char>(127,127,44));	}),
	RUN("parsing plain values: negative char", {
		return plain<signed char>::run(env, CSTR("-100"),
						_<signed char>(-100,-100,-100), error_t::noerror);	}),
	RUN("parsing plain values: negative char with overflow", {
		return plain<signed char>::run(env, CSTR("-300"),
											_<signed char>(-128,-128,-44));	}),
	RUN("parsing plain values: unsigned char with overflow", {
		return plain<unsigned char>::run(env, CSTR("300"),
															_(255,255,44));	}),
	RUN("parsing plain values: negative short with overflow", {
		return plain<short>::run(env, CSTR("-40000"),
											_<short>(-32768,-32768,25536));	}),
	RUN("parsing plain values: positive short with overflow", {
		return plain<short>::run(env, CSTR("40000"), _(32767,32767,-25536));}),
	RUN("parsing plain values: unsigned short", {
		return plain<unsigned short>::run(env, CSTR("5536"),
					_<unsigned short>(5536,5536,5536), error_t::noerror);	}),
	RUN("parsing plain values: unsigned short with overflow", {
		return plain<unsigned short>::run(env, CSTR("65536"),
					_<unsigned short>(65535,65535,0));						}),
	RUN("parsing plain values: positive long with overflow", {
		return plain<long>::run(env, CSTR("2147483649"),
			_(2147483647L, 2147483647L, -2147483647L));						}),
	RUN("parsing plain values: long long", {
		return plain<long long>::run(env, CSTR("9223372036854775810"),
	_(9223372036854775807LL, 9223372036854775807LL, -9223372036854775806LL));}),
};


