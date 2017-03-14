/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * io.hpp - header file for esp8266 example
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

#ifndef IO_HPP_
#define IO_HPP_
#include "micurest.hpp"
#include "enumnames.hpp"
#ifndef NAME
#	define NAME(s) constexpr const char* s() noexcept {return #s;}
#endif

namespace io {
using cojson::char_t;
using cojson::details::value;
using cojson::details::clas;

typedef unsigned char port_t;
typedef unsigned char pin_t;
typedef unsigned char gpio_t;
typedef unsigned char bit_t;

enum class func_t : unsigned char {
	gpio,
	hspi,
	uart,
	pwm,
	ir,
	special,
	__unknown__
};

enum class mode_t : unsigned char {
	na, /* N/A if a pin is controlled by peripheral */
	in,
	out,
	pulldown,
	pullup,
	opendrain,
	highdrive,
	__unknown__
};

namespace name {
	NAME(gpio)
	NAME(hspi)
	NAME(uart)
	NAME(pwm)
	NAME(ir)
	NAME(in)
//	NAME(special)
	NAME(out)
	NAME(na)
	NAME(pulldn)
	NAME(pullup)
	NAME(opendr)
	NAME(highdr)
	NAME(mode)
	NAME(enable)
}
/*
template<mode_t K, enumnames::name V>
struct _ : enumnames::tuple<mode_t, K,V> {};

template<func_t K, enumnames::name V>
struct __ : enumnames::tuple<func_t, K,V> {};

struct mode_names : enumnames::names<mode_t,
	_<mode_t::na,			name::na >,
	_<mode_t::in,			name::in >,
	_<mode_t::out,			name::out>,
	_<mode_t::pulldown,		name::pulldn>,
	_<mode_t::pullup,		name::pullup>,
	_<mode_t::opendrain,	name::opendr>,
	_<mode_t::highdrive,	name::highdr>,
	_<mode_t::__unknown__,	nullptr  >> {
};

struct func_names : enumnames::names<func_t,
	__<func_t::gpio,		name::gpio>,
	__<func_t::hspi,		name::hspi>,
	__<func_t::uart,		name::uart>,
	__<func_t::pwm,			name::pwm>,
	__<func_t::ir,			name::ir>,
	__<func_t::other,		name::other>,
	__<func_t::__unknown__,	nullptr  >> {
};
*/
}

#endif /* GPIO_HPP_ */
