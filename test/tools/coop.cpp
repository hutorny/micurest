/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * coop.cpp - cojson tests, implementation of command line options lib
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

#include "coop.hpp"
#include "stdlib.h"

namespace coop {
namespace details {

const char* argument::prefixes[2] = { "-", "--" };
const char* argument::separator = "=";
static size_t prefix_lengths[2] = { 1, 2 };
static size_t separator_length = 1;

void helper::validate_prefixes() noexcept {
	if( argument::prefixes[0] == nullptr ) argument::prefixes[0] = "";
	if( argument::prefixes[1] == nullptr ) argument::prefixes[1] = "";
	if( argument::separator == nullptr ) argument::separator = "";
	prefix_lengths[0] = strlen(argument::prefixes[0]);
	prefix_lengths[1] = strlen(argument::prefixes[1]);
	separator_length = strlen(argument::separator);
}


bool helper::match_prefix(const char * str, bool full) noexcept {
	return 0 != prefix_lengths[full ? 1 : 0] && 0 == strncmp(str,
		argument::prefixes[full ? 1 : 0], prefix_lengths[full ? 1 : 0]);
}

bool helper::is_prefixed(const char * str, bool& full) noexcept {
	bool shrt = match_prefix(str,false);
	full = match_prefix(str,true);
	return shrt || full;
}

size_t argument::match(const char* name, const char* arg, bool sep, bool full)
	noexcept {
	size_t off = prefix_lengths[full ? 1 : 0];
	size_t len = strlen(name);

	if( strncmp(arg+off, name, len) ) return 0;

	off += len;

	if( sep && separator_length ) {
		if( strncmp(arg+off, argument::separator, separator_length) )
			return 0;
		else
			off += separator_length;
	} else {
		if( arg[off] ) return 0; /* input is longer than the option */
	}
	return off;
}

/** finds s among l, last element of l must be null 			*/
int string_find(const char* l[], const char *s) noexcept {
	int i = 0;
	while( *l ) {
		if( strcmp(*l,s) == 0 ) return i;
		++l;
		++i;
	}
	return -1;
}

template<>
bool string_to<bool>(bool& dst, const char* src) noexcept {
	static const char * yes[] = { "yes", "true", "1", nullptr };
	static const char * no[]  = { "no", "false", "0", nullptr };
	bool y = string_find(yes, src) >= 0;
	bool n = string_find(no, src) >= 0;
	dst = y || ! n;
	return y || n;
}

template<>
bool string_to<long>(long& dst, const char* src) noexcept {
	char* ep = nullptr;
	long tmp = strtol(src, &ep, 0);
	bool ok = (ep != src) && (ep != nullptr) && (*ep == 0);
	if( ok ) dst = tmp;
	return ok;
}

summary helper::run(int argc, const char** argv,
		const argument& (*items)(int), int len) noexcept {
	summary result { nullptr, nullptr, 0, 0 };

	validate_prefixes();
	int i, u = -1;
	while(--argc > 0 && *++argv ) {
		bool success = false;
		bool full = false;
		bool prefixed = is_prefixed(*argv, full);
		for(i=0; i < len; i++) {
			if( i == u ) continue; // skip handler of unknown
			const argument& a(items(i));
			if( u == -1 && a.unknown() ) {
				u = i; //remember handler of unknown
				continue;
			}
			if( a.ordinal() == prefixed ) continue;
			if( a.apply(*argv, full) ) {
				++result.count;
				success = true;
				break;
			}
		}
		if( success ) continue;
		if( u == -1 ) {
			/* no handler of unknown detected */
			result.unknown = *argv;
			break;
		}
		else {
			const argument& a(items(u));
			if( ! a.apply(*argv,full) ) {
				/* handler of unknown failed too */
				result.unknown = *argv;
				break;
			}
		}
	}
	short ord = 0;
	for(i=0; i < len; i++) {
		if( i == u ) continue; // skip handler of unknown
		const argument& a(items(i));
		if( a.ordinal() ) ++ord;
		if( a.mandatory() && ! a.present() ) {
			if( a.ordinal() ) {
				if( result.first_missing == 0 )
					result.first_missing = ord;
			} else {
				if( result.missing == nullptr )
					result.missing = a.name(true);
			}
		}
		if( result.first_missing != 0 && result.missing != nullptr )
			break; /* all set, no sense for further iterations */
	}
	return result;
}

}}


