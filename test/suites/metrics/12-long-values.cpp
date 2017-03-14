/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 12-long-values.cpp - cojson tests, code size metrics
 * metric=read/write two long values
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

#include "bench.hpp"
using namespace cojson;
using namespace test;

static long& long1() noexcept {
	static long val;
	return val;
}

static long& long2() noexcept {
	static long val;
	return val;
}

static void run(lexer& in, ostream& out) {
	V<long, long1>().read(in);
	V<long, long1>().write(out);
	V<long, long2>().read(in);
	V<long, long2>().write(out);
}

static runner test(run);
/* avr pgm/data: / bytes */
