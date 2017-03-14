/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 003.cpp - cojson tests, writing POD objects
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

static struct Pod {
	signed char c;
	short  i;
	long l;
	unsigned long long u;
	char s[8];
	const char * get() const noexcept {
		return s;
	}
	static Pod * instance() noexcept;
	typedef accessor::function<Pod,Pod::instance> X;
} pod = { 126, -25536, 9999999L, 2147483648ULL, "\"bcde\n\r"};

Pod* Pod::instance() noexcept { return &pod; }

NAME(c)
NAME(i)
NAME(l)
NAME(u)
NAME(s)

static const value& podo() {
	return V<Pod::X, O<Pod,
			P<Pod, c, decltype(Pod::c), &Pod::c>,
			P<Pod, i, decltype(Pod::i), &Pod::i>,
			P<Pod, l, decltype(Pod::l), &Pod::l>,
			P<Pod, u, decltype(Pod::u), &Pod::u>,
			P<Pod, s, sizeof(Pod::s),   &Pod::s>
		>
	>();
}

template<class X, const clas<typename X::clas>& (*S)() noexcept>
const value& V1() noexcept {
	static const objects<X,S> l;
	return l;
}


static class Edo {
	signed char c;
	short  i;
	long l;
	long long u;
	char s[16];
public:
	Edo(int _c, short _i, long _l, long long _u) :
		c(_c), i(_i), l(_l), u(_u) {
		sets("solidus \\\\\\\\\\\\\\");
	}
	signed char get_c() const noexcept { return c; }
	inline void set_c(signed char v) noexcept  { c = v; }
	inline short get_i() const noexcept  { return i; }
	inline void set_i(short v) noexcept  { i = v; }
	inline long get_l() const noexcept  { return l; }
	inline void set_l(long v) noexcept  { l = v; }
	inline long long get_u() const noexcept { return u; }
	inline void set_u(long long v) noexcept { u = v; }
	inline const char* get_s() const noexcept  { return s; }
	template<unsigned N = sizeof(s)>
	inline void sets(const char (&v)[N]) noexcept  {
		for(unsigned i = 0; i < sizeof(s) && i < N; ++i) s[i] = v[i];
	}
	static Edo * instance() noexcept;
	typedef accessor::function<Edo,Edo::instance> X;
	static const value& edos() noexcept {
		return V<Edo::X, O<Edo,
			Q<Edo, ::c, signed char, &Edo::get_c, &Edo::set_c>,
			Q<Edo, ::i, short, &Edo::get_i, &Edo::set_i>,
			Q<Edo, ::l, long, &Edo::get_l, &Edo::set_l>,
			Q<Edo, ::u, long long,  &Edo::get_u, &Edo::set_u>,
			P<Edo, ::s, countof(&Edo::s), &Edo::s>
		>>();
	}
	static const clas<Edo>& edod() noexcept {
		return O<Edo,
			Q<Edo, ::c, signed char, &Edo::get_c, &Edo::set_c>,
			Q<Edo, ::i, short,  &Edo::get_i, &Edo::set_i>,
			Q<Edo, ::l, long, &Edo::get_l, &Edo::set_l>,
			Q<Edo, ::u, long long,  &Edo::get_u, &Edo::set_u>,
			P<Edo, ::s, countof(&Edo::s), &Edo::s>
		>();
	}
} edo(-127, (short)0x7888, 0x79999999L, 0x8765436112345678LL);

Edo* Edo::instance() noexcept { return &edo; }


static const value& edox() {
	return V<Edo::X, O<Edo,
		Q<Edo, i, short, &Edo::get_i>,
		Q<Edo, l, long, &Edo::set_l>
	>>();
}

struct Test003 : Test {
	static Test003 tests[];
	inline Test003(cstring name, cstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}

#define RUN(name, body) Test003(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test003 Test003::tests[] = {
	RUN("POD object", {
		return _R(podo().write(env.output),env);		}),
	RUN("POD static", {
		return _R(Edo::edos().write(env.output),env);	}),
	RUN("POD dynamic", {
		Edo e(1,2, 3, 4);
		e.sets("dynamic");
		return _R(Edo::edod().write(e,env.output),env);	}),
	RUN("properties with only setter or getter", {
		return _R(edox().write(env.output),env);		}),
};

#undef _T_
#define _T_ (300)


static cstring const Master[details::countof(Test003::tests)] = {
	 _P_(0), _P_(1), _P_(2), _P_(3)
};

#include "003.inc"

cstring Test003::master() const noexcept {
	return Master[index()];
}


