/*
 * Copyright (C) 2015,2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * test.hpp - cojson tests, shared definitions
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

#pragma once
template<>
struct integer_limits_names<progmem<char>> {
	static inline progmem<char> min() {
		static const char s[] __attribute__((progmem)) ="min";
		return progmem<char>(s);
	}
	static inline progmem<char> max() {
		static const char s[] __attribute__((progmem)) ="max";
		return progmem<char>(s);
	}
	static inline progmem<char> pot() {
		static const char s[] __attribute__((progmem)) ="pot";
		return progmem<char>(s);
	}
};

