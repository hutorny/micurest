/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurest_arduino.h - the primary include for Arduino platform
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
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#ifndef MICUREST_ARDUINO_H_
#define MICUREST_ARDUINO_H_
#ifndef	NETWORK_ARDUINO_HPP_
#	include "network_arduino.hpp"
#endif
#ifndef COJSON_HELPERS_HPP_
#	include "cojson_helpers.hpp"
#endif

namespace micurest {
using cojson::details::progmem;
}
/*
 * Macro for literal definitions
 */
#define NAME(s) static inline progmem<char> s() noexcept { 			\
	static const char l[] __attribute__((progmem)) = #s; 			\
	return progmem<char>(l);}
#define ALIAS(f,s) static inline progmem<char> f() noexcept {		\
	static const char l[] __attribute__((progmem)) = #s; 			\
	return progmem<char>(l); }
#define ENUM(s) constexpr const char* s() noexcept {return #s;}



#endif /* USER_UTILS_HPP_ */
