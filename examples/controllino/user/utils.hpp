/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * utils.hpp - definition of utility classes for a µcuREST example
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

#ifndef USER_UTILS_HPP_
#define USER_UTILS_HPP_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "Controllino.h"
#pragma GCC diagnostic pop
#ifndef	MICUREST_ARDUINO_H_
#	include "micurest_arduino.h"
#endif
#ifndef COJSON_HELPERS_HPP_
#	include "cojson_helpers.hpp"
#endif

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

using cojson::O;
using cojson::P;
using cojson::details::clas;
using cojson::details::lexer;
using cojson::details::ostream;
using cojson::details::progmem;
using micurest::status_t;
using micurest::details::message;
using micurest::resource::node;

/* example of a custom stringifier/parser for IPv4 addresses			*/
struct ip_addr {
    uint32_t addr;
	bool read(lexer& in) noexcept;
    bool write(ostream&) const noexcept;
    inline void operator=(uint32_t val) noexcept { addr = val; }
};

/* example of a custom stringifier/parser for MAC addresses				*/
struct mac_addr {
	static constexpr unsigned len = 6;
	uint8_t addr[len];
	bool read(lexer& in) noexcept;
    bool write(ostream&) const noexcept;
};


namespace name {
	NAME(day)
	NAME(weekday)
	NAME(month)
	NAME(year)
	NAME(hour)
	NAME(minute)
	NAME(second)
}

/* example of a custom JSON object - RTC date time							*/
struct datetime {
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char weekday;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;

	/* get is used in template<class C> object defined below				*/
	inline void get() noexcept {
		Controllino_ReadTimeDate(&day, &weekday, &month, &year,
													&hour, &minute, &second);
	}

	/* put is used in template<class C> object defined below				*/
	inline bool put() {
		return !Controllino_SetTimeDate(day, weekday, month, year,
														hour, minute, second);
	}

	/* read is used in template<class C> object defined below				*/
	inline bool read(lexer& in) noexcept {
		return json().read(*this,in);
	}

	/* write is used in template<class C> object defined below				*/
    inline bool write(ostream& out) const noexcept {
		return json().write(*this,out);
    }

    /* this method maps datetime members to properties of a JSON object
     * it is used in read and write above									*/
	static const clas<datetime>& json() noexcept {
		return
			O<datetime,
				P<datetime, name::year, unsigned char, &datetime::year>,
				P<datetime, name::month, unsigned char, &datetime::month>,
				P<datetime, name::day, unsigned char, &datetime::day>,
				P<datetime, name::weekday, unsigned char, &datetime::weekday>,
				P<datetime, name::hour, unsigned char, &datetime::hour>,
				P<datetime, name::minute, unsigned char, &datetime::minute>,
				P<datetime, name::second, unsigned char, &datetime::second>
			>();
	}
};

/* this template illustrates how to implement micurest API with
 * complex objects  														*/
template<class C>
struct object : micurest::details::json {
	void get(message& msg) const noexcept {
		msg.status(status_t::OK);
		C obj = {};
		obj.get();  /* get and write methods are contract demands to C		*/
		obj.write(msg.obody());
	}
	void put(message& msg) const noexcept {
		if( msg.req().content_type == mediatype() ) {
			C obj = {};
			cojson::lexer in(msg.ibody());
			if( obj.read(in) ) {
				if( obj.put() ) /* read and put method are contract demands */
					msg.status(status_t::No_Content);
				else
					msg.status(status_t::Bad_Request);
				return;
			}
		}
		msg.bad_content();
	}
};

/* this function instantiates a template and used in micurest resource maps */
template<class C>
inline const node& Object() noexcept {
	static const object<C> local;
	return local;
}

#endif /* USER_UTILS_HPP_ */
