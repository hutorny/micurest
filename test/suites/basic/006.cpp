/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 006.cpp - cojson tests, writing with Write
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

namespace name {
	NAME(c)
	NAME(i)
	NAME(l)
	NAME(u)
	NAME(s)
}
namespace Test006 {
static struct Pod {
	signed char c;
	short  i;
	long l;
	unsigned long long u;
//	char s[8];
//	const char * get() const noexcept {
//		return s;
//	}
	typedef ObjectJson<Pod,
			decltype(c),
			decltype(i),
			decltype(l),
			decltype(u)>::
		PropertyNames<
			name::c,
			name::i,
			name::l,
			name::u>::
		FieldPointers<
			&Pod::c,
			&Pod::i,
			&Pod::l,
			&Pod::u> Object;
	bool read(lexer& in) noexcept {
		return Object::json().read(*this, in);
	}
	bool write(ostream& out) const noexcept {
		return Object::json().write(*this, out);
	}
} pod = { 126, -25536, 9999999L, 2147483648ULL };

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
	bool read(lexer& in) noexcept {
		return json().read(*this, in);
	}
	bool write(ostream& out) const noexcept {
		return json().write(*this, out);
	}

	static const clas<Edo>& json() noexcept {
		return O<Edo,
			Q<Edo, name::c, decltype(c), &Edo::get_c, &Edo::set_c>,
			Q<Edo, name::i, decltype(i), &Edo::get_i, &Edo::set_i>,
			Q<Edo, name::l, decltype(l), &Edo::get_l, &Edo::set_l>,
			Q<Edo, name::u, decltype(u),  &Edo::get_u, &Edo::set_u>,
			P<Edo, name::s, countof(&Edo::s), &Edo::s>
		>();
	}
} edo(-127, (short)0x7888, 0x79999999L, 0x8765436112345678LL);


struct Test006 : Test {
	static Test006 tests[];
	inline Test006(cstring name, cstring desc, runner func)
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}

void t(ostream& out) {

	Write(edo, out);
}

#define PDO_INIT1 { 126, -25536, 9999999L, 2147483648ULL }

#define RUN(name, body) Test006(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test006 Test006::tests[] = {
	RUN("pod.write", {
		return _R(pod.write(env.output),env);								}),
	RUN("edo.write", {
		return _R(edo.write(env.output),env);								}),
	RUN("Write(pod)", {
		return _R(Write(pod,env.output),env);								}),
	RUN("Write(edo)", {
		Edo e(1,2, 3, 4);
		e.sets("d\004namic");
		return _R(Write(e,env.output),env);									}),
	RUN("Write(long)", {
		long l = 47653L;
		return _R(Write(l,env.output),env);								}),
	RUN("Write(char*)", {
		char s[] = "char*";
		char* l = s;
		return _R(Write(l,env.output),env);									}),
	RUN("Write(const char*)", {
		const char* l = "const char*";
		return _R(Write(l,env.output),env);									}),
	RUN("Write(char[])", {
		char l[] = "char[]";
		return _R(Write(&l[0],env.output),env);									}),
	RUN("pod.read", {
		cstream inp(CSTR("{\"c\": 1, \"i\":2, \"l\":3, \"u\":4, \"s\":\"1234567\"}"));
		lexer in(inp);
		Pod l PDO_INIT1;
		return _R(l.read(in) && l.write(env.output),env);				}),
	RUN("Read(pod)", {
		cstream inp(CSTR("{\"c\":10,\"i\":20,\"l\":30,\"u\":40,\"s\":\"10203040506070\"}"));
		lexer in(inp);
		Pod l PDO_INIT1;
		return _R(Read(l, in) && l.write(env.output),env);				}),
	RUN("edo.read", {
		cstream inp(CSTR("{\"c\":100,\"i\":200,\"l\":300,\"u\":400,\"s\":\"100200300400500\"}"));
		lexer in(inp);
		return _R(edo.read(in) &&  edo.write(env.output),env);				}),
	RUN("Read(edo)", {
		cstream inp(CSTR("{\"i\":210,\"l\":310,\"s\":\"000210310000500\"}"));
		lexer in(inp);
		Edo l(-12, (short)0x8000, 0x80000000L, 0x7865436112345678LL);
		return _R(Read(l, in) &&  l.write(env.output) ,env);				}),
	RUN("Read(unsigned int)", {
		cstream inp(CSTR("21031"));
		lexer in(inp);
		unsigned int l = 0;
		return _R(Read(l,inp)&& Write(l,env.output),env);					}),
};

#undef _T_
#define _T_ (600)


static cstring const Master[details::countof(Test006::tests)] = {
	 _P_(0), _P_(1), _P_(2), _P_(3), _P_(4),
	 _P_(5), _P_(6), _P_(7), _P_(8), _P_(9),
	 _P_(10), _P_(11), _P_(12),
};

#include "006.inc"

cstring Test006::master() const noexcept {
	return Master[index()];
}

}
