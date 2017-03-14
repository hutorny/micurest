/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 07-object-pdo.cpp - cojson tests, code size metrics
 * metric=read/write plain data object
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

struct Pdo {
	struct Name {
		NAME(u)
		NAME(s)
	};
    char s[16];
    short u;
    static const clas<Pdo>& structure() {
    	return O<Pdo,
    		P<Pdo, Name::s, countof(&Pdo::s), &Pdo::s>,
			P<Pdo, Name::u, decltype(Pdo::u), &Pdo::u>
    >();
    }
};

static void run(lexer& in, ostream& out) {
	Pdo& pdo = *(Pdo*) nullptr;
	Pdo::structure().read(pdo, in);
	Pdo::structure().write(pdo, out);
}

static runner test(run);
/* avr pgm/data: 4180/105 bytes  */
