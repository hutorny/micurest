/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * access_log.hpp - basic access logging
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
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
#pragma once
#include "miculog.hpp"

namespace micurest {
namespace network {
/***************************************************************************/
struct access_log : miculog::Log<access_log> {
	using level = miculog::level;
	static inline constexpr bool enabled() noexcept {
		return miculog::Log<access_log>::enabled(level::info); }
	template<class Addr>
	inline static void log(const char* label, const Addr& addr) noexcept {
		if( enabled() )
			_log(label, addr, '\n');
	}
	template<class Addr, typename ... Args>
	inline static void log(const char* label, const Addr& addr,
			const char* fmt, Args ... args) noexcept {
		if( enabled() ) {
			_log(label, addr, ' ');
			appender::log(static_cast<miculog::level>(-1), fmt, args...);
		}
	}
private:
	template<class Addr>
	static void _log(const char* method, const Addr& addr, char) noexcept;
};
}}
