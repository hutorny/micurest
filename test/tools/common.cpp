/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * common.cpp - cojson tests, test framework implementation
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

#include <string.h>
#include "common.hpp"

#ifndef COJSON_SUITE_SIZE
#	define COJSON_SUITE_SIZE (200)
#endif

namespace cojson {


	namespace test {

	static const Test * tests[COJSON_SUITE_SIZE];
	static int tcount = 0;
	cstream cstream::instance;

	template<>
	bool match<char const*>(const char* a, void const* b, unsigned n) noexcept {
		return memcmp(a,b,n) == 0;
	}

	bool rstream::put(char_t val) noexcept {
		if( ptr == nullptr ) {
			Environment::instance().dump(val, error()== error_t::noerror);
			return true;
		}
		if( pos >= max ) {
			error(error_t::eof);
			return false;
		}
		Environment::instance().dump(val, error()== error_t::noerror);
		if( ptr[pos] == val ) {
			//TODO print output if( Environment::instance().n)
			++pos;
		} else {
			error(error_t::mismatch);
		}
		return true;
	}

	unsigned Test::count() noexcept {
		return tcount;
	}

	void Test::add(const Test *test) noexcept {
		if( tcount < COJSON_SUITE_SIZE )
			tests[tcount++] = test;
	}
	int Test::benchmark(const Environment& env) noexcept {
		int t = env.getsingle();
		if( t < 0 ) {
			env.msgt(LVL::silent, TSTR("Benchmark mode is missing test num to run\n"));
			return -1;
		}
		const Test* test = tests[t];
		int count = env.getloopcount();
		env.msg(LVL::normal, "Benchmarking test #%d for %d loops\n", t, count);
		env.startclock();
		while( --count >= 0 ) {
			if( test->run(env) ) {
				env.msg(LVL::silent, "Test #%d failed\n", t);
				return -1;
			}
		}
		env.msg(LVL::silent, "%d loops complete in %ld us\n",
				env.getloopcount(),env.elapsed());
		return 0;
	}
	int Test::runall(const Environment& env) noexcept {
		int i = env.getsingle();
		if( tcount <= 0 ) {
			env.msgt(LVL::silent, TSTR("No tests available\n"));
			return bad;
		}
		if( i >= 0 ) {
			if( i >= (signed int)tcount ) {
				env.msg(LVL::silent, "Tests %d not available\n",i);
				return bad;
			}
			env.msg(LVL::normal, "Running test # %d\n", i);
		}
		else
			env.msg(LVL::verbose,"Running %u tests\n",count());

		int n = i < 0 ? tcount : i + 1;
		if( i < 0 ) i = 0;
		int bad = 0;
		const Test** test = tests + i;

		env.begin();
		while(i < n) {
			env.next();
			result_t r = success;
			const Test* t = *test;
			if( t && (nullptr != t->frun) ) {
				env.msg(LVL::verbose, "INFO #%3d: running '%s:%d' ",
					i, env.shortname(t->filename), t->index());
				env.msgt(LVL::verbose, t->description);
				env.matching(t->master());
				if( (r = t->run(env) ) ) {
					env.msg(LVL::silent,
						"BAD  #%3d: test '%s:%d' returned %X\n", i,
						env.shortname(t->filename), t->index(), r);
					++bad;
				} else {
					if( ! env.matches() ) {
						env.msg(LVL::normal,
							"BAD  #%3d: test '%s:%d' does not match master\n",
							i, env.shortname(t->filename),t->index());
						env.msgc(LVL::debug, t->master());
						++bad;
						if( env.stoponfail() ) break;
					}
				}
			} else {
				env.msg(LVL::silent,
					"FAIL #%3d: test '%s' not runnable\n", i,
					(t ? env.shortname(t->filename) : "."));
				if( env.stoponfail() ) break;
			}
			env.resetbuffsize();
			env.dump(r == success && env.output.error() == error_t::noerror);
			env.master(t->filename, t->index());
			++i;
			++test;
		}
		if( env.getsingle() < 0 ) {
			if( bad ) {
				env.msg(LVL::normal,
						"STAT    : %d out of %d tests fail\n",bad, tcount);
			} else {
				env.msg(LVL::normal,
					"STAT    : all %d tests pass\n",tcount);
			}
		} else {
			env.msg(LVL::normal, "STAT    : test #%d %s\n", env.getsingle(),
					(bad ? "fail" : "pass"));
		}
		env.end();
		return bad;
	}
}}
