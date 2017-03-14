/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 100.cpp - test for floating converstion
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
#include <stdlib.h>
#include "test.hpp"
#include "cojson_float.hpp"
namespace cojson {
namespace test {

static constexpr const bool low_end = sizeof(double) < 8;

struct Test100 : Test {
	static Test100 tests[];
	inline Test100(tstring name, tstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

using namespace floating;
using namespace details;

static result_t test_conversion(const Environment& env, double val,
		int precision) noexcept {
	char buff[32];
	buffer out(buff);
	//out.restart();
	serialize(val, out, precision);
	out.put(0);
	double check = atof(out.begin());
	if( fabs(val - check)/fabs(val + check) > 5*exp_10<double>(-precision) ) {
		env.msg(LVL::normal,
			"failed with p = %d: %.16g != %.16g \"%.*g\"!=\"%s\"\n",
			precision, val, check, precision, val, out.begin());
		return result_t::bad;
	}
	return result_t::success;
}

static result_t test_conversions(const Environment& env, double val) noexcept  {
	for(int i = 0; i <= (low_end?6:8); ++i)
		if( test_conversion(env, val, i) )
			return result_t::bad;
	return result_t::success;
}

#ifndef M_E
# define M_E		2.7182818284590452354	/* e */
#endif
#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif
#pragma GCC diagnostic ignored "-Woverflow"
static constexpr double number(bool positive) noexcept {
	return  low_end ? (positive ? -1e38 : 1e38) : (positive ? -1e150 : 1e150);
}

double starters[] = { number(false), number(true) };
double factors[] = { 9.999, 3.999, M_PI, M_E };

static result_t runtests(const Environment& env) noexcept  {
	result_t result = result_t::success;
	test_conversion(env, -100.984867210912, 4);
	env.msg(LVL::verbose,"Testing %s endian floating::serialize",
			(cojson::floating::target_is_le ? "little" : "big"));
	for(size_t s = 0; s < sizeof(starters)/sizeof(starters[0]); ++s)
		for(size_t f = 0; f < sizeof(factors)/sizeof(factors[0]); ++f) {
			double v = starters[s];
			for(size_t i = 0; i < (low_end?76:300); ++i) {
				if( test_conversions(env, v) ) result = result_t::bad;
				v /= factors[f];
			}
			env.msg(LVL::normal,".");
		}
	env.msg(LVL::normal,"\n");
	return result;
}

#define RUN(name, body) Test100(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)

Test100 Test100::tests[] = {
	Test100(OMIT(__FILE__),OMIT("cojson float conversion"), runtests)
};
}}
