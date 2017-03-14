/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 035.cpp - cojson tests, reading objects
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

static unsigned items[] = { 0, 0, 0, 0, 0 };
static unsigned * uitem(unsigned n) noexcept {
	return n < static_cast<int>((sizeof(items)/sizeof(items[0])))
		? items + n : nullptr;
}

static char str[16] = "";

static char* str_ptr() noexcept {
	return str;
}

static char str2[16] = "";
static char* str2_ptr() noexcept {
	return str2;
}

static int _c = 0;
static int& c_ref() noexcept { return _c; }

NAME(strname)
NAME(a)
NAME(b)
NAME(c)

static const value& simple() noexcept {
	return V<
		M<strname, sizeof(str), str_ptr>
	>();
}

struct simple_master {
	char strname[sizeof(str)];
	inline bool match() const noexcept {
		return strcmp(strname,str) == 0;
	}
	static inline void clear() noexcept {
		memset(str, 0, sizeof(str));
	}
	inline bool run(const Environment& env) const noexcept {
		bool r = match();
		env.out(r, "{\"strname\":\"%s\"}\n", str);
		clear();
		return r;
	}

};

static const value& complex() noexcept {
	return V<
		M<a,V<unsigned, uitem>>,
		M<b,sizeof(str), str_ptr>,
		M<c, int, c_ref>,
		M<strname, V<M<a,sizeof(str2),str2_ptr>>>
	>();
}

struct complex_master {
	unsigned a[sizeof(items)];
	char b[sizeof(str)];
	int c;
	char strname[sizeof(str2)];
	inline bool match() const noexcept {
		return
			memcmp(a, items, sizeof(items)) == 0 &&
			strcmp(b,str) == 0 &&
			c == _c &&
			strcmp(strname, str2) == 0;
	}
	static inline void clear() noexcept {
		_c = 0;
		memset(items, 0, sizeof(items));
		memset(str, 0, sizeof(str));
		memset(str2, 0, sizeof(str2));
	}
	inline bool run(const Environment& env) const noexcept {
		bool r = match();
		env.out(r, "{\"a\" : [%d, %d, %d, %d, %d], \"b\":\"%s\", "
				   "\"c\":%d, \"strname\":{\"a\":\"%s\"}}\n",
			items[0], items[1], items[2], items[3], items[4], str, _c, str2);
		clear();
		return r;
	}

};


struct Test035 : Test {
	static Test035 tests[];
	inline Test035(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
};

#define COMMA ,

#define RUN(name, body) Test035(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test035 Test035::tests[] = {
	RUN("reading simple object", {
		return  runx<simple COMMA simple_master>(env,
				CSTR("{\"strname\":\"1234567\"}"),
				simple_master { "1234567" });
	}),
	RUN("reading complex object", {
		return  runx<complex COMMA complex_master>(env,
		CSTR("{\"strname\": { \"a\" : \"complex \\\"string\" },"
			"\"b\": \"plain \\n string \","
			"\"a\": [1,2,3,4,5],"
			"\"c\": 7 }"),
			complex_master { {1,2,3,4,5}, "plain \n string ",
					7, "complex \"string"  });
	}),
	RUN("reading simple object, extra member", {
		return  runx<simple COMMA simple_master>(env,
				CSTR("{\"a\":100, \"strname\":\"lkjhg\"}"),
				simple_master { "lkjhg" });
	}),
	RUN("reading simple object, extra nested object", {
		return  runx<simple COMMA simple_master>(env,
		CSTR("{\"a\":{\"strname\": \"qwerty\"}, \"strname\":\"lkjhg\"}"),
				simple_master { "lkjhg" });
	}),
	RUN("reading simple object, extra nested object", {
		return  runx<simple COMMA simple_master>(env,
		CSTR("{\"strname\":\"lkjhg\", \"a\":{\"strname\": \"qwerty\"}}"),
				simple_master { "lkjhg" });
	}),
};



