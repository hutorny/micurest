/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 101.cpp - cojson tests, writing double values
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

named_static<double,100> d0(CSTR("D0"),12345678.91);
named_static<double,101> d1(CSTR("D1"),0.00003);
named_static<float,100> f0(CSTR("F0"),976.54321);
named_static<float,101> f1(CSTR("F1"),0.0004);

typedef decltype(d0) D0;
typedef decltype(d1) D1;
typedef decltype(f0) F0;
typedef decltype(f1) F1;


static const value& byref() {
	return
	V<
		M<D0::name, D0::type, D0::ref>,
		M<D1::name, D1::type, D1::ref>,
		M<F0::name, F0::type, F0::ref>,
		M<F1::name, F1::type, F1::ref>
	>();
}

static const value& byptr() {
	return
	V<
		M<D0::name, D0::type, D0::ptr>,
		M<D1::name, D1::type, D1::ptr>,
		M<F0::name, F0::type, F0::ptr>,
		M<F1::name, F1::type, F1::ptr>
	>();
}

static const value& byget() {
	return
	V<
		M<D0::name, D0::type, D0::get>,
		M<D1::name, D1::type, D1::get, D1::set>,
		M<F0::name, F0::type, F0::get>,
		M<F1::name, F1::type, F1::get, F1::set>
	>();
}


struct Test101 : Test {
	static Test101 tests[];
	inline Test101(cstring name, cstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};

static inline void setminmax() noexcept {
	d0 = std::numeric_limits<double>::min();
	d1 = std::numeric_limits<double>::max();
	f0 = std::numeric_limits<float>::min();
	f1 = std::numeric_limits<float>::max();
}

static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}

#define RUN(name, body) Test101(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test101 Test101::tests[] = {
	RUN("double/float by ref", {
//dbg("In Test101\n");
		return _R(byref().write(env.output), env);	}),
	RUN("double/float by ptr", {
		return _R(byptr().write(env.output), env);	}),
	RUN("double/float by get", {
		return _R(byget().write(env.output), env);	}),
	RUN("double/float limits", {
		setminmax();
		return _R(byref().write(env.output), env);	}),
	RUN("overrun with sprintf", {
		setminmax();
		env.setbuffsize(26);
		bool pass = byref().write(env.output);
		error_t errors = Test::expected(env.error(),error_t::eof);
		return combine2(!pass, errors==error_t::noerror,
				error_t::noerror, errors);
	}),
};
#undef  _T_
#define _T_ (10100)
static cstring const Master[details::countof(Test101::tests)] = {
	_P_(0), _P_(1), _P_(2), _P_(3), _P_(4)
};
#if defined __AVR__
#	include "101avr.inc"
#elif defined TEST_WITH_SPRINTF
#	include "101f.inc"
#else
#	include "101.inc"
#endif
cstring Test101::master() const noexcept {
	return Master[index()];
}


