/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 00-base.cpp - cojson tests, code size metrics
 * metric=base application
 * NOTE: These tests are not to be run!
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
#ifdef ARDUINO
#	include "Arduino.h"
#	undef F
#	undef bit
#	undef abs
#else
#	include <stdio.h>
#endif
#include "bench.hpp"
namespace cojson {
namespace test {

static runner::func tests[10];
static unsigned char count = 0;

void runner::add(runner::func r) noexcept {
	if( count >= countof(tests) ) return;
	tests[count++] = r;
}

Environment& Environment::instance() noexcept {
	return * (Environment*) nullptr;
}

}}
using namespace cojson;
using namespace test;

template<config::iostate_is V= config::iostate>
class iostate_v : public iostate {};

template<>
class iostate_v<config::iostate_is::_virtual> : public virtual iostate {
public:
	inline iostate_v() noexcept : iostate() {}
	void error(error_t) noexcept {}
	void clear() noexcept { }
	error_t error() const noexcept { return error_t::noerror; }
};

static struct inull : istream, iostate_v<> {
	bool get(char_t&) noexcept { return true; }
} in;


static struct onull : ostream, iostate_v<> {
	bool put(char_t) noexcept { return true; }
} out;

static void run(lexer&, ostream&) {}

runner empty(run);

void setup() {
	volatile Config& tmp = *(Config*)nullptr;
	/*some statements to activate operations on double, and long */
	tmp.loadavg[0] = tmp.loadavg[1] * 10 + tmp.loadavg[2];
	tmp.memtotal = tmp.uptime * 10L + tmp.connmax;
}

void loop() {
	lexer json(in);
	for(unsigned char i = 0; i < count; ++i)
		tests[i](json,out);
}

#ifndef ARDUINO
int main(void) {
	setup();
	while(true) loop();
	return 0;
}
#endif
