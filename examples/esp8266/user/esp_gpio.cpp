/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * esp_gpio.cpp - Example of  µcuREST API for ESP SDK
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "esp8266.hpp"
#include "io.hpp"
extern "C" {
#	include "user.h"
}

using cojson::size_t;
using micurest::D;
using micurest::N;
using micurest::E;
using cojson::V;
using cojson::P;
using cojson::O;
using cojson::details::countof;
using micurest::resource::node;
/*****************************************************************************/

namespace espconfig {
using namespace io;

static constexpr const port_t ports[] = {0};
/* gpio6-gpio11 are used for SD card and not exposed in this example */
static constexpr const gpio_t gpios[] = {  0,  1,  2,  3,  4,  5, 12, 13, 14, 15};
static constexpr const pin_t  pads[] =  { 15, 26, 14, 25, 16, 24, 10, 12,  9, 13};

static constexpr const uint32 gpio_mux_map[GPIO_PIN_COUNT] = {
	PERIPHS_IO_MUX_GPIO0_U,
	PERIPHS_IO_MUX_U0TXD_U,
	PERIPHS_IO_MUX_GPIO2_U,
	PERIPHS_IO_MUX_U0RXD_U,
	PERIPHS_IO_MUX_GPIO4_U,
	PERIPHS_IO_MUX_GPIO5_U,
	PERIPHS_IO_MUX_SD_CLK_U,
	PERIPHS_IO_MUX_SD_DATA0_U,
	PERIPHS_IO_MUX_SD_DATA1_U,
	PERIPHS_IO_MUX_SD_DATA2_U,
	PERIPHS_IO_MUX_SD_DATA3_U,
	PERIPHS_IO_MUX_SD_CMD_U,
	PERIPHS_IO_MUX_MTDI_U,
	PERIPHS_IO_MUX_MTCK_U,
	PERIPHS_IO_MUX_MTMS_U,
	PERIPHS_IO_MUX_MTDO_U,
};

static constexpr const unsigned char gpio_func_map[GPIO_PIN_COUNT] = {
	FUNC_GPIO0,
	FUNC_GPIO1,
	FUNC_GPIO2,
	FUNC_GPIO3,
	FUNC_GPIO4,
	FUNC_GPIO5,
	0xFF, //	FUNC_GPIO6,
	0xFF, //	FUNC_GPIO7,
	0xFF, //	FUNC_GPIO8,
	FUNC_GPIO9,
	FUNC_GPIO10,
	0xFF, //	FUNC_GPIO11,
	FUNC_GPIO12,
	FUNC_GPIO13,
	FUNC_GPIO14,
	FUNC_GPIO15
};


static inline bool find(const unsigned char *l, size_t n, unsigned char& v) noexcept {
	while(n--) if( l[n]==v ) {
		v = n;
		return true;
	}
	return false;
}
template<typename T, size_t N>
static inline bool have(const T (&l)[N], T v) noexcept  { return find(l,N,v); }

template<typename T, size_t N>
static inline bool find(const T (&l)[N], T& v) noexcept  { return find(l,N,v); }

using cojson::details::ostream;
using cojson::details::lexer;
using micurest::accessor::vector;
using micurest::accessor::bunch;

struct gpio {
	mode_t mode;
//	func_t func; /* decided not to cope with pin functions in this example */
	static gpio get(size_t n) noexcept {
		gpio local;
		if( is_gpio(n) ) {
			if( is_out(n) )
				local.mode = mode_t::out;
			else {
				if( is_pullup(n) )
					local.mode = mode_t::pullup;
				else
					local.mode = mode_t::in;
			}
		} else
			local.mode = mode_t::na;
		return local;
	}
	static void set(size_t n, gpio val) noexcept {
		switch(val.mode) {
		case io::mode_t::na: 	as_other(n); return;
		case io::mode_t::in:	as_gpio(n); in(n); no_pullup(n); return;
		case io::mode_t::pullup:as_gpio(n); in(n); pullup(n); return;
		case io::mode_t::out: 	as_gpio(n); out(n); return;
		default:;
		}
	}
	static bool has(size_t n) noexcept { return have(gpios, static_cast<gpio_t>(n)); }
	typedef vector<gpio, get, set, has> X;
	static const clas<gpio>& S() noexcept {
		return O<gpio,
			P<gpio, io::name::mode, decltype(gpio::mode), &gpio::mode>
		>();
	}

	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}
	static inline bool is_pullup(gpio_t n) noexcept {
		return READ_PERI_REG(gpio_mux_map[n]) & PERIPHS_IO_MUX_PULLUP;
	}

	static inline void pullup(gpio_t n) noexcept {
		WRITE_PERI_REG(gpio_mux_map[n],
			READ_PERI_REG(gpio_mux_map[n]) | PERIPHS_IO_MUX_PULLUP);
	}

	static inline void no_pullup(gpio_t n) noexcept {
		WRITE_PERI_REG(gpio_mux_map[n],
			READ_PERI_REG(gpio_mux_map[n]) & ~PERIPHS_IO_MUX_PULLUP);
	}

	static inline constexpr uint32 funcmask(uint32 func) noexcept {
		return
           ((((func&BIT2)<<2)|(func&0x3))<<PERIPHS_IO_MUX_FUNC_S);
	}

	static inline bool is_gpio(gpio_t n) noexcept {
		return (READ_PERI_REG(gpio_mux_map[n]) & funcmask(0x7)) == funcmask(gpio_func_map[n]);
	}

	static inline bool is_out(gpio_t n) noexcept {
		return GPIO_REG_READ(GPIO_ENABLE_ADDRESS) & (1<<n);
	}
	static inline void out(gpio_t n) noexcept {
		GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
				GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1<<n));
	}
	static inline void in(gpio_t n) noexcept {
		GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS,
				GPIO_REG_READ(GPIO_ENABLE_ADDRESS) & ~(1<<n));
	}

	static inline void as_gpio(gpio_t n) noexcept {
		PIN_FUNC_SELECT(gpio_mux_map[n], gpio_func_map[n]);
	}
	static inline void as_other(gpio_t n) noexcept {
		PIN_FUNC_SELECT(gpio_mux_map[n], 1);
	}
};

struct allgpio : gpio {
	pin_t gpio_;
	static allgpio get(size_t n) noexcept {
		allgpio l;
		l.mode = gpio::get(gpios[n]).mode;
		l.gpio_ = gpios[n];
		return l;
	}
	inline mode_t getmode() const noexcept {
		return mode;
	}
	static bool has(size_t n) noexcept { return n < countof(gpios); }
	inline void setmode(mode_t m) noexcept {
		mode = m;
	}
	static const value& A() noexcept {
		return V<bunch<allgpio, get, has>>();
	}
	typedef cojson::accessor::methods<allgpio,
		decltype(allgpio::mode), &allgpio::getmode, &allgpio::setmode> methods;
	static const clas<allgpio>& S() noexcept {
		return O<allgpio,
			P<allgpio, io::name::mode, methods>,
			P<allgpio, io::name::gpio,decltype(allgpio::gpio_),&allgpio::gpio_>
		>();
	}

	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}
};

struct pad : gpio {
	static gpio get(size_t n) noexcept {
		unsigned char i = n;
		if( find(espconfig::pads, i) )
			return gpio::get(gpios[i]);
		return gpio{};
	}
	static void set(size_t n, gpio val) noexcept {
		unsigned char i = n;
		if( find(espconfig::pads, i) )
			gpio::set(gpios[i], val);
	}
	static bool has(size_t n) noexcept { return have(pads, static_cast<gpio_t>(n)); }
	typedef vector<gpio, get, set, has> X;
};

struct port {
	uint32 enable;

	static inline port get() noexcept {
		return port { GPIO_REG_READ(GPIO_ENABLE_ADDRESS) };
	}
	static inline void as_gpio(uint32 mask) noexcept {
	for(gpio_t i=0; i < GPIO_PIN_COUNT; ++i, mask <<= 1)
		if( have(gpios, i) && (mask & 1) )
			gpio::as_gpio(i);
	}

	inline void set() noexcept {
		GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, enable);
	}

	static port get(size_t num) noexcept {
		return num == 0 ? get() : port{};
	}
	static bool has(size_t n) noexcept { return n == 0; }

	static void set(size_t num, port v) noexcept {
		if( num == 0 ) v.set();
	}
	static const clas<port>& S() noexcept {
		return O<port,
			P<port, io::name::enable,  decltype(port::enable),  &port::enable>
		>();
	}
	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}

	typedef vector<port, get, set, has> X;
};
}

namespace espgpio {

static inline io::bit_t gpio_get(size_t n) noexcept {
	return GPIO_REG_READ(GPIO_IN_ADDRESS) & (1<<n) ? 1:0;
}

static inline void gpio_set(size_t n, io::bit_t v) noexcept {
	GPIO_REG_WRITE((v?GPIO_OUT_W1TS_ADDRESS:GPIO_OUT_W1TC_ADDRESS), 1<<n);
}

static inline io::bit_t pad_get(size_t n) noexcept {
	io::pin_t p = n;
	if( ! espconfig::find(espconfig::pads, p) ) return 0;
	return gpio_get(espconfig::gpios[p]);
}

static void pad_set(size_t n, io::bit_t v) noexcept {
	io::pin_t p = n;
	if( ! espconfig::find(espconfig::pads, p) ) return;
	n = 1<<p;
	gpio_set(espconfig::gpios[p], v);
}

static uint32 port_get(size_t p) noexcept {
	return p == 0 ? gpio_input_get() : 0;
}

static void port_set(size_t p, uint32 v) noexcept {
	if( p != 0 ) return;
	GPIO_REG_WRITE(GPIO_OUT_ADDRESS, v);
}

using ostream=cojson::details::ostream;
using value=cojson::details::value;
using lexer=cojson::details::lexer;
using micurest::accessor::vector;
using micurest::accessor::bunch;


struct allgpio  {
	io::bit_t in;
	io::pin_t gpio_;
	io::bit_t enabled;
	static allgpio get(size_t n) noexcept {
		allgpio l;
		l.in = gpio_get(n);
		l.enabled = espconfig::port::get().enable & (1<<n) ? 1 : 0;
		l.gpio_ = espconfig::gpios[n];
		return l;
	}
	static const value& A() noexcept {
		return V<bunch<allgpio, get, espconfig::allgpio::has>>();
	}
	static const clas<allgpio>& S() noexcept {
		return O<allgpio,
			P<allgpio, io::name::in,decltype(allgpio::in),&allgpio::in>,
			P<allgpio, io::name::enable,decltype(allgpio::enabled),&allgpio::enabled>,
			P<allgpio, io::name::gpio,decltype(allgpio::gpio_),&allgpio::gpio_>
		>();
	}
	inline bool write(ostream& out) const noexcept {
		return S().write(*this, out);
	}
	inline bool read(lexer& in) noexcept {
		return S().read(*this, in);
	}

};

}

struct meta_json {
	static constexpr const char* body() noexcept { return _0; }
private:
	static constexpr const char _0[] = {
// tools/json-minify user/meta.json | file2c > user/meta.json.inc
#		include "meta.json.inc"
		,0
	};
};

constexpr const char meta_json::_0[];
static constexpr auto mtu_size = 1400;
//TODO use computed sizeof(index_html)
static constexpr unsigned html_size = 509; // sizeof(index_html)
static_assert(sizeof(meta_json) + html_size + 44 < mtu_size,
		"Data size exceeds MTU pay load");

using micurest::identity::numeric;

const entry& esp_config_gpio() noexcept {
	return
		D<name::gpio,
			E<name::asterisk, N<espconfig::allgpio::A>>,/// /config/gpio/*
			E<numeric, N<espconfig::gpio::X>>			/// /config/gpio/0 ..15
		>();
}

const entry& esp_config_port()  noexcept {
	return
		D<name::port,
			E<numeric, N<espconfig::port::X>>
		>();
}
const entry& esp_config_pad()  noexcept {
	return
		D<name::pad,
			E<numeric, N<espconfig::pad::X>>
		>();
}

const node& esp_gpio_metadata() noexcept {
	return N<micurest::media::json, meta_json::body>();
}


const node& esp_config_modenames() noexcept {
	return N<Enums<espconfig::mode_names>>();
}
const entry& esp_gpio_meta() noexcept {
	return
		D<name::meta,
			E<name::asterisk, esp_gpio_metadata>,
			E<name::mode, esp_config_modenames>
		>();
}

const entry& esp_gpio_all() noexcept {
	return E<name::asterisk, N<espgpio::allgpio::A>>();
}

using micurest::accessor::vector;
using micurest::accessor::bunch;

const entry& esp_gpio_entry() noexcept {
	return
		E<numeric,											/// /gpio/0..15
			N<vector<io::bit_t, espgpio::gpio_get, espgpio::gpio_set,
				espconfig::gpio::has>>
		>();
}

const entry& esp_pad_entry() noexcept {
	return
		D<name::pad,
			E<numeric,										/// /pad/<N>
				N<vector<io::bit_t, espgpio::pad_get,
					espgpio::pad_set, espconfig::pad::has>>
			>
		>();
}

const entry& esp_port_entry() noexcept {
	return
		D<name::port,
			E<numeric,										/// /port/0
				N<vector<uint32, espgpio::port_get,
					espgpio::port_set, espconfig::port::has>>
			>
		>();
}

namespace enumnames {
	bool match(const char* a, const char* b) noexcept {
		return cojson::details::match(a,b);
	}
}

