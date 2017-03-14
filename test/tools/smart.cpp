/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * smart.cpp - cojson tests, Linkit Smart specific implementation
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

#include "Arduino.h"
#include <avr/interrupt.h>
#undef F
#undef bit
#undef abs
#include "common.hpp"
#include <string.h>

#ifndef COJSON_TEST_BUFFER_SIZE
#	define COJSON_TEST_BUFFER_SIZE (128)
#endif
namespace avr {

/** special function register accessible as a regular RAM location */
template<unsigned char addr>
struct sfr {
	typedef unsigned char data_t;
	constexpr static unsigned address = addr;
	static constexpr volatile data_t* reg = reinterpret_cast<volatile data_t*>(addr);
	static inline void set(data_t val) noexcept {
		asm volatile(
			"sts	%0, %1"
			:
			: "M"(address), "r"(val)
			: "memory"
		);

	}
	static inline data_t get() noexcept {
		register data_t val;
		asm volatile(
			"lds	%0, %1"
			: "r"(val)
			: "M"(address)
			:
		);
		return val;
	}
};

template<class gfr_t, unsigned _bit>
struct sfrbit {
	typedef typename gfr_t::data_t data_t;
	constexpr static unsigned char address = gfr_t::address;
	constexpr static data_t mask = (1U << _bit);
	constexpr static unsigned bit = _bit;

	static inline void set() noexcept {
		asm volatile(
			"lds	r24, %0\n"
			"ori	r24, %1\n"
			"sts	%0, r24"
			:
			: "M"(address), "i"(mask)
			: "r24","memory"
		);
	}
	static inline void clr() noexcept {
		asm volatile(
			"lds	r24, %0\n"
			"andi	r24, %1\n"
			"sts	%0, r24"
			:
			: "M"(address), "i"(~mask)
			: "r24","memory"
		);
	}
	static inline bool get() noexcept {
		register bool v;
		asm volatile(
			"lds	%0, %1\n"
			"sbrs	%0, %2\n"
			"clr	%0"
			: "=r"(v)
			: "M"(address), "I"(_bit)
			:
		);
		return v;
	}

};
struct interrupts {
	static inline void disable() noexcept {
		asm volatile(
			"in __tmp_reg__,__SREG__\n"
			"cli"
			::: "memory"
		);
	}
	static inline void restore() noexcept {
		asm volatile(
			"out __SREG__, __tmp_reg__"
			::: "memory"
		);
	}
};
/** double byte special register */
template<unsigned char addr>
struct dsr {
	typedef unsigned short data_t;
	static constexpr unsigned char upper = addr + 1;
	static inline data_t get() noexcept {
		data_t res;
		/* [ds-15.3 Accessing 16-bit Registers]
		 *  For a 16-bit read, the low byte must be read
		 *  before the high byte.
		 */
		asm volatile (
			"lds %A0, %1\n"
			"lds %B0, %2\n"
			: "=r"(res)
			: "i"(addr), "i"(addr+1)
			: "memory"
		);
		return res;
	}
	static inline void set(data_t val) {
		/* [ds-15.3 Accessing 16-bit Registers]
		 * To do a 16-bit write, the high byte must be written
		 * before the low byte.
		 */
		asm volatile (
			"sts %1, %B2\n"
			"sts %0, %A2\n"
			:: "i"(addr), "i"(addr+1), "r"(val)
			: "memory"
		);
	}
};

}
namespace atmega {
using namespace avr;
struct wdt : avr::sfr<0x60> {
	template<unsigned bit>
	struct wdtbit : sfrbit<wdt, bit> {};

	/**
	 * [ds-10.9.2] WDTCSR – Watchdog Timer Control Register
	 */
	struct wdif : wdtbit<7> {}; /** Watchdog Interrupt Flag	  	  */
	struct wdie : wdtbit<6> {}; /** Watchdog Interrupt Enable	  */
	struct wdce : wdtbit<4> {}; /** Watchdog Change Enable  	  */
	struct wde  : wdtbit<3> {}; /** Watchdog Reset Enable	  	  */
	struct wdp3 : wdtbit<5> {}; /** Watchdog Timer Prescaler 3	  */
	struct wdp2 : wdtbit<2> {}; /** Watchdog Timer Prescaler 2	  */
	struct wdp1 : wdtbit<1> {}; /** Watchdog Timer Prescaler 1	  */
	struct wdp0 : wdtbit<0> {}; /** Watchdog Timer Prescaler 0 	  */

	/** [ds-Table 10-2] Watchdog Timer Prescale Select
	 * Typical Time-out at VCC = 5.0V
	 */

	enum class prescaler_t {
		_16ms = 0,
		_32ms =                                          wdp0::mask,
		_64ms =                             wdp1::mask,
		_125ms=                             wdp1::mask | wdp0::mask,
		_250ms=                wdp2::mask,
		_500ms=	               wdp2::mask              | wdp0::mask,
		_1s   =				   wdp2::mask | wdp1::mask,
		_2s   =                wdp2::mask | wdp1::mask | wdp0::mask,
		_4s   =	wdp3::mask,
		_8s   =	wdp3::mask                             | wdp0::mask,
	};

	static inline void enable(prescaler_t&& prescaler, bool &&interrupts = false) {
		avr::interrupts::disable();
		reset();
		set(wdce::mask);
		set(static_cast<data_t>(prescaler) | (interrupts? wdie::mask : 0));
		avr::interrupts::restore();
	}

	static inline void disable() {
		avr::interrupts::disable();
		set(wdce::mask);
		set(0);
		avr::interrupts::restore();
	}

	static inline void reset() {
		asm volatile("wdr");
	}

	static inline volatile bool elapsed() {
		return get();
	}

	static inline volatile bool get() {
		return get() & wdif::mask;
	}

	inline operator bool() const volatile {
		return get();
	}
};
struct sp : dsr<0x5D> {

};
}

namespace cojson {
namespace test {

struct DefaultEnvironment : Environment {
	mutable long usec_start = 0;
	inline DefaultEnvironment() noexcept {
		setverbose(normal);
		setoutlvl(nothing);
	}

	void master(const char* file, int index) const noexcept { }

	void startclock() const noexcept {
		usec_start = micros();
	}
	long elapsed() const noexcept {
		return micros() - usec_start;
	}

	bool write(char b) const noexcept {
		if( b ) Serial1.write(b);
		return b;
	}
	mutable char buff[128];
	void msg(verbosity lvl, const char *fmt, ...) const noexcept  {
		if( lvl > options.level ) return;
		va_list args;
		va_start(args, fmt);
		vsprintf(buff, fmt, args);
		va_end(args);
		Serial1.write(buff);
	}

	void out(bool success, const char *fmt, ...) const noexcept {
		if( noout(success, false) ) return;
		va_list args;
		va_start(args, fmt);
		vsprintf(buff, fmt, args);
		va_end(args);
		Serial1.write(buff);
	}
	friend void dbg(const char *fmt, ...);
	const char* shortname(progmem<char> filename) const noexcept {
		static char buff[8] = {0};
		if(filename == nullptr) return "";
		const char * r = strrchr_P((const char *)filename,'/');
		r = (r == nullptr) ? (const char *)filename : r+1;
		strncpy_P(buff,r,sizeof(buff));
		return buff;
	}
	const char* shortname(const char* filename) const noexcept {
		if(filename == nullptr) return "";
		const char * r = strrchr(filename,'/');
		return r == nullptr ? filename : ++r;
	}
	void msgc(verbosity lvl, cstring str) const noexcept {
		if( lvl > options.level ) return;
		if( str != nullptr )
			while(*str) Serial1.write(*str++);
		Serial1.write('\n');
	}
	void msgt(verbosity lvl, tstring str) const noexcept {
		if( lvl > options.level ) return;
		if( str != nullptr )
			while(*str) Serial1.write(*str++);
		Serial1.write('\n');
	}

} environment;

Environment& Environment::instance() noexcept {
	return environment;
}
template<>
bool match<progmem<char>>(progmem<char> a, const void* b, unsigned n) noexcept {
	return memcmp_P(b,(const char*)a,n) == 0;
}

template<>
unsigned strlen(progmem<char> s) noexcept {
	return strlen_P((const char*)s);
}


}}

using namespace cojson;
using namespace cojson::test;

static constexpr int ledPin = 13;
static constexpr int greenLed = 3;

tstring help_str = TSTR(
	"cojson test runner\n"
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
	"watchdog control:\n"
	"  W    - watchdog on\n"
	"  w    - watchdog off\n"
	"example:\n"
	"1 1000b\n"
	"  run test 1 1000 times\n"
	"n a *<cr>\n"
	"  verbosity normal, output all, run all tests\n");

static void print_help() {
	environment.msgt(LVL::silent, help_str);
}

static unsigned short spmin = 0xFFFF;
extern "C" void* __noinit_end;

static int command() {
	int value = 0;
	bool has = false;
	unsigned char toggle = 0;
	int chr;
	while( ! Serial1.available() ) {
		delay(100);
		Serial1.write((++toggle) & 4 ?  "\r:" : "\r.");
		digitalWrite(ledPin, toggle & 0x10 ? HIGH : LOW);
	}
	while(true) {
		while( ! Serial1.available() ) {
			delay(100);
			digitalWrite(ledPin, (++toggle) & 0x10 ? HIGH : LOW);
		}
		chr = Serial1.read();
		Serial1.write(chr);
		switch( chr ) {
		default: Serial1.write("\r?"); break;
		case 'w':
			atmega::wdt::disable();
			environment.msg(LVL::silent,"atchdog off\n");
			break;
		case 'W':
			atmega::wdt::enable(atmega::wdt::prescaler_t::_16ms, true);
			environment.msg(LVL::silent,"atchdog on\n");
			break;
		case '?': print_help(); break;
		case '.': environment.msg(LVL::silent, "SP min: %X end:%X\n", spmin,(unsigned short)&__noinit_end); break;
		case '\r':
		case '\n': if( has ) environment.setsingle(value); return 1;
		case '\t':
		case ' ': if( has ) environment.setsingle(value);
			value = 0; has = false; break;
		case 'd': environment.setverbose(LVL::debug); break;
		case 'v': environment.setverbose(LVL::verbose); break;
		case 's': environment.setverbose(LVL::silent); break;
		case 'n': environment.setverbose(LVL::normal); break;
		case '-': environment.setoutlvl(Environment::dumping::nothing); break;
		case 'a': environment.setoutlvl(Environment::dumping::all); break;
		case 'j': environment.setoutlvl(Environment::dumping::as_json); break;
		case 'p': environment.setoutlvl(Environment::dumping::positive); break;
		case 'q': environment.setoutlvl(Environment::dumping::negative); break;
		case '*': environment.setsingle(-1); break;
		case 'b':
				environment.setbenchmark(has ? value : 1000);
				Serial1.write('\n');
				return 2;
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
}

ISR(WDT_vect) {
	atmega::wdt::wdif::clr();
	atmega::wdt::reset();
	static unsigned short sp;
	sp = atmega::sp::get();
	if( sp < spmin ) spmin = sp;
	if( sp < reinterpret_cast<unsigned short>(&__noinit_end))
		digitalWrite(greenLed, HIGH);
}

void dbg(const char *fmt, ...) noexcept  {
	va_list args;
	va_start(args, fmt);
	vsprintf(environment.buff, fmt, args);
	va_end(args);
	Serial1.write(environment.buff);
}

void setup() {
	Serial1.begin(115200);
	pinMode(ledPin, OUTPUT);
	pinMode(greenLed, OUTPUT);
	digitalWrite(greenLed, HIGH);
	print_help();
	digitalWrite(greenLed, LOW);
	atmega::wdt::enable(atmega::wdt::prescaler_t::_16ms, true);
}

void loop() {
	switch( command() ) {
	case 1: Test::runall(environment); break;
	case 2: Test::benchmark(environment); break;
	default:
		Serial1.write("\n?\n");
		delay(1000);
	}
}

USBDevice_ USBDevice;
void USBDevice_::attach() {}
USBDevice_::USBDevice_() {}
/*
Program:   42166 bytes (16.1% Full)
Data:       7148 bytes (87.3% Full)
Moving masters to pgmspace:
Program:   42406 bytes (16.2% Full)
Data:       5460 bytes (66.7% Full)
Program:   50346 bytes (19.2% Full)
Data:       6316 bytes (77.1% Full)
omit names
Program:   48810 bytes (18.6% Full)
Data:       5056 bytes (61.7% Full)
+031.cpp,032.cpp
Program:   65038 bytes (24.8% Full)
Data:       7094 bytes (86.6% Full)

Program:   70024 bytes (26.7% Full)
Data:       7540 bytes (92.0% Full)

 */



