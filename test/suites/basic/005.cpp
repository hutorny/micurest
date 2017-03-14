/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 005.cpp - cojson tests, writing values
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


struct Test005 : Test {
	static Test005 tests[];
	inline Test005(cstring name, cstring desc, runner func)
		noexcept : Test(name, desc,func) {}
	int index() const noexcept {
		return (this-tests);
	}
	cstring master() const noexcept;
};

typedef named_static<short, 5> VI;
typedef named_static<unsigned short, 500> VS;

VI vi(CSTR("vi"), 32567);
VS vs(CSTR("vs"), 1234);


static inline result_t _R(bool pass, const Environment& env) noexcept {
	return combine1(pass, error_t::noerror,	env.output.error());
}

#define RUN(name, body) Test005(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test005 Test005::tests[] = {
	RUN("writing values: w/ getter&setter", {
		return _R(V<VI::type, VI::get, VI::set>().write(env.output),env);
	}),
	RUN("writing values: w/ setter only", {
		return _R(V<VI::type, VI::set>().write(env.output),env);
	}),
	RUN("writing values: w/ getter only", {
		return _R(V<VS::type, VS::get>().write(env.output),env);
	}),

};

#undef _T_
#define _T_ (500)


static cstring const Master[details::countof(Test005::tests)] = {
	 _P_(0), _P_(1), _P_(2)
};

#include "005.inc"

cstring Test005::master() const noexcept {
	return Master[index()];
}


