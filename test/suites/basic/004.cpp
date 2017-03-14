/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 004.cpp - cojson tests, writing extern C arrays and objects
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
#include "004.h"

static const value& array1() {
	return V<unsigned, uitem>();
}

NAME(a)
NAME(b)
NAME(c)
NAME(i)
NAME(l)
NAME(u)
NAME(s)

static const value& strings() {
	return V<strv>();
}

static const value& simple() {
	return V<
		M<s, strv>
	>();
}


static const value& array2() {
	return V<
		V<unsigned, uitem>,
		V<strv>,
		V<M<s, V<strv>>>
	>();
}

static const value& complex() {
	return V<
		M<a,V<unsigned, uitem>>,
		M<b,strv>,
		M<s, V<M<a, strv>>>
	>();
}

inline const char* get() { return cpod()->s; }
typedef accessor::function<CPod,cpod> X;

static const value& cpodo() {
	return V<X, O<CPod,
		P<CPod, c, decltype(CPod::c), &CPod::c>,
		P<CPod, i, decltype(CPod::i), &CPod::i>,
		P<CPod, l, decltype(CPod::l), &CPod::l>,
		P<CPod, u, decltype(CPod::u), &CPod::u>,
		P<CPod, s, sizeof(CPod::s), &CPod::s>
	>>();
}

static const clas<CPod>& dpodo() {
	return O<CPod,
		P<CPod, c, decltype(CPod::c), &CPod::c>,
		P<CPod, i, decltype(CPod::i), &CPod::i>,
		P<CPod, l, decltype(CPod::l), &CPod::l>,
		P<CPod, u, decltype(CPod::u), &CPod::u>,
		P<CPod, s, sizeof(CPod::s), &CPod::s>
	>();
}

struct Test004 : Test {
	static Test004 tests[];
	inline Test004(cstring name, cstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}


#define RUN(name, body) Test004(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test004 Test004::tests[] = {
	RUN("array of unsigned", {
		return _R(array1().write(env.output), env);		}),
	RUN("array of various", {
		return _R(array2().write(env.output), env);		}),
	RUN("strings", {
		return _R(::strings().write(env.output), env);	}),
	RUN("simple object", {
		return _R(simple().write(env.output), env);		}),
	RUN("complex object", {
		return _R(complex().write(env.output), env);	}),
	RUN("C POD object", {
		return _R(cpodo().write(env.output), env);		}),
	RUN("C POD dynamic", {
		return _R(dpodo().write(dpod, env.output), env);}),

};

#undef _T_
#define _T_ (400)

static cstring const Master[details::countof(Test004::tests)] = {
	 _P_(0), _P_(1), _P_(2), _P_(3), _P_(4), _P_(5), _P_(6)
};

#include "004.inc"

cstring Test004::master() const noexcept {
	return Master[index()];
}


