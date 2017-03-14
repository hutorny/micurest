/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 036.cpp - cojson tests, reading POD objects
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
#include <string.h>

NAME(c)
NAME(i)
NAME(l)
NAME(u)
NAME(s)

static const value& podo() noexcept;
static const value& edos() noexcept;

static struct Pod36 {
	char c;
	int  i;
	long l;
	unsigned long long u;
	char s[8];
	const char * get() const noexcept {
		return s;
	}
	int set(int i, char c) noexcept {
		if( i < 0 ) s[0] = 0;
		if( i >= (int)sizeof(s) ) return -1;
		s[i] = c;
		return 0;
	}
	static Pod36 * instance() noexcept;
	typedef accessor::function<Pod36,Pod36::instance> X;

	inline bool match() const noexcept {
		Pod36* that(instance());
		return
			that->c == c &&
			that->i == i &&
			that->l == l &&
			that->u == u &&
			memcmp(s, that->s, sizeof(s)) == 0;
	}
	static inline void clear() noexcept {
		memset(instance(), 0, sizeof(*instance()));
	}
	inline bool run(const Environment& env) const noexcept {
		bool r = match();
		podo().write(env.output);
		clear();
		return r;
	}
} pod = { 0, 0, 0L, 0ULL, {0,0,0,0,0,0,0,0} };

Pod36* Pod36::instance() noexcept { return &pod; }

static const value& podo() noexcept {
	return V<Pod36::X, O<Pod36,
		P<Pod36, c, decltype(Pod36::c), &Pod36::c>,
		P<Pod36, i, decltype(Pod36::i), &Pod36::i>,
		P<Pod36, l, decltype(Pod36::l), &Pod36::l>,
		P<Pod36, u, decltype(Pod36::u), &Pod36::u>,
		P<Pod36, s, sizeof(Pod36::s), &Pod36::s>
	>>();
}

struct Edo36 {
	char c;
	short  i;
	long l;
	long long u;
	char s[16];
	char get_c() const noexcept { return c; }
	inline void set_c(char v) noexcept { c = v; }
	inline short get_i() const noexcept { return i; }
	inline void set_i(short v) noexcept { i = v; }
	inline long get_l() const noexcept { return l; }
	inline void set_l(long v) noexcept { l = v; }
	inline long long get_u() const noexcept { return u; }
	inline void set_u(long long v) noexcept { u = v; }
	inline const char* get_s() const noexcept { return s; }
	inline char * ptr_s() noexcept {
		return s;
	}

	inline bool match(const Edo36* that) const noexcept {
		return
			that->c == c &&
			that->i == i &&
			that->l == l &&
			that->u == u &&
			memcmp(s, that->s, sizeof(s)) == 0;
	}
	static inline void clear() noexcept {
		memset(instance(), 0, sizeof(*instance()));
	}
	bool run(const Environment& env, const Edo36* that = nullptr) const noexcept;
	static Edo36 * instance() noexcept;
	typedef accessor::function<Edo36,Edo36::instance> X;
} edo;

Edo36* Edo36::instance() noexcept { return &edo; }

struct Xdo36 {
	char s[40];
	Edo36 edo;
	inline bool match(const Xdo36* that) const noexcept {
		return
			memcmp(s, that->s, sizeof(s)) == 0 &&
			edo.match(&that->edo);
	}
	bool run(const Environment& env, const Xdo36* that) const noexcept;
};

static const value& edos() noexcept {
	return V<Edo36::X, O<Edo36,
		Q<Edo36, c, char, &Edo36::get_c, &Edo36::set_c>,
		Q<Edo36, i, short, &Edo36::get_i, &Edo36::set_i>,
		Q<Edo36, l, long, &Edo36::get_l, &Edo36::set_l>,
		Q<Edo36, u, long long,  &Edo36::get_u, &Edo36::set_u>,
		P<Edo36, s, sizeof(Edo36::s), &Edo36::s>
	>>();
}

static const clas<Edo36>& edod() noexcept {
	return O<Edo36,
		Q<Edo36, c, char, &Edo36::get_c, &Edo36::set_c>,
		Q<Edo36, i, short,  &Edo36::get_i, &Edo36::set_i>,
		Q<Edo36, l, long, &Edo36::get_l, &Edo36::set_l>,
		Q<Edo36, u, long long,  &Edo36::get_u, &Edo36::set_u>,
		P<Edo36, s, sizeof(Edo36::s), &Edo36::s>
	>();
}

static const clas<Edo36>& edox() noexcept {
	return O<Edo36,
		Q<Edo36, i, short,  &Edo36::get_i, &Edo36::set_i>,
		Q<Edo36, l, long, &Edo36::get_l, &Edo36::set_l>,
		P<Edo36, s, sizeof(Edo36::s), &Edo36::s>
	>();
}

static const clas<Xdo36>& xdo() noexcept {
	return O<Xdo36,
		P<Xdo36, s, sizeof(Xdo36::s), &Xdo36::s>,
		P<Xdo36, c, Edo36, &Xdo36::edo, edod>
	>();
}


bool Edo36::run(const Environment& env, const Edo36* that) const noexcept {
	bool r = match(that == nullptr ? instance() : that);
	if( that )
		edod().write(*that, env.output);
	else
		edos().write(env.output);
	if( ! that )
		clear();
	return r;
}

bool Xdo36::run(const Environment& env, const Xdo36* that) const noexcept {
	xdo().write(*that, env.output);
	return match(that);

}


struct Test036 : Test {
	static Test036 tests[];
	inline Test036(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
};



#define COMMA ,
#define RUN(name, body) Test036(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)

Test036 Test036::tests[] = {
	RUN("reading POD object", {
		return  runx<podo COMMA Pod36>(env,
		CSTR("{\"c\": 1, \"i\":2, \"l\":3, \"u\":4, \"s\":\"1234567\"}"),
			Pod36 {1,2,3,4, "1234567"});								}),
	RUN("POD static", {
		return  runx<edos COMMA Edo36>(env,
		CSTR("{\"c\":10,\"i\":20,\"l\":30,\"u\":40,\"s\":\"10203040506070\"}"),
			Edo36 {10,20,30,40, "10203040506070\0"});						}),
	RUN("POD dynamic", {
		return runx<Edo36 COMMA edod>(env,
		CSTR("{\"c\":100,\"i\":200,\"l\":300,\"u\":400,\"s\":\"100200300400500\"}"),
			Edo36 { 100,200,300L,400LL, "100200300400500" },
			Edo36 { 0,0,0,0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"});			}),
	RUN("POD dynamic with string field", {
		return runx<Edo36 COMMA edox>(env,
		CSTR("{\"i\":210,\"l\":310,\"s\":\"000210310000500\"}"),
			Edo36 { 0,210,310L,0 , "000210310000500" },
			Edo36 { 0,0,0,0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"});			}),
	RUN("Nested PDO", {
		return runx<Xdo36 COMMA xdo>(env,
		CSTR("{\"c\":{\"i\":230,\"l\":330,\"s\":\"nested\"}, \"s\":\"parent\"}"),
			Xdo36 { "parent", {0,230,330L,0 , "nested"} },
			Xdo36 { "\0", {0,0,0,0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"}});	}),
	RUN("POD with excessive members", {
		return runx<Edo36 COMMA edox>(env,
		CSTR("{\"c\":120,\"i\":220,\"l\":320,\"u\":420,\"s\":\"000220320000520\"}"),
			Edo36 { 0,220,320L, 0, "000220320000520" },
			Edo36 { 0,0,0,0, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"});			}),
};
