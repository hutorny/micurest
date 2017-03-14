/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 080.cpp - cojson tests, benchmarking
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

#include <stdio.h>
#include <string.h>
#include "bench.hpp"

using namespace cojson;
using namespace test;

struct Config080 : Config {
	inline result_t run(const Environment& env, cstring in) noexcept {
		bool pass = structure().read(*this, test::json(in));
		if( !pass || env.isbenchmark() ) return
				combine1(pass, test::json(cstring(nullptr)).error());
		pass = structure().write(*this, env.output);
		return combine1(pass, test::json().error(), env.error());
	}
	inline void clear() noexcept {
		memset(this, 0, sizeof(*this));
	}
};

static const Config config1 {
	{ "dhcp", {127,0,0,1}, {255,0,0,0}, {10,0,0,1}, 30000L, 100L, "eth0",
		{{8,8,8,8}, {1,2,3,4}, {5,6,7,8}}
	},
	1, 2, 3, 4, 5, 6, 7, 8,
	{}, 9, "Tue Sep  8 06:21:31 2015", 10, { 0.001, 0.01, 0.1 }
};

#ifndef BENCH_DATA_ATTR
#	define BENCH_DATA_ATTR
#endif

static Config080 config2 BENCH_DATA_ATTR;

static nul nil;

static inline ostream& output(const Environment& env) noexcept {
	return env.isbenchmark() ? (ostream&)nil : (ostream&)env.output;
}

static inline result_t _R(bool pass, error_t in, const Environment& env) noexcept {
	return combine1(pass, in, output(env).error());
}

struct Test080 : Test {
	static Test080 tests[];
	inline Test080(cstring name, cstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
	static const char datain[];
	static double data[3];

	static double* dbl3(cojson::size_t i) noexcept {
		return i<3 ? data+i : nullptr;
	}
	static void clear() noexcept {
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
	}

	static inline result_t run2(const Environment& env, cstring in) noexcept {
		clear();
		bool pass =
			V<double, dbl3>().read(test::json(in));
		pass = V<double, dbl3>().write(output(env));
		return combine1(pass, test::json().error(), output(env).error());
	}

};

double Test080::data[3] = {0,0,0};

#define COMMA ,
#define RUN(name, body) Test080(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)

Test080 Test080::tests[] = {
	RUN("benchmarking: reading/writing array[3] of double", {
		return Test080::run2(env,
			CSTR("[0.0029296875, 0.0146484375, 0.04541015625]"));
	}),
	RUN("benchmarking: reading single nested property", {
			config2.clear();
			bool pass = Config::structure().read(config2,
					test::json(CSTR("{\"wan\"\t:\t{ \"expires\": 0 }}")));
			return _R(pass, test::json().error(), env);
		}),
	RUN("benchmarking: reading/writing custom type", {
		config2.clear();
		bool pass = Config::Wan::structure().read(
			config2.wan, test::json(CSTR("{ \"ipaddr\": [192,168,159,47] }")));
		pass = pass && Config::Wan::structure().write(config2.wan, output(env));
		return _R(pass, test::json().error(), env);
	}),
	RUN("benchmarking: writing Config", {
		bool pass = Config::structure().write(config1, output(env));
		return _R(pass, error_t::noerror, env) ; }),
	RUN("benchmarking: reading Config", {
		config2.clear();
		return config2.run(env, cstring(Test080::datain));
	}),
};

reader<ip4_t> reader<ip4_t>::unit __attribute__((weak));

#undef  _T_
#define _T_ (8000)
static cstring const Master[std::extent<decltype(Test080::tests)>::value] = {
	_P_(0), cstring(nullptr), _P_(2), _P_(3), _P_(4)
};

#ifdef __AVR__
#	include "080.avr.inc"
#else
#	include "080.inc"
#endif
cstring Test080::master() const noexcept {
	return Master[index()];
}
#ifndef CSTRING_PROGMEM
const char Test080::datain[] = {
#else
const char Test080::datain[] __attribute__((progmem))= {
#endif
/* cat 080.json | file2c > 080.in.inc */
#include "080.in.inc"
,0};
