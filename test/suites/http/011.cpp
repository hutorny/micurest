/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 010.cpp - µcuREST tests, plain values
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "micurest.hpp"
#include "test.hpp"
using namespace micurest;

static const directory& root() noexcept;

struct Test011 : Test {
	static Test011 tests[];
	inline Test011(tstring name, tstring desc, cstring input) noexcept
		: Test(name, desc), inp(input), expected(application::result_t::close){}
	inline Test011(tstring name, tstring desc, cstring input,
			application::result_t expect) noexcept
		: Test(name, desc), inp(input), expected(expect){}
	int index() const noexcept  {
		return (this-tests);
	}
	result_t run(const Environment& env) const noexcept {
		cstream in(inp);
		application app(root());
		cleanup();
		application::result_t res = app.service(in, env.output);
		return res == expected ? success : (result_t)(bad | (uint8_t)res);
	}
	static void cleanup() noexcept;
	cstring master() const noexcept;
	cstring const inp;
	application::result_t expected;
};

/* Test plan
 * positive tests:
 * 		PUT/GET JSON objects
 */


namespace name {
	NAME(data)
	NAME(obj)
}

struct obj {
	uint32_t data;

	static const clas<obj>& S() noexcept {
		return O<obj,
			P<obj, name::data,  decltype(obj::data),  &obj::data>
		>();
	}
	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}
	static obj instance;
	typedef cojson::accessor::pointer<obj, &instance> X;
};

obj obj::instance;

static const directory& root() noexcept {
	return Root<
		E<name::obj, N<V<obj::X,obj::S>>>,
		E<name::data, resource::NodeJSONRpc<V<obj::X,obj::S>>>
	>();
}

void Test011::cleanup() noexcept {
	obj::instance.data = 0;
}

#define RUN(name, input) Test011(OMIT(__FILE__),OMIT(name), CSTR(input))
#define NEG(name, input, result) Test011(OMIT(__FILE__),OMIT(name), CSTR(input), result)

Test011 Test011::tests[] = {
	RUN("GET /obj",
		"GET /obj\r\n\r\n"),
	RUN("PUT /obj",
		"PUT /obj\r\nContent-Type: application/json\r\n\r\n{\"data\":1}"),
	RUN("PUT /obj w/ws",
		"PUT /obj\r\nContent-Type: application/json\r\n\r\n{ \"data\" : 2 }"),
	RUN("POST /obj",
		"POST /obj\r\nContent-Type: application/json\r\n\r\n{\"data\":3}"),
	RUN("POST /data",
		"POST /data\r\nContent-Type: application/json-rpc\r\n\r\n{\"data\":3}"),
	RUN("POST /obj w/ Accept",
		"POST /obj\r\nAccept: text/html,application/json;q=0.8\r\n"
		"Content-Type: application/json\r\n\r\n{\"data\":4}"),
	RUN("GET /obj w/ Accept",
		"GET /data\r\nAccept: application/json-rpc,text/html\r\n\r\n"),
	RUN("GET /obj w/ Accept",
		"GET /obj\r\nAccept: application/json,text/html\r\n\r\n"),
};

#undef _T_
#define _T_ (1100)

static cstring const Master[countof(Test011::tests)] = {
	 _P_( 0), _P_( 1), _P_( 2), _P_( 3), _P_( 4),
	 _P_( 5), _P_( 6), _P_( 7) //, _P_( 8), _P_( 9),
//	 _P_(10), _P_(11), _P_(12), _P_(13), _P_(14),
};

#include "011.inc"

cstring Test011::master() const noexcept {
	return Master[index()];
}

