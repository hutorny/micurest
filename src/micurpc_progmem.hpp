/*
 * micurpc_progmem.hpp
 *
 *  Created on: Apr 14, 2017
 *      Author: eugene
 */

/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurpc.cpp - JSON RPC implementation
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
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
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#pragma once

template<>
struct literal<progmem<char>> :cojson::details::literal {
	typedef progmem<char> pstr;
	static inline pstr code() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "code";
		return pstr(s);
	}
	static inline pstr error() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "error";
		return pstr(s);
	}
	static inline pstr id() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "id";
		return pstr(s);
	}
	static inline pstr jsonrpc() noexcept {
		static constexpr const char s[] __attribute__((progmem))= "jsonrpc";
		return pstr(s);
	}
	static inline pstr message() noexcept {
		static constexpr const char s[] __attribute__((progmem))= "message";
		return pstr(s);
	}
	static inline pstr method() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "method";
		return pstr(s);
	}
	static inline pstr params() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "params";
		return pstr(s);
	}
	static inline pstr result() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "result";
		return pstr(s);
	}
	static inline const pstr _2_0() noexcept {
		static constexpr const char s[] __attribute__((progmem)) = "2.0";
		return pstr(s);
	};
	static inline const pstr id_is_too_long() noexcept {
		static constexpr const char s[] __attribute__((progmem)) =
				"Request is too long";
		return pstr(s);
	}
	static inline const pstr method_is_too_long() noexcept {
		static constexpr const char s[] __attribute__((progmem)) =
				"Method name is too long";
		return pstr(s);
	}
	static inline const pstr string_is_too_long() noexcept {
		static constexpr const char s[] __attribute__((progmem)) =
				"String name is too long";
		return pstr(s);
	}

	template<size_t N>
	static inline void copy(char (&dst)[N], pstr src) noexcept {
		micurest::details::copy(dst,src,N);
	}
};
