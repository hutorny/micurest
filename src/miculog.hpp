/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * miculog.hpp - simple logging facilities
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

#ifndef LOG_HPP_
#define LOG_HPP_

namespace miculog {

enum class level {
	trace,
	debug,
	info,
	warn,
	error,
	fail,
	none
};

struct log_impl {
	static void vmsg(level lvl, const char* fmt, va_list args);
};

template<level L=level::error>
struct log : log_impl {
	inline static void msg(miculog::level lvl, const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 2, 3)));
	inline static void trace(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	inline static void debug(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	inline static void info(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	inline static void warn(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	template<typename T>
	inline static void warn_if(T test, const char* fmt, ...) noexcept {
		if( level::warn >= L && test) {
			va_list args;
			va_start(args, fmt);
			vmsg(level::info, fmt, args);
			va_end(args);
		}
	}
	inline static void error(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	template<typename T>
	inline static void err_if(T test, const char* fmt, ...) noexcept {
		if( level::error >= L && test ) {
			va_list args;
			va_start(args, fmt);
			vmsg(level::info, fmt, args);
			va_end(args);
		}
	}
	inline static void fail(const char* fmt, ...) noexcept
										__attribute__ ((format (printf, 1, 2)));
	template<typename T>
	inline static void fail_if(T test, const char* fmt, ...) noexcept {
		if( level::fail >= L && test ) {
			va_list args;
			va_start(args, fmt);
			vmsg(level::info, fmt, args);
			va_end(args);
		}
	}
	static constexpr level lvl = L;
};

template<level L>
inline void log<L>::msg(miculog::level lvl, const char* fmt, ...) noexcept {
	if( lvl >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(lvl, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::trace(const char* fmt, ...) noexcept {
	if( level::trace >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::trace, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::debug(const char* fmt, ...) noexcept {
	if( level::debug >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::debug, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::info(const char* fmt, ...) noexcept {
	if( level::info >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::info, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::warn(const char* fmt, ...) noexcept {
	if( level::warn >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::warn, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::error(const char* fmt, ...) noexcept {
	if( level::error >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::error, fmt, args);
		va_end(args);
	}
}

template<level L>
inline void log<L>::fail(const char* fmt, ...) noexcept {
	if( level::fail >= L ) {
		va_list args;
		va_start(args, fmt);
		vmsg(level::fail, fmt, args);
		va_end(args);
	}
}


}
#endif /* USER_LOG_HPP_ */
