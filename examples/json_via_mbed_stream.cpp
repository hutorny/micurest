/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * json_via_mbed_stream.cpp - example for using cojson with mbed streams
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 *
 * The COJSON Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The COJSON Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "mbed.h"
#include "cojson.hpp"

using namespace cojson;

class jsonw : public details::ostream {
private:
	mbed::Stream & out;
public:
	inline jsonw(mbed::Stream& o) noexcept : out(o) {}
	bool put(char_t c) noexcept {
		if( out.putc(c) != EOF ) return true;
		error(details::error_t::ioerror);
		return false;
	}
};

class jsonr : public details::istream {
	mbed::Stream& in;
private:
	bool get(char_t& c) noexcept {
		int chr = in.getc();
		if(  chr != EOF ) {
			c = chr;
			return true;
		} else {
			istream::error(details::error_t::eof);
			c = iostate::eos_c;
			return false;
		}
	}
public:
	inline jsonr(mbed::Stream& i) noexcept :	in(i) {}
};

struct Pdo {
	short data;
	char message[16];
	static mbed::DigitalOut led;

	bool get() const noexcept {
		return led;
	}
	void set(bool v) noexcept {
		led = v;
	}
	struct Name {
		static constexpr const char* led() noexcept { return "led"; }
		static constexpr const char* data() noexcept { return "data"; }
		static constexpr const char* msg() noexcept { return "msg"; }
	};
	static const details::clas<Pdo>& json() noexcept {

		return
			O<Pdo,
				P<Pdo,Name::msg, sizeof(message), &Pdo::message>,
				P<Pdo,Name::data, short, &Pdo::data>,
				P<Pdo, Name::led,
					accessor::methods<Pdo, bool, &Pdo::get, &Pdo::set>>
			>();
	}
	using error_t = cojson::details::error_t;
	error_t read(mbed::Stream& in) {
		jsonr text(in);
		details::lexer lexer(text);
		json().read(*this, lexer);
		return text.error();
	}
	error_t write(mbed::Stream& out) {
		jsonw text(out);
		json().write(*this, text);
		return text.error();
	}

};
mbed::DigitalOut Pdo::led(LED_RED);
static Pdo pdo;

void json_read_write() {
	Serial serial(STDIO_UART_TX, STDIO_UART_RX);
	pdo.write(serial);
	pdo.read(serial);
}

