/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 032.cpp - cojson tests, reading homogeneous arrays
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
namespace cojson { namespace test {

struct Test032 : Test {
	static Test032 tests[];
	inline Test032(cstring name, cstring desc, runner func) noexcept
	  : Test(name, desc, func) {}
	int index() const noexcept {
		return (this-tests);
	}
};
/* combination <T,N> should not be used twice in a test */
template<typename T, unsigned N>	T arr<T,N>::data[N];

#define COMMA ,
#define RUN(name, body) Test032(OMIT(__FILE__),OMIT(name), \
		[](const Environment& env) noexcept -> result_t body)
Test032 Test032::tests[] = {
	RUN("parsing array: short[6]", {
		return arr<short COMMA 6>::run(env, CSTR("[ 1, 2, 3, 4, 5, 6]"),
				{1, 2, 3, 4, 5, 6}); }),
	RUN("parsing array: unsigned[8] with one negative", {
		return arr<unsigned COMMA 8>::run(env,
		CSTR( "[ 150, 260, 370, -400, 580, 690, 800, 1000]"),
				{150, 260, 370, 0, 580, 690, 800, 1000}, error_t::mismatch); }),
	RUN("parsing array: int[6] + traling spaces", {
		return arr<int COMMA 6>::run(env,
			CSTR( "[ 1 , 2\n, 3\t , 4   ,5, 6 ] \n"),
					{1, 2, 3, 4, 5, 6}); }),
	RUN("parsing array: int[4] + no spaces", {
		return arr<int COMMA 4>::run(env, CSTR("[1,2,3,4] \n"),{1, 2, 3, 4});}),
	RUN("parsing array: bool[6] + traling spaces", {
		return arr<bool COMMA 6>::run(env,
			CSTR("[true , true, false ,false  ,true, true ] \n"),
				  {true, true, false, false, true, true}); }),
	RUN("parsing array: int[5] + extra elements", {
		return arr<int COMMA 5>::run(env, CSTR("[ 1 , 2 , 3 , 4 , 5, 6, 7]"),
				{1, 2, 3, 4, 5}, error_t::overrun); }),
	RUN("parsing array: long[2] + excessive elements", {
		return arr<long COMMA 2>::run(env, CSTR("[1,2] [3]"), {1, 2}); }),
	RUN("parsing array: short[2] null", {
		return arr<short COMMA 2>::run(env, CSTR("null"), {0,0}); }),
	RUN("parsing array: int[3] mismatching float", {
		return arr<int COMMA 3>::run(env, CSTR("[1, 2.0, 3 ]"), {1, 0, 3},
				error_t::mismatch); }),
	RUN("parsing array: char[3] mismatching array", {
		return arr<signed char COMMA 3>::run(env, CSTR("[1, [2, 3], 4]"),
				{1, 0, 4}, error_t::mismatch); }),
	RUN("parsing array: char[4] mismatching string", {
		return arr<signed char COMMA 4>::run(env, CSTR("[1, \"2\", 3, 4 ]"),
				{1, 0, 3, 4}, error_t::mismatch); }),
	RUN("parsing array: char[2] mismatching depth", {
		return arr<signed char COMMA 2>::run(env, CSTR("[[1, 2]]"), {0, 0},
				error_t::mismatch); }),
	RUN("parsing array: char[5] mismatching object", {
		return arr<signed char COMMA 5>::run(env, CSTR("[1, 2,{}, {} , 4]"),
				{1, 2, 0, 0, 4}, error_t::mismatch); }),
	RUN("parsing array: int[2] mismatching nested object", {
		return arr<int COMMA 2>::run(env, CSTR("[{\"i\":{}}, 2]"), {0, 2},
				error_t::mismatch); }),
	RUN("parsing array: short[5] mismatching high depth", {
		return arr<short COMMA 5>::run(env, CSTR(
				"[1, [[[[[[[[]]]]]]]], "
				"{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{\"a\":{}}}}}}}},"
				"{\"a\":[{\"a\":{\"a\":[[{\"a\":[]}]]}}]},"
				"5]"),
				{1, 0, 0, 0, 5}, error_t::mismatch); }),
	RUN("parsing array: short[3] overrun depth", {
		return arr<short COMMA 3>::run(env,
			CSTR("[1, [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]"),
				{1, 0, 0}, error_t::mismatch | error_t::bad ); }),
	RUN("parsing array: uchar[2] wrong end", {
		return arr<unsigned char COMMA 2>::run(env,
				CSTR("[1, 2}"), {1, 2}, error_t::bad ); }),
	RUN("parsing array: uchar[3] wrong end in mismatched", {
		return arr<unsigned char COMMA 3>::run(env, CSTR("[1, [[]}], 3]"),
				{1, 0, 0}, error_t::mismatch | error_t::bad ); }),
	RUN("parsing array: long[3]", {
		return arr<long COMMA 3>::run2(env, CSTR("[\n\t1,\n\t2,\n\t3\n]"),
			{1, 2, 3}); }),
};

}}
