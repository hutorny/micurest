/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 001.cpp - cojson tests, writing integer limits
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

template<typename T>
static const value& structure() {
	typedef integer_limits<T> L;
	return
	V<
		M<L::min, T, L::minr>,
		M<L::pot, T, L::potp>,
		M<L::max, T, L::maxg>
	>();
}

struct Test001 : Test {
	static Test001 tests[];
	inline Test001(cstring name, cstring desc, runner func)
		noexcept : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror, env.error());
}

#define RUN(name, body) Test001(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test001 Test001::tests[] = {
	RUN("integer limits: char", {
		return _R(structure<signed char>().write(env.output),env);				}),
	RUN("integer limits: unsigned char", {
		return _R(structure<unsigned char>().write(env.output),env);	}),
	RUN("integer limits: short", {
		return _R(structure<short>().write(env.output),env);			}),
	RUN("integer limits: unsigned short", {
		return _R(structure<unsigned short>().write(env.output),env);	}),
	RUN("integer limits: long", {
		return _R(structure<long>().write(env.output),env);				}),
	RUN("integer limits: unsigned long", {
		return _R(structure<unsigned long>().write(env.output),env);	}),
	RUN("integer limits: long long", {
		return _R(structure<long long>().write(env.output),env);		}),
	RUN("integer limits: unsigned long long", {
		return _R(structure<unsigned long long>().write(env.output),env);}),
	RUN("overrun with local", {
			env.setbuffsize(32);
			bool pass = ! structure<long>().write(env.output);
			error_t errors = Test::expected(env.error(),error_t::eof);
			return combine2(pass, errors == error_t::noerror,
					error_t::noerror, errors);							}),

};
#undef _T_
#define _T_ (100)

static cstring const Master[details::countof(Test001::tests)] = {
	 _P_(0), _P_(1), _P_(2), _P_(3), _P_(4), _P_(5), _P_(6), _P_(7), _P_(8)
};

#include "001.inc"

cstring Test001::master() const noexcept {
	return Master[index()];
}


