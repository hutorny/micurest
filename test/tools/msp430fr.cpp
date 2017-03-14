/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * msp430fr.cpp - cojson tests, MSP430 specific implementation (not finished)
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
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include "common.hpp"
#include "msp430.hpp"

#ifndef COJSON_TEST_BUFFER_SIZE
#	define COJSON_TEST_BUFFER_SIZE (256)
#endif

using namespace msp430;
constexpr uart tty(msp430::uart::channel_t::esci_a0);
constexpr uart aux(msp430::uart::channel_t::esci_a1);
using led_red = gpio::port<gpio::port_t::p1>::pin<0>;
using led_grn = gpio::port<gpio::port_t::p9>::pin<7>;

namespace cojson {


namespace test {

class ostream : public details::ostream {
public:
	inline ostream(const uart& dev) : device(dev) {}
	bool put(char_t c) noexcept {
		return device.put(c);
	}
	void put(const char* str) {
		if( str != nullptr )
			while(*str) device.put(*str++);
	}
	void putf(const char * fmt, va_list arg) noexcept {
		while( *fmt ) {
			if( *fmt == '%' ) {
				putarg(++fmt,arg);
			} else {
				device.put(*fmt);
			}
			++fmt;
		}
	}
	template<typename T>
	inline ostream& operator<<(const T& val) noexcept {
		putv(val);
		return *this;
	}

private:
	template<typename T>
	inline void putv(const T& val) noexcept {
		details::writer<T>::write(val,*this);
	}
	inline void putarg(char const *& fmt, va_list& arg) noexcept;
	void ashex(unsigned long val) {
		unsigned i = 8;
		bool was = false;
		char d1, d0;
		while(i) {
			d1 = hexdigit(val,--i);
			d0 = hexdigit(val,--i);
			if( was || i == 0 || (was = (d1 != '0' && d0 != '0')) ) {
				device.put(d1);
				device.put(d0);
			}
		}
	}
	inline char hexdigit(uint16_t val, uint8_t pos) noexcept {
		val >>= (pos*4);
		val &= 0xF;
		return val < 0xA ? val+'0' : val + 'A' - 0xA;
	}
	const uart& device;
};

class istream : public details::istream {
public:
	inline istream(const uart& dev) : device(dev) {}
	bool get(char_t& c) noexcept {
		return device.get(c, device.blocking_t::blocking);
	}
	bool peek(char_t& c) noexcept {
		return device.get(c, device.blocking_t::non_blocking);
	}
private:
	const uart& device;
};


static ostream cout(tty);
static istream cin(tty);
static ostream cerr(aux);

void ostream::putarg(const char *& fmt, va_list& arg) noexcept {
	bool islong = false;
	while(true) {
		switch( *fmt ) {
		//implements only specifiers in use
		case '%': device.put('%'); return;
		case 'd':
			if(islong) putv(va_arg(arg,long));
			else putv(va_arg(arg,int));
			return;
		case 'u':
			if(islong) putv(va_arg(arg,unsigned long));
			putv(va_arg(arg,unsigned int)); return;
		case 'g':
			putv(va_arg(arg,double)); return;
		case 'x':
		case 'X':
			if( islong ) ashex(va_arg(arg,unsigned long));
			else ashex(va_arg(arg,unsigned));
			return;
		case 's': put(va_arg(arg,const char*)); return;
		case 'l': islong = true; ++fmt; break;
		case '.':
		case '8':
		case '3': ++fmt; break;
		default:
			device.put('%');
			device.put(*fmt);
			va_arg(arg,void*);
			return;
		}
	}
}


struct DefaultEnvironment : Environment {
	mutable long usec_start = 0;
	inline DefaultEnvironment() noexcept {
		setverbose(normal);
		setoutlvl(nothing);
	}

	void master(const char* file, int index) const noexcept {}

	void startclock() const noexcept {
//		usec_start = micros();
	}
	long elapsed() const noexcept {
//		return micros() - usec_start;
		return 0;
	}

	bool write(char b) const noexcept {
		if( b ) cout.put(b);
		return b;
	}
	mutable char buff[128];
	void msg(verbosity lvl, const char *fmt, ...) const noexcept  {
		if( lvl > options.level ) return;
		va_list args;
		va_start(args, fmt);
		cout.putf(fmt, args);
		va_end(args);
	}

	void msgc(verbosity lvl, cstring master) const noexcept {
		if( lvl > options.level ) return;
		cout.put((const char_t*)master);
		cout.put('\n');
	}

	void msgt(verbosity lvl, tstring master) const noexcept {
		if( lvl > options.level ) return;
		cout.put((const char_t*)master);
		cout.put('\n');
	}

	void out(bool success, const char *fmt, ...) const noexcept {
		if( noout(success, false) ) return;
		va_list args;
		va_start(args, fmt);
		cout.putf(fmt, args);
		va_end(args);
	}
	const char* shortname(const char* filename) const noexcept {
		return filename;
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
	cout.put(help);
}

static int command() {
	int value = 0;
	bool has = false;
	unsigned short led_cnt = 0;
	unsigned short chr_cnt = 0;
	char chr;
	while( ! cin.peek(chr) ) {
		if( led_cnt++ & 0x1000 )
			led_red::tgl();
		cout.put(led_cnt & 0x1000 ? "\r:" : "\r.");
	}
	while(true) {
		led_grn::set();
		chr_cnt = led_cnt += 0x1000;
		cout.put(chr);
		switch( chr ) {
		default: cout.put("\r?"); break;
		case '?': print_help(); break;
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
				cout.put('\n');
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
		while( ! cin.peek(chr) ) {
			if( chr_cnt == led_cnt ) led_grn::clr();
			if( led_cnt++ & 0x1000 ) {
				led_red::tgl();
			}
		}
	}
	return 1;
}

static inline void init_gpio() {
    gpio::port<gpio::port_t::p1>::init();
    gpio::port<gpio::port_t::p2>::init();
    gpio::port<gpio::port_t::p3>::init();
    gpio::port<gpio::port_t::p4>::init();
    gpio::port<gpio::port_t::p5>::init();
    gpio::port<gpio::port_t::p6>::init();
    gpio::port<gpio::port_t::p7>::init();
    gpio::port<gpio::port_t::p8>::init();
    gpio::port<gpio::port_t::p9>::init();
    /* set function for LED pins */
    led_red::sel();
    led_grn::sel();
    /* Bit 4 Reserved Reserved. Must be written as 1. */
    SFRRPCR = 0x10 |
    		SYSRSTRE | SYSRSTUP; /* no NMI, pullup */

}
static inline void init_clock() {
    // Set DCO frequency to default 8MHz
	clocks::dco::setup(clocks::dco::frequency_t::_8MHz);
    // Configure MCLK and SMCLK to default 8MHz
	clocks::mclk::init<clocks::dco>(clocks::divider_t::_1);
	clocks::smclk::init<clocks::dco>(clocks::divider_t::_1);
}


void setup() {
    WDTCTL = WDTPW | WDTHOLD | WDTCNTCL | WDTSSEL__VLO;	// Stop watchdog timer
	init_gpio();
	pmm::unlockLPM5();
    init_clock();
    tty.begin(tty.baudrate_t::_115200);
    aux.begin(aux.baudrate_t::_115200);
    tty.put("\ncojson test runner\n");
}

void loop() {
	switch( command() ) {
	case 1:
		Test::runall(environment); break;
	case 2:
		Test::benchmark(environment); break;
	default:
		cout.put("\n?\n");
		//delay(1000);
	}

}

void dbg(const char *fmt, ...) noexcept	__attribute__ ((format (printf, 1, 2)));
void dbg(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	cerr.putf(fmt, args);
	va_end(args);
}


int main(void) {
	setup();
	while(true) loop();
	return 0;
}

