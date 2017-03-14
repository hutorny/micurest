/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 102.cpp - cojson tests, reading double values
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

#include <math.h>
#include "test.hpp"

template<typename T>
inline constexpr int odif(T a, T b) noexcept {
	return a == b ? -8 * (int)sizeof(T) : log2(fabs((a-b)/(a+b)));
}

struct Test102 : Test {
	static Test102 tests[];
	inline Test102(cstring name, cstring desc, runner func)
		noexcept : Test(name, desc, func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

template<typename T>
struct plain {
	static T value;
	static void set(T val) noexcept { value = val; }
	static bool run(const Environment& env, const char_t* data, T answer)
		noexcept {
		value = 0;
		bool r = V<T,&plain::set>().read(test::json(data));
		r =  r && eq(answer,value);
		env.out(r, fmt<T>(), value);
		return r;
	}
	static constexpr bool eq(const T& a, const T& b) noexcept {
		return a == b;
	}
};

template<typename T>
T plain<T>::value;

template<>
constexpr bool plain<double>::eq(const double& a, const double& b) noexcept {
	return odif(a,b) <= -51; /* last bit error */
}

static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror, env.output.error());
}

#define RUN(name, body) Test102(__FILE__,name, \
		[](const Environment& env) noexcept -> result_t body)
Test102 Test102::tests[] = {
	RUN("parsing plain values: double no frac", {
		return _R(plain<double>::run(env,
			"400000000000000000000000000000000", 4E32), env);				}),
	RUN("parsing plain values: double with exp", {
		return _R(plain<double>::run(env, "2.225E307", 2.225E307), env);	}),
	RUN("parsing plain values: double with -exp", {
		return _R(plain<double>::run(env, "2E-37", 2E-37), env);			}),
	RUN("parsing plain values: negative double", {
		return _R(plain<double>::run(env, "-3.141528", -3.141528), env);	}),
	RUN("parsing plain values: double + trailing spaces", {
		return _R(plain<double>::run(env, "4.\n ", 4.0), env);				}),
};


