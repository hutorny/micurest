/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 09-complex-object.cpp - cojson tests, code size metrics
 * metric=read/write complex object
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

reader<ip4_t> reader<ip4_t>::unit __attribute__((weak));

static void run(lexer& in, ostream& out) {
	Config& tmp = *(Config*)nullptr;
	Config::structure().read(tmp, in);
	Config::structure().write(tmp, out);
}

static runner test(run);
/* avr pgm/data: 13224/1050 bytes  */
