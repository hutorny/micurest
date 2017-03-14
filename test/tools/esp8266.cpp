/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * teensy.cpp - cojson tests, Teensy 3.1 specific implementation
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

#include <stdarg.h>
#include <string.h>
#include "esp8266_user.h"
#include "common.hpp"
#include "dbg.h"

#ifndef COJSON_TEST_BUFFER_SIZE
#	define COJSON_TEST_BUFFER_SIZE (4096)
#endif

static struct Console {
	inline void write(const char* s) noexcept { serial_write(s); }
	inline void write(char c) noexcept { serial_writec(c); }
	inline bool available() noexcept { return serial_available(); }
	char read() noexcept { return serial_read(); }
	inline Console() {
	}
	static inline void attach() {
		user_rx_installcb(rx_callback);
		user_hb_installcb(hb_callback);
	}
private:
	bool command(unsigned len);
	static void rx_callback(unsigned len);
	static void hb_callback(int cnt) {
		if( ! busy ) {
			serial_write(cnt & 1 ? "\r:" : "\r.");
		}
	}
	static bool busy;
} Serial;

void Console::rx_callback(unsigned len) {
	busy = true;
	busy = Serial.command(len);
}

void console_attach() {
	Console::attach();
}

bool Console::busy = false;

namespace cojson {
	namespace details {

	template<typename T>
	inline T pow10(signed short v) noexcept {
		T pow = 1;
		while( v > 0 ) { pow *= 10.; --v; }
		while( v < 0 ) { pow /= 10.; ++v; }
		return pow;
	}

	}

namespace test {

struct DefaultEnvironment : Environment {
	mutable unsigned usec_start = 0;
	inline DefaultEnvironment() noexcept {
		setverbose(normal);
		setoutlvl(nothing);
	}

	/** match output with the given master  */

	void master(const char* file, int index) const noexcept { }

	void startclock() const noexcept {
		usec_start = micros();
	}
	long elapsed() const noexcept {
		return micros() - usec_start;
	}

	bool write(char b) const noexcept {
		if( b ) Serial.write(b);
		return b;
	}

	void msg(verbosity lvl, const char *fmt, ...) const noexcept  {
		if( lvl > options.level ) return;
		va_list args;
		va_start(args, fmt);
		ets_vprintf(ets_putc, fmt, args);
		va_end(args);
	}

	void msgc(verbosity lvl, cstring master) const noexcept {
		if( lvl > options.level ) return;
		Serial.write((const char_t*)master);
		Serial.write('\n');
	}

	void msgt(verbosity lvl, tstring master) const noexcept {
		if( lvl > options.level ) return;
		Serial.write((const char_t*)master);
		Serial.write('\n');
	}

	void out(bool success, const char *fmt, ...) const noexcept {
		if( noout(success, false) ) return;
		va_list args;
		va_start(args, fmt);
		ets_vprintf(ets_putc, fmt, args);
		va_end(args);
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

static void print_help() {
	static constexpr const char help[] =
	"\ncojson test runner\n"
	"<cr> - run selected test(s)\n"
	"  <K>  - select single test K\n"
	"  *    - select all tests\n"
	"  <N>b - run selected test N times\n"
	"verbosity control:\n"
	"  v    - verbose\n"
	"  n    - normal\n"
	"  s    - silent\n"
	"output control:\n"
	"  a    - all\n"
	"  j    - as json\n"
	"  p    - positive only\n"
	"  q    - negative only\n"
	"  -    - nothing\n"
	"example:\n"
	"1 1000b\n"
	"  run test 1 1000 times\n"
	"n a *<cr>\n"
	"  verbosity normal, output all, run all tests\n";
	Serial.write(help);
}


bool Console::command(unsigned len) {
	static int value = 0;
	static bool has = false;
	int chr;
	while(len--) {
		chr = read();
		switch( chr ) {
		default: write("\r?"); break;
		case '?': print_help(); break;
		case '\r':
		case '\n': if( has ) environment.setsingle(value);
			Test::runall(environment);
			value = 0; has = false; return false;
		case '\t':
		case ' ': if( has ) environment.setsingle(value);
			value = 0; has = false; break;
		case 'd': environment.setverbose(LVL::debug); write("ebug\n");  break;
		case 'v': environment.setverbose(LVL::verbose); write("erbose\n"); break;
		case 's': environment.setverbose(LVL::silent); write("ilent\n"); break;
		case 'n': environment.setverbose(LVL::normal); write("ormal\n"); break;
		case '-': environment.setoutlvl(Environment::dumping::nothing); break;
		case 'a': environment.setoutlvl(Environment::dumping::all); break;
		case 'j': environment.setoutlvl(Environment::dumping::as_json); break;
		case 'p': environment.setoutlvl(Environment::dumping::positive); break;
		case 'q': environment.setoutlvl(Environment::dumping::negative); break;
		case '*': environment.setsingle(-1); break;
		case 'b':
				environment.setbenchmark(has ? value : 1000);
				write('\n');
				system_soft_wdt_stop();
				Test::benchmark(environment);
				system_soft_wdt_restart();
				value = 0; has = false;
				return false;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': has = true; value = value * 10 + chr - '0'; break;
		}
	}
	return true;
}

void dbg(const char *fmt, ...) noexcept  {
	va_list args;
	va_start(args, fmt);
	ets_vprintf(ets_putc, fmt, args);
	va_end(args);
}
