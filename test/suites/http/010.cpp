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

struct Test010 : Test {
	static Test010 tests[];
	inline Test010(tstring name, tstring desc, cstring input) noexcept
		: Test(name, desc), inp(input), expected(application::result_t::close){}
	inline Test010(tstring name, tstring desc, cstring input,
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
		return res == expected ? success : bad;
	}
	static void cleanup() noexcept;
	cstring master() const noexcept;
	cstring const inp;
	application::result_t expected;
};

/* Test plan
 * positive tests:
 + 		GET read-only texts
 + 		PUT/GET plain text values,
 + 		PUT/GET JSON objects (011.cpp)
 * 		PUT/GET blobs
 * 		PUT/GET deep items
 * 		PUT/GET with Etag
 * negative tests
 * 		bad request
 * 		name too long
 * 		not found
 * 		bad option
 * 		content too large
 * 		bad method
 * 		Etag too long
 * 		blob too long
 * 		text too long
 * negative tests with single custom response
 * negative tests with set of custom responses
 *
 * FIXME blobs with zero cannot be tested with cstream.
 */


namespace name {
ALIAS(index,index.html)
NAME(natural)
NAME(numeric)
NAME(text)
NAME(logical)
NAME(blob)
NAME(dir)
}

static cstring  index_html() noexcept {
	return CSTR(
		"<html>\n\t<body>\n"
		"\t<h1>Plain text values</h1>\n"
		"\t</body>\n</html>");
}

using namespace cojson::accessor;
static unsigned natural;
static float numeric;
static bool logical;
char_t text[32];
unsigned char blob[256];
unsigned blob_length = 0;

void Test010::cleanup() noexcept {
	natural = 0;
	numeric = 0;
	logical = false;
	memset(text,0,sizeof(text));
	memset(blob,0,sizeof(text));
	blob_length = 0;
}

static float get_numeric() noexcept { return numeric; }
static void put_numeric(float v) noexcept { numeric = v; }

static const directory& root() noexcept {
	return Root<
		E<name::index,
			N<media::text::html, index_html>
		>,
		F<name::natural, pointer<unsigned, &natural>>,
		F<name::numeric, pointer<float, &numeric>>,
		F<name::logical, pointer<bool, &logical>>,
		D<name::dir,
			F<name::natural, unsigned, &natural>,
			F<name::numeric, float, get_numeric, put_numeric>,
			F<name::text, countof(text), text>
		>,
		F<name::blob,&blob_length, sizeof(blob), blob>
	>();
}


#define RUN(name, input) Test010(OMIT(__FILE__),OMIT(name), CSTR(input))
#define NEG(name, input, result) Test010(OMIT(__FILE__),OMIT(name), CSTR(input), result)

Test010 Test010::tests[] = {
	NEG("GET /",
		"GET /\r\n\r\n", application::result_t::bad),
	RUN("GET /index.html",
		"GET /index.html\r\n\r\n"),
	RUN("GET / html",
		"GET /\r\n""Accept: text/html\r\n\r\n"),
	RUN("GET / HTTP/0.1 html",
		"GET / HTTP/0.1\r\nAccept: text/html\r\n\r\n"),
	RUN("GET / json",
		"GET /\r\nAccept: application/json\r\n\r\n"),
	RUN("GET /natural",
		"GET /natural\r\n\r\n"),
	RUN("GET /natural text",
		"GET /natural\r\nAccept: text/html\r\n\r\n"),
	RUN("GET /numeric",
		"GET /numeric\r\n\r\n"),
	RUN("PUT /numeric",
		"PUT /numeric\r\n"
		"Content-Type: application/json\r\n\r\n20\r\n"),
	RUN("GET /logical",
		"GET /logical\r\n\r\n"),
	RUN("PUT /logical",
		"PUT /logical\r\n"
		"Content-Type: application/json\r\n\r\n true \r\n"),
	RUN("GET /logical",
		"GET /logical HTTP/0.1\r\n\r\n"),
	RUN("GET /dir",
		"GET /dir\r\nAccept: text/plain\r\n\r\n"),
	RUN("GET /dir/numeric",
		"GET /dir/numeric\r\n\r\n"),
	RUN("PUT /dir/numeric",
		"PUT /dir/numeric\r\n"
		"Content-Length: 7\r\n"
		"Content-Type: application/json\r\n\r\n0.65535\r\n"),
	RUN("GET /dir/numeric",
		"GET /dir/numeric\r\nAccept: text/plain\r\n\r\n"),
	RUN("PUT /dir/natural",
		"PUT /dir/natural\r\n"
		"Content-Type: application/json\r\n\r\n65535\r\n"
		"GET /dir/natural\r\n\r\n"),
	RUN("PUT /dir/text", "PUT /dir/text\r\n"
		"Content-Type: text/plain\r\n\r\na single line text\r\n"
		"GET /dir/text\r\n\r\n"),
	RUN("PUT /dir/text length", "PUT /dir/text\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 23\r\n\r\n"
		"multiple\nlines\nмessage\n\r\n"
		"GET /dir/text\r\n\r\n"),
	RUN("PUT /blob", "PUT /blob\r\n"
		"Content-Type: application/octet-stream\r\n"
		"Content-Length: 27\r\n\r\n"
		"\001\002\003\004\nbinary\nbytes\nµessage\n"
		"GET /blob\r\n\r\n"),
};

#undef _T_
#define _T_ (1000)

static cstring const Master[countof(Test010::tests)] = {
	 _P_( 0), _P_( 1), _P_( 2), _P_( 3), _P_( 4),
	 _P_( 5), _P_( 6), _P_( 7), _P_( 8), _P_( 9),
	 _P_(10), _P_(11), _P_(12), _P_(13), _P_(14),
	 _P_(15), _P_(16), _P_(17), _P_(18), _P_(19),
//	 _P_(20), _P_(21), _P_(22), _P_(23), _P_(24),
//	 _P_(25), _P_(26), _P_(27), _P_(28), _P_(29),
};

#include "010.inc"

cstring Test010::master() const noexcept {
	return Master[index()];
}

