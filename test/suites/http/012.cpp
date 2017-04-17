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

struct Test012 : Test {
	static Test012 tests[];
	inline Test012(tstring name, tstring desc, cstring input) noexcept
		: Test(name, desc), inp(input), expected(application::result_t::close){}
	inline Test012(tstring name, tstring desc, cstring input,
			int expect) noexcept
		: Test(name, desc), inp(input), expected(application::result_t(expect)){}
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
 * negative tests:
 * 		PUT/GET malformed headers, unsupported commands,
 */


namespace name {
	NAME(data)
	NAME(obj)
}

struct nobj {
	uint32_t data;

	static const clas<nobj>& S() noexcept {
		return O<nobj,
			P<nobj, name::data,  decltype(nobj::data),  &nobj::data>
		>();
	}
	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}
	static nobj instance;
	typedef cojson::accessor::pointer<nobj, &instance> X;
};

nobj nobj::instance;

static const directory& root() noexcept {
	return Root<
		E<name::obj, N<V<nobj::X,nobj::S>>>,
		E<name::data, resource::NodeJSONRpc<V<nobj::X,nobj::S>>>
	>();
}

void Test012::cleanup() noexcept {
	nobj::instance.data = 0;
}

#define RUN(name, input) Test012(OMIT(__FILE__),OMIT(name), CSTR(input))
#define NEG(name, input, result) Test012(OMIT(__FILE__),OMIT(name), CSTR(input), result)

Test012 Test012::tests[] = {
	NEG("OPTIONS",
		"OPTIONS\r\n\r\n", 1),
	NEG("PUT /obj Content+",
		"PUT /obj\r\nContent+Type: application/json\r\n\r\n{\"data\":1}", 1),
	NEG("PUT /notfound",
		"PUT /notfound\r\nContent-Type: application/json\r\n\r\n{\"data\":2}", 1),
	NEG("POST /obj wrong",
		"POST /obj\r\nContent-Type: text/html\r\n\r\n{\"data\":3}", 1),
	RUN("POST /data json+rpc",
		"POST /data\r\nContent-Type: application/json+rpc\r\n\r\n{\"data\":3}" /*FIXME , 1*/),
	NEG("POST /obj w/ Accept",
		"POST /obj\r\nAccept: text/html,application/json;q=0.8\r\n"
		"Content-Type: application/json-rpc\r\n\r\n{\"data\":4}", 1),
	NEG("CONNECT",
		"CONNECT /obj\r\n\r\n", 1),
};

#undef _T_
#define _T_ (1200)

static cstring const Master[countof(Test012::tests)] = {
	 _P_( 0), _P_( 1), _P_( 2), _P_( 3), _P_( 4),
	 _P_( 5), _P_( 6), //_P_( 7), _P_( 8), _P_( 9),
//	 _P_(10), _P_(11), _P_(12), _P_(13), _P_(14),
};

#include "012.inc"

cstring Test012::master() const noexcept {
	return Master[index()];
}

