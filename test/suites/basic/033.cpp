/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 033.cpp - cojson tests, reading heterogeneous arrays
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
#include <string.h>

template<typename T>
inline constexpr int odif(T a, T b) noexcept {
	return a == b ? -8 * (int)sizeof(T) : log(fabs((a-b)/(a+b)))/log(2);
}


struct Test033 : Test {
	static Test033 tests[];
	inline Test033(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc, func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

static named_static<unsigned, 33> uint33(CSTR("uint"), 0);
static named_static<long long, 33> llong(CSTR("llong"), 0LL);
static named_static<float, 33> dbl(CSTR("dbl"), 0.0);
static static_string<char, 16, 33> str33;
static static_string<short, 4, 33> ashort;
static static_string<char, 16, 330> str330;

static const value& array1() noexcept {
	return V<
		V<decltype(uint33)::type, decltype(uint33)::ref>,
		V<decltype(dbl)::type,  decltype(dbl)::set>,
		V<decltype(llong)::type,decltype(llong)::ptr>,
		V<decltype(str33)::size, decltype(str33)::ptr>
	>();
}

struct master1 {
	decltype(uint33)::type  _uint;
	decltype(dbl)::type   _dbl;
	decltype(llong)::type _llong;
	decltype(str33)::type _str33[decltype(str33)::size];
	inline bool match() const noexcept {
		return
			_uint == uint33 &&
			(odif(_dbl, dbl.value) < -22) &&
			_llong == llong &&
			(strcmp(_str33, str33.data) == 0);
	}
	inline bool run(const Environment& env) const noexcept {
		bool r = match();
		env.out(r, "[%d, %g, %lld, \"%s\"]\n", uint33.value, (double)dbl.value, llong.value,
			str33.get());
		clear();
		return r;
	}
	inline void clear() const noexcept {
		uint33 = 0;
		dbl = 0.;
		llong = 0;
		memset(str33.data,0,sizeof(_str33));
	}
};

static const value& array2() noexcept {
	return V<
		V<decltype(ashort)::type,decltype(ashort)::arr>,
		V<decltype(str330)::size, decltype(str330)::ptr>,
		V<decltype(uint33)::type,  decltype(uint33)::ref>
	>();
}

struct master2 {
	decltype(ashort)::type  _ashort[decltype(ashort)::size];
	decltype(str330)::type _str330[decltype(str330)::size];
	decltype(uint33)::type   _uint;
	inline bool match() const noexcept {
		return
			_uint == uint33 &&
			(memcmp(_ashort,ashort.data, sizeof(_ashort)) == 0) &&
			(strcmp(_str330, str330.data) == 0);
	}
	inline bool run(const Environment& env) const noexcept {
		bool r = match();
		env.out(r, "[[%d, %d, %d, %d], \"%s\", %d]\n",
				*ashort.arr(0), *ashort.arr(1), *ashort.arr(2), *ashort.arr(3),
				str330.get(), uint33.value);
		clear();
		return r;
	}
	inline void clear() const noexcept {
		memset(ashort.data,0,sizeof(_ashort));
		memset(str330.data,0,sizeof(_str330));
		uint33 = 0.;
	}
};

#define COMMA ,
#define RUN(name, body) Test033(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test033 Test033::tests[] = {
	RUN("parsing array: array1 of various", {
		return runx<array1 COMMA master1>(env,
		  CSTR("[23000, 4.5e3, 500000000000, \"test string\"]"),
		master1 {23000, 4.5e3, 500000000000LL, "test string"}); }),
	RUN("parsing array: array1 w/o spaces", {
		return runx<array1 COMMA master1>(env,
		CSTR(  "[3,0.,5,\"3,0.,5\"]"),
		master1 {3, 0, 5LL, "3,0.,5"}); }),
	RUN("parsing array: array1 w extra spaces", {
		return runx<array1 COMMA master1>(env,
		CSTR("[\t-1 \n,\t 0.1\n,    777\n,\n\"0, 0.1, 777\"]"),
		master1 {0, 0.1, 777LL, "0, 0.1, 777"}, error_t::mismatch); }),
	RUN("parsing array: array1 w extra elements", {
		return runx<array1 COMMA master1>(env,
		CSTR("[1, 2, 3,\"4\", 5]"),
		master1 {1, 2, 3LL, "4"}, error_t::overrun); }),
	RUN("parsing array: array1 w null", {
		return runx<array1 COMMA master1>(env,
		CSTR("[1, 2, 3, null]"),
		master1 {1, 2, 3, ""}); }),
	RUN("parsing array: array1 w mismatching elements", {
		return runx<array1 COMMA master1>(env,
		  CSTR("[1, 2, \"3\",\"4\"]"),
		master1 {1, 2, 0, "4"}, error_t::mismatch); }),
	RUN("parsing array: array1 w extra spaces", {
		return runx<array1 COMMA master1>(env,
		CSTR("[\t-1 \n,\t 0.1\n,    777\n,\n\"0, 0.1, 777\"]"),
		master1 {0, 0.1, 777LL, "0, 0.1, 777"}, error_t::mismatch); }),
	RUN("parsing array: array2 of various ", {
		return runx<array2 COMMA master2>(env,
		CSTR("[[1,2,3,4], \"[1,2,3,4], 3\", 3]"),
		master2 {{1,2,3,4}, "[1,2,3,4], 3", 3 }); }),
	RUN("parsing array: array2 missing depth ", {
		return runx<array2 COMMA master2>(env,
		CSTR("[1,2,3,4, \"[1,2,3,4], 3\", 3]"),
		master2 {{0,0,0,0}, "", 3 }, error_t::mismatch | error_t::overrun); }),
	RUN("parsing array: array2 extra nested elements ", {
		return runx<array2 COMMA master2>(env,
		CSTR("[[1,2,3,4,5,6], \"[1,2,3,4], 7\", 7]"),
		master2 {{1,2,3,4}, "[1,2,3,4], 7", 7 }, error_t::overrun); }),
	RUN("parsing array: array2 empty nested array", {
		return runx<array2 COMMA master2>(env,
		CSTR("[[], \"[0,0,0,0], 1\", 1]"),
		master2 {{0,0,0,0}, "[0,0,0,0], 1", 1 }); }),
	RUN("parsing array: array1 w excessive elements", {
		return runx<array1 COMMA master1>(env,
		CSTR("[10, 20, 30, \"ok\"] [1,2,3]"),
		master1 {10, 20, 30, "ok"}); }),
};


