/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * 004.cpp.c - cojson tests, writing extern C arrays and objects
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

#include "004.h"
#include "stdio.h"

unsigned * uitem(unsigned n) {
	static unsigned items[] = { 111, 222, 333, 444, 555 };
	if(n < (sizeof(items)/sizeof(items[0]))) return items + n;
	return (unsigned *)0;
}

static char str[] = "extern\t\"C\"";

const char* strv() {
	return str;
}

static struct CPod cpods = { 9, -666, 888L, 99999999LL, "extern POD"};

struct CPod * cpod() {
	return &cpods;
}

struct CPod dpod = { 9, 300, 400L, 99999999LL, "extern POD"};

