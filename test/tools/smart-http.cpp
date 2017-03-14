/*
 * Copyright (C) 2016 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * smart-http.cpp - micurest tests, LinkIt Smart 7688 specific implementation
 *
 * !!! NOT FINISHED
 *
 * This file is part of MICUREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The MICUREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License v2
 * as published by the Free Software Foundation;
 *
 * The MICUREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the MICUREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "Arduino.h"
#include <avr/interrupt.h>
#undef F
#undef bit
#undef abs
#include <string.h>
#include "micurest.hpp"
#include "common.hpp"

using namespace micurest;
namespace smart {
class ostream : public cojson::details::ostream {
	Print& out;
public:
	inline ostream(Print& o) noexcept : out(o) {}
	bool put(char_t c) noexcept {
		return out.write(c) == 1;
	}
};

class istream : public /*micurest::details::lexer, private */ cojson::details::istream {
	Stream& in;
private:
	static const constexpr unsigned long timeout = 5000UL;
	bool get(char_t& c) noexcept {
		unsigned long now = millis();
		while( ! in.available() && (millis() - now) < timeout ) yield();
		int r = -1;
		if( ! in.available() || (r = in.read()) == -1 ) {
			Serial.write("\r\n<<eof>>\r\n");
			cojson::details::istream::error(cojson::details::error_t::eof);
			c = iostate::eos_c;
			return false;
		}
		c = static_cast<char_t>(r);
		Serial.print(c);
		Serial.print(' ');
		Serial.println(r);

		return true;
	}
public:
	inline istream(Stream& i) noexcept
	  :	/*lexer(static_cast<cojson::details::istream&>(*this)), */ in(i) {}
};

namespace name {
ALIAS(index,index.html)
NAME(natural)
NAME(numeric)
NAME(text)
NAME(logical)
NAME(blob)
NAME(dir)
}


const directory& root() noexcept;

using namespace cojson::accessor;
static unsigned natural;
static float numeric;
static bool logical;
char_t text[32];
unsigned char blob[256];
unsigned blob_length = 0;

static cstring  index_html() noexcept {
	return CSTR(
		"<html>\n\t<body>\n"
		"\t<h1>Plain text values</h1>\n"
		"\t</body>\n</html>");
}

static float get_numeric() noexcept { return numeric; }
static void put_numeric(float v) noexcept { numeric = v; }

const directory& root() noexcept {
	return Root<
		E<name::index,
			N<media::text::html, index_html>
		>,
		F<name::natural, pointer<unsigned, &natural>>,
		F<name::numeric, pointer<float, &numeric>>,
		F<name::logical, pointer<bool, &logical>>,
		D<name::dir,
			F<name::natural, unsigned, &natural>,
			F<name::numeric, float, get_numeric, put_numeric>,
			F<name::text, countof(text), text>
		>,
		F<name::blob,&blob_length, sizeof(blob), blob>
	>();
}
}

using namespace smart;

static constexpr int ledPin = 13;
static constexpr int greenLed = 3;

void setup() {
	Serial1.begin(115200);
	Serial1.setTimeout(10000);
	Serial.begin(115200);
	pinMode(ledPin, OUTPUT);
	pinMode(greenLed, OUTPUT);
	digitalWrite(ledPin, HIGH);
	digitalWrite(greenLed, LOW);
	Serial.write("micurest http server for LinkIt Smart 7688 Duo ATmega32U4\r\n");
}

void loop() {
	static smart::istream in(Serial1);
	static smart::ostream out(Serial1);
	static application app(root());
	while( true ) {
		if( Serial1.available() ) {
			digitalWrite(ledPin, LOW);
			digitalWrite(greenLed, LOW);
			if(!!app.service(in, out) )
				digitalWrite(greenLed, HIGH);
			else {
				digitalWrite(ledPin, HIGH);
				Serial1.readStringUntil('\n');
			}
		}
	}
}
