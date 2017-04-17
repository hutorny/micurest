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
#include "dbg.h"

using namespace micurest;

static const directory& root() noexcept;

struct Test013 : Test {
	static Test013 tests[];
	inline Test013(tstring name, tstring desc, cstring header, cstring input) noexcept
		: Test(name, desc), hdr(header), inp(input), expected(application::result_t::close){}
	inline Test013(tstring name, tstring desc, cstring header, cstring input,
			int expect) noexcept
		: Test(name, desc), hdr(header), inp(input), expected(application::result_t(expect)){}
	int index() const noexcept  {
		return (this-tests);
	}
	result_t run(const Environment& env) const noexcept {
		cstream in(hdr);
		application app(root());
		micurest::details::httpmessage::state_pdo pdo = {};
		cleanup();
		application::result_t res = app.service(in, env.output, &pdo);
		if( res != application::result_t::fragment ) return bad;
		cstream body(inp);
		res = app.service(body, env.output, &pdo);
		return res == expected ? success : (result_t)(bad | (uint8_t)res);
	}
	static void cleanup() noexcept;
	cstring master() const noexcept;
	cstring const hdr;
	cstring const inp;
	application::result_t expected;
};

/* Test plan
 * positive tests:
 * 		PUT/POST fragmented
 * Most of the browsers fragment PUT/POST on header and body parts,
 * These tests verify behavior in such cases
 */

namespace name {
ALIAS(index,index.html)
NAME(natural)
NAME(numeric)
NAME(text)
NAME(logical)
NAME(blob)
NAME(data)
NAME(obj)
NAME(dir)
}

using namespace cojson::accessor;
static unsigned natural;
static float numeric;
static bool logical;
char_t text13[32];
unsigned char blob13[256];
unsigned blob13_length = 0;

static float get_numeric() noexcept { return numeric; }
static void put_numeric(float v) noexcept { numeric = v; }

struct fobj {
	uint32_t data;

	static const clas<fobj>& S() noexcept {
		return O<fobj,
			P<fobj, name::data,  decltype(fobj::data),  &fobj::data>
		>();
	}
	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}
	static fobj instance;
	typedef cojson::accessor::pointer<fobj, &instance> X;
};

fobj fobj::instance;

static const directory& root() noexcept {
	return Root<
		F<name::natural, pointer<unsigned, &natural>>,
		F<name::numeric, pointer<float, &numeric>>,
		F<name::logical, pointer<bool, &logical>>,
		D<name::dir,
			F<name::natural, unsigned, &natural>,
			F<name::numeric, float, get_numeric, put_numeric>,
			F<name::text, countof(text13), text13>
		>,
		F<name::blob,&blob13_length, sizeof(blob13), blob13>,
		E<name::obj, N<V<fobj::X,fobj::S>>>,
		E<name::data, resource::NodeJSONRpc<V<fobj::X,fobj::S>>>
	>();
}

void Test013::cleanup() noexcept {
	fobj::instance.data = 0;
	natural = 0;
	numeric = 0;
	logical = false;
	memset(text13,0,sizeof(text13));
	memset(blob13,0,sizeof(blob13));
	blob13_length = 0;
}

#define RUN(name, header, input) Test013(OMIT(__FILE__),OMIT(name), CSTR(header), CSTR(input))
#define NEG(name, header, input, result) Test013(OMIT(__FILE__),OMIT(name), CSTR(header), CSTR(input), result)

Test013 Test013::tests[] = {
	RUN("PUT /numeric",
		"PUT /numeric\r\n"
		"Content-Type: application/json\r\n\r\n",
		"20\r\n"),
	RUN("PUT /logical",
		"PUT /logical\r\n"
		"Content-Type: application/json\r\n\r\n",
		"true \r\n"),
	RUN("PUT /dir/numeric",
		"PUT /dir/numeric\r\n"
		"Content-Length: 7\r\n"
		"Content-Type: application/json\r\n\r\n",
		"0.65535\r\n"),
	RUN("PUT /dir/natural",
		"PUT /dir/natural\r\n"
		"Content-Type: application/json\r\n\r\n",
		"65535\r\n"),
	RUN("PUT /dir/text",
		"PUT /dir/text\r\n"
		"Content-Type: text/plain\r\n\r\n",
		"a single line text\r\n"),
	RUN("PUT /dir/text length",
		"PUT /dir/text\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 23\r\n\r\n",
		"multiple\nlines\nмessage\n\r\n"),
	RUN("PUT /blob", "PUT /blob\r\n"
		"Content-Type: application/octet-stream\r\n"
		"Content-Length: 27\r\n\r\n",
		"\001\002\003\004\nbinary\nbytes\nµessage\n"),
	RUN("PUT /obj",
		"PUT /obj\r\nContent-Type: application/json\r\n\r\n",
		"{\"data\":1}"),
	RUN("PUT /obj w/ws",
		"PUT /obj\r\nContent-Type: application/json\r\n\r\n",
		"{ \"data\" : 2 }"),
	RUN("POST /obj",
		"POST /obj\r\nContent-Type: application/json\r\n\r\n",
		"{\"data\":3}"),
//10
	RUN("POST /data",
		"POST /data\r\nContent-Type: application/json-rpc\r\n\r\n",
		"{\"data\":3}"),
	RUN("POST /obj w/ Accept",
		"POST /obj\r\nAccept: text/html,application/json;q=0.8\r\n"
		"Content-Type: application/json\r\n\r\n",
		"{\"data\":4}"),
};

#undef _T_
#define _T_ (1300)

static cstring const Master[countof(Test013::tests)] = {
	 _P_( 0), _P_( 1), _P_( 2), _P_( 3), _P_( 4),
	 _P_( 5), _P_( 6), _P_( 7), _P_( 8), _P_( 9),
	 _P_(10), _P_(11) //, _P_(12), _P_(13), _P_(14),
};

#include "013.inc"

cstring Test013::master() const noexcept {
	return Master[index()];
}

