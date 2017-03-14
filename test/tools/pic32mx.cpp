/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * pic32mx.cpp - cojson tests, PIC32MX specific implementation (not finished)
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
#include <stdio.h>
#include "common.hpp"

#ifndef COJSON_TEST_BUFFER_SIZE
#	define COJSON_TEST_BUFFER_SIZE (4096)
#endif


namespace cojson {

namespace test {

struct DefaultEnvironment : Environment {
	mutable long usec_start = 0;
	inline DefaultEnvironment() noexcept  {
		setverbose(normal);
		setoutlvl(nothing);
	}

	void master(const char* file, int index) const noexcept { }

	void startclock() const noexcept {
//		usec_start = micros();
	}
	long elapsed() const noexcept {
//		return micros() - usec_start;
		return 0;
	}

	bool write(char_t b) const noexcept {
//		Serial.write(b);
		return b;
	}

	mutable char buff[128];
	void msg(verbosity lvl, const char *fmt, ...) const noexcept  {
		if( lvl > options.level ) return;
		va_list args;
		va_start(args, fmt);
		vsprintf(buff, fmt, args);
		va_end(args);
//		Serial.write(buff);
	}

	void msgc(verbosity lvl, cstring master) const noexcept {
		if( lvl > options.level ) return;
//		Serial.write((const char_t*)master);
//		Serial.write('\n');
	}

	void msgt(verbosity lvl, tstring master) const noexcept {
		if( lvl > options.level ) return;
//		Serial.write((const char_t*)master);
//		Serial.write('\n');
	}

	void out(bool success, const char *fmt, ...) const noexcept {
		if( noout(success, false) ) return;
		va_list args;
		va_start(args, fmt);
		vsprintf(buff, fmt, args);
		va_end(args);
//		Serial.write(buff);
	}
	const char* shortname(const char* filename) const noexcept {
		const char* r;
		return ((r=strrchr(filename,'/'))) ? ++r : filename;
	}
};


static DefaultEnvironment environment;

Environment& Environment::instance() noexcept {
	return environment;
}


}}

//static constexpr int ledPin = 13;

//static void print_help() {
//	static constexpr const char help[] =
//	"\ncojson test runner\n"
//	"<cr> - run selected test(s)\n"
//	"  <K>  - select single test K\n"
//	"  *    - select all tests\n"
//	"  <N>b - run selected test N times\n"
//	"verbosity control:\n"
//	"  v    - verbose\n"
//	"  n    - normal\n"
//	"  s    - silent\n"
//	"output control:\n"
//	"  a    - all\n"
//	"  j    - as json\n"
//	"  p    - positive only\n"
//	"  q    - negative only\n"
//	"  -    - nothing\n"
//	"example:\n"
//	"1 1000b\n"
//	"  run test 1 1000 times\n"
//	"n a *<cr>\n"
//	"  verbosity normal, output all, run all tests\n";
//	Serial.write(help);
//}
/*
   text	   data	    bss	    dec	    hex	filename
 100952	   2628	  12264	 115844	  1c484	cojson.elf
 */
static int command() {
//	int value = 0;
//	bool has = false;
//	unsigned char toggle = 0;
//	int chr;
//	while( ! Serial.available() ) {
//		delay(100);
//		Serial.write((++toggle) & 4 ?  "\r:" : "\r.");
//		digitalWrite(ledPin, toggle & 0x10 ? HIGH : LOW);
//	}
//	while(true) {
//		while( ! Serial.available() ) {
//			delay(100);
//			digitalWrite(ledPin, (++toggle) & 0x10 ? HIGH : LOW);
//		}
//		chr = Serial.read();
//		Serial.write(chr);
//		switch( chr ) {
//		default: Serial.write("\r?"); break;
//		case '?': print_help(); break;
//		case '\r':
//		case '\n': if( has ) environment.setsingle(value); return 1;
//		case '\t':
//		case ' ': if( has ) environment.setsingle(value);
//			value = 0; has = false; break;
//		case 'v': environment.setverbose(LVL::verbose); break;
//		case 's': environment.setverbose(LVL::silent); break;
//		case 'n': environment.setverbose(LVL::normal); break;
//		case '-': environment.setoutlvl(Environment::dumping::nothing); break;
//		case 'a': environment.setoutlvl(Environment::dumping::all); break;
//		case 'j': environment.setoutlvl(Environment::dumping::as_json); break;
//		case 'p': environment.setoutlvl(Environment::dumping::positive); break;
//		case 'q': environment.setoutlvl(Environment::dumping::negative); break;
//		case '*': environment.setsingle(-1); break;
//		case 'b':
//				environment.setbenchmark(has ? value : 1000);
//				Serial.write('\n');
//				return 2;
//		case '0':
//		case '1':
//		case '2':
//		case '3':
//		case '4':
//		case '5':
//		case '6':
//		case '7':
//		case '8':
//		case '9': has = true; value = value * 10 + chr - '0'; break;
//		}
//	}
	return 1;
}


void setup() {
//	Serial.begin(9600);
//	pinMode(ledPin, OUTPUT);
}

void loop() {
	switch( command() ) {
	case 1: Test::runall(environment); break;
	case 2: Test::benchmark(environment); break;
//	default:
//		Serial.write("\n?\n");
//		delay(1000);
	}

}

int main(void) {
	setup();
	while(true) loop();
	return 0;
}

