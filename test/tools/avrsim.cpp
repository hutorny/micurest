/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * avrsim.cpp - cojson tests, avr-sim specific implementation
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


#include "avrsim.hpp"
#include <string.h>

using namespace cojson;
using namespace cojson::test;
using namespace avrsim;

#ifndef COJSON_TEST_BUFFER_SIZE
#	define COJSON_TEST_BUFFER_SIZE (128)
#endif


namespace cojson {
namespace test {

/* -W 0xDC, */
static osimstream<0xDC> simout;
static test_buffer<COJSON_TEST_BUFFER_SIZE> outb;


static struct DefaultEnvironment : McuEnvironment {
	DefaultEnvironment(utils::buffer& out) noexcept : McuEnvironment(out) {}
	/* prints a message to stderr 				*/
	void msg(verbosity lvl, const char *fmt, ...) const noexcept
			__attribute__ ((format (printf, 3, 4))) {
		simout.put(fmt);
	}
	/* prints test result per success and dumping settings 	*/
	void out(bool success, const char *fmt, ...) const noexcept
			__attribute__ ((format (printf, 3, 4))) {
		simout.put(fmt);
	}
	const char* shortname(const char* filename) const noexcept {
		return filename;
	}
	void startclock() const noexcept {
		//TODO
	}
	long long elapsed() const noexcept {
		return 0;
	}
	void setbuffsize(unsigned size) const noexcept {
		outb.setlimit(size);
	}
	void resetbuffsize() const noexcept {
		outb.setlimit(outb.maxsize());
	}
	void write(char b) const noexcept {
		simout.put(b);
	}
	void write(const char *str) const noexcept  {
		simout.put(str);
	}
	void write(const char *buffer, unsigned size) const noexcept {
		while(size--) simout.put(*buffer++);
	}

	/** match output with the given master  */
	int match(const char* data) const noexcept {
		return data ? ! memcmp(
			output.begin(), data, output.count()*sizeof(char_t))
			: !0;
	}


} environment(outb);
}}

int main() {
	Test::runall(environment);
	halt();
	return 0;
}

