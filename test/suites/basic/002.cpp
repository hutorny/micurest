/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 002.cpp - cojson tests, writing arrays and objects
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

static unsigned* uitem(cojson::size_t n) noexcept {
	static unsigned items[] = { 123, 456, 789, 1000, 10000 };
	return n < countof(items)? items + n : nullptr;
}

static const value& array1() noexcept {
	return V<unsigned, uitem>();
}

static char str[] = "string\tvalue";

static const char* str_get() noexcept {
	return str;
}

static char* str_ptr() noexcept {
	return str;
}
template<typename T = cstring>
struct names;

template<>
struct names<const char*> {
	static const char* strname() noexcept {
		return "string";
	}
	static const char* a() {
		return "a";
	}

	static const char* b() {
		return "b";
	}
};

#if __AVR__
template<>
struct names<progmem<char>> {
	static progmem<char> strname() noexcept {
		static const char s[] __attribute__((progmem)) = "string";
		return progmem<char>(s);
	}
	static progmem<char> a() {
		static const char s[] __attribute__((progmem)) = "a";
		return progmem<char>(s);
	}

	static progmem<char> b() {
		static const char s[] __attribute__((progmem)) = "b";
		return progmem<char>(s);
	}
};
#endif

static const value& strings() {
	return V<str_get>();
}

static const value& simple() {
	return V<
		M<names<>::strname, sizeof(str), str_ptr>
	>();
}


static const value& array2() {
	return V<
		V<unsigned, uitem>,
		V<sizeof(str), str_ptr>,
		V<M<names<>::strname, V<sizeof(str), str_ptr>>>
	>();
}

/* read-only string */
template<cojson::size_t N, char_t* (*F)() noexcept>
const value& RO() noexcept {
	static const struct local : string {
		inline local(char_t* s, cojson::size_t length) noexcept
		  : string(s, length) {}
		bool write(ostream& out) const noexcept {
			return value::null(out);
		}
	} l(F(),N);
	return l;
}


static const value& array3() {
	return V<
		V<unsigned, uitem>,
		RO<sizeof(str), str_ptr>, //str_set
		V<M<names<>::strname, RO<sizeof(str), str_ptr>>> //str_set
	>();
}


static const value& complex() {
	return V<
		M<names<>::a,V<unsigned, uitem>>,
		M<names<>::b,str_get>,
		M<names<>::strname, V<M<names<>::a,str_get>>>
	>();
}


struct Test002 : Test {
	static Test002 tests[];
	inline Test002(cstring name, cstring desc, runner func)
		noexcept : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}

#define RUN(name, body) Test002(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test002 Test002::tests[] = {
	RUN("array of unsigned", {
		return _R(array1().write(env.output),env);		}),
	RUN("array of various", {
		return _R(array2().write(env.output),env);		}),
	RUN("strings", {
		return _R(::strings().write(env.output),env);		}),
	RUN("simple object", {
		return _R(simple().write(env.output),env);		}),
	RUN("complex object", {
		return _R(complex().write(env.output),env);		}),
	RUN("overrun on object", {
		env.setbuffsize(32);
		bool pass =! complex().write(env.output);
		error_t errors = Test::expected(env.output.error(), error_t::eof);
		return combine2(pass, errors == error_t::noerror,
				error_t::noerror, errors);				}),
	RUN("array with some members missing", {
		return _R(array3().write(env.output),env);		}),
};

#undef _T_
#define _T_ (200)

static cstring const Master[details::countof(Test002::tests)] = {
	 _P_(0), _P_(1), _P_(2), _P_(3), _P_(4), _P_(5), _P_(6)
};

#include "002.inc"

cstring Test002::master() const noexcept {
	return Master[index()];
}


