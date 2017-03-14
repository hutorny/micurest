/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 071.cpp - cojson tests, read/write with char16_t
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
#include "host-env.hpp"
#include <uchar.h>

#define WNAME(s) static inline constexpr const char_t* s() noexcept {return u"" #s;}

WNAME(c)
WNAME(i)
WNAME(l)
WNAME(u)
WNAME(s)


namespace cojson {
namespace test {
template<>
bool HostEnvironment::put<char16_t>(char_t chr, FILE* out) const noexcept {
	/* combination of char16_t char_t used to catch misconfiguration for
	 * this test															 */
	char mb[MB_CUR_MAX];
	mbstate_t ps {0, 0};
	int n = c16rtomb(mb, chr, &ps);
	msg(verbosity::debug,"c16rtomb -> %d '%s'", n, mb);
	if( n < 0 ) {
		msg(verbosity::silent,"ERR:\tinvalid character %X\n", chr);
		return false;
	}
	for(int i = 0; i < n; ++i )
		if( escape(mb[i], out) ) return false;
	return true;
}
template<typename T, unsigned N>	T arr<T,N>::data[N];
}

using namespace details;

namespace wchar_test {
using namespace test;

static struct Pod {
	char c;
	int  i;
	long l;
	unsigned long long u;
	char_t s[8];
	const char_t * get() const noexcept {
		return s;
	}
	static Pod * instance() noexcept;
	typedef accessor::function<Pod,Pod::instance> X;
} pod = { 126, -65536, 9999999L, 2147483648ULL, u"\"bcde\n\r"};

Pod* Pod::instance() noexcept { return &pod; }

static const value& podo() {
	return cojson::V<Pod::X, O<Pod,
			P<Pod, c, decltype(Pod::c), &Pod::c>,
			P<Pod, i, decltype(Pod::i), &Pod::i>,
			P<Pod, l, decltype(Pod::l), &Pod::l>,
			P<Pod, u, decltype(Pod::u), &Pod::u>,
			P<Pod, s, countof(&Pod::s), &Pod::s>
		>
	>();
}

static class Edo {
	char c;
	short  i;
	long l;
	long long u;
	char_t s[16];
public:
	Edo(int _c, short _i, long _l, long long _u) :
		c(_c), i(_i), l(_l), u(_u) {
		sets(u"solidus \\\\\\\\\\\\\\");
	}
	char get_c() const noexcept { return c; }
	inline void set_c(char v) noexcept  { c = v; }
	inline short get_i() const noexcept  { return i; }
	inline void set_i(short v) noexcept  { i = v; }
	inline long get_l() const noexcept  { return l; }
	inline void set_l(long v) noexcept  { l = v; }
	inline long long get_u() const noexcept { return u; }
	inline void set_u(long long v) noexcept { u = v; }
	inline const char_t* get_s() const noexcept  { return s; }
	template<unsigned N = sizeof(s)>
	inline void sets(const char_t (&v)[N]) noexcept  {
		for(unsigned i = 0; i < sizeof(s) && i < N; ++i) s[i] = v[i];
	}
	static Edo * instance() noexcept;
	typedef accessor::function<Edo,Edo::instance> X;
	static const value& edos() noexcept {
		return cojson::V<Edo::X, O<Edo,
			Q<Edo, ::c, char, &Edo::get_c, &Edo::set_c>,
			Q<Edo, ::i, short, &Edo::get_i, &Edo::set_i>,
			Q<Edo, ::l, long, &Edo::get_l, &Edo::set_l>,
			Q<Edo, ::u, long long,  &Edo::get_u, &Edo::set_u>,
			P<Edo, ::s, countof(&Edo::s), &Edo::s>
		>>();
	}
	static const clas<Edo>& edod() noexcept {
		return O<Edo,
			Q<Edo, ::c, char, &Edo::get_c, &Edo::set_c>,
			Q<Edo, ::i, short,  &Edo::get_i, &Edo::set_i>,
			Q<Edo, ::l, long, &Edo::get_l, &Edo::set_l>,
			Q<Edo, ::u, long long,  &Edo::get_u, &Edo::set_u>,
			P<Edo, ::s, countof(&Edo::s), &Edo::s>
		>();
	}
} edo(-127, (short)0x8888, 0x79999999L, 0x8765436112345678LL);

Edo* Edo::instance() noexcept { return &edo; }

static const value& edox() {
	return cojson::V<Edo::X, O<Edo,
		Q<Edo, i, short, &Edo::get_i>,
		Q<Edo, l, long, &Edo::set_l>
	>>();
}

typedef named_static<double,71> D0;
D0 d0(u"D0",std::numeric_limits<double>::min());
typedef named_static<float, 71> F0;
F0 f0(u"F0",std::numeric_limits<float>::min());

static const value& byref() {
	return
	V<
		M<D0::name, D0::type, D0::ref>,
		M<F0::name, F0::type, F0::ref>
	>();
}


struct Test071 : Test {
	static Test071 tests[];
	inline Test071(tstring name, tstring desc, runner func)
	  : Test(name, desc,func) {
		static register_specifier<char_t> r;
	}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, cojson::error_t::noerror,	env.output.error());
}
#define COMMA ,
#define RUN(name, body) Test071(__FILE__,name, \
		[](const Environment& env) noexcept -> result_t body)
Test071 Test071::tests[] = {
	RUN("POD object", {
		return _R(podo().write(env.output),env);		}),
	RUN("POD static", {
		return _R(Edo::edos().write(env.output),env);	}),
	RUN("POD dynamic", {
		Edo e(1,2, 3, 4);
		e.sets(u"dynamic");
		return _R(Edo::edod().write(e,env.output),env);	}),
	RUN("properties with only setter or getter", {
		return _R(edox().write(env.output),env);		}),
	RUN("parsing string: plain to w/o string", {
		return sstr<70>::run(env, u"\"qwerty\"", u"qwerty");					}),
	RUN("parsing string: escaped to a r/w string", {
		return sstr<70>::run(env, u"\"quotes: \\\"\"", u"quotes: \"");			}),
	RUN("parsing string: leading spaces", {
		return sstr<70>::run(env, u"\n\t\t  \"NORMAL\"", u"NORMAL"); 			}),
	RUN("parsing string: common escapes", {
		return sstr<70>::run(env, u"\"\\b\\f\\n\\r\\tFIVE\"", u"\b\f\n\r\tFIVE"); }),
	RUN("parsing string: hex codes", {
		return sstr<70>::run(env, u"\"\\u0040\\u004F\\u00A2\\u00C3\"",
				u"\x40\x4F\xA2\xC3"); }),
	RUN("parsing string: UTF-8", {
		return sstr<70>::run(env, u"\"Йцуке Ğä Ψσμα\"", u"Йцуке Ğä Ψσμα"); }),
	RUN("parsing string: string + trailing spaces", {
		return sstr<70>::run(env, u"\"qwerty\" \n", u"qwerty");					}),
	RUN("parsing array: short[6]", {
		return arr<short COMMA 6>::run(env,u"[ 1, 2, 3, 4, 5, 6]",
				{1, 2, 3, 4, 5, 6}); }),
	RUN("double/float by ref", {
		return _R(byref().write(env.output), env);	}),
};
#undef  _T_
#define _T_ (7100)
static cstring const Master[std::extent<decltype(Test071::tests)>::value] = {
	_P_(0), _P_(1), _P_(2), _P_(3), _P_(4), _P_(5), _P_(6),
	_P_(7), _P_(8), _P_(9), _P_(10), _P_(11), _P_(12)
};

#include "071.inc"

cstring Test071::master() const noexcept {
	return Master[index()];
}
}}


