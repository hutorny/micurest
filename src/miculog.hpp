/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * miculog.hpp - simple logging facilities
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * https://opensource.org/licenses/MIT
 */
/*
 * Simple and configurable logging facilities
 *
 * Concepts
 *	Classes defined here facilitate logging functionality with
 *	compile-time configurable levels and appenders on per-user-class basis.
 *	Logging statement with opted off levels are completely removed from
 *	generated code by the compiler on optimization phase. If logging is
 *	turned off completely, compiler builds application without any appenders
 *	provided
 *
 * Usage
 *  Define a static instance of Log class in the compilation unit where it will
 *  be used:
 *
 *  	static miculog::Log<myclass> log;
 *
 *	Use methods, available in log:
 *
 *		log.error("error code:%d\n", err);
 *
 *  This will emit log statements ending up in default appender, which
 *  calls vprintf for levels error and fail
 *
 *  To change enabled levels, specialize template ClassLogLevels
 *  as the following:
 *
 *  	namespace miculog {
 *		template<> struct ClassLogLevels<myclass> :
 *		Levels<level::error,level::fail> {}; }
 *
 *	To change appender specialize template appender as the following:
 *
 *  	namespace miculog {
 *		template<> struct appender<myclass> {
 *	    	static void log(const char* fmt, ...) noexcept;
 *	    };}
 *
 *	and implement appender<myclass,build::Default>::log
 *
 *	Generally, you may add these template specializations in any place in
 *	your code before the actual instantiation of template Log,
 *	Recommended practice is to include them in your own copy of
 *	configuration.h file, which should be found in the include path
 *	before the one provided with this library
 *
 *	You may also replace default appender with a suitable implementation
 *  simply by removing miculog.cpp from the build and providing your own
 *  implementation elsewhere
 */
#pragma once
#include <configuration.h>
#include "miculog.ccs"
namespace miculog {

template<class C>
struct Log {
	using current = typename configuration::Selector<C>;
	typedef miculog::appender<C,typename current::build> appender;

	static inline constexpr const auto enabled(level lvl) {
		return details::is_set(levels, lvl);
	}
	template<typename ... T>
	inline static void trace(const char* fmt, T ... args) noexcept {
		if( enabled(level::trace) )
			appender::log(level::trace, fmt, args...);
	}
	template<typename ... T>
	inline static void debug(const char* fmt, T ... args) noexcept {
		if( enabled(level::debug) )
			appender::log(level::debug, fmt, args...);
	}
	template<typename ... T>
	inline static void info(const char* fmt, T ... args) noexcept {
		if( enabled(level::info) )
			appender::log(level::info, fmt, args...);
	}
	template<typename ... T>
	inline static void warn(const char* fmt, T ... args) noexcept {
		if( enabled(level::warn) )
			appender::log(level::warn, fmt, args...);
	}
	template<typename ... T>
	inline static void error(const char* fmt, T ... args) noexcept {
		if( enabled(level::error) )
			appender::log(level::error, fmt, args...);
	}
	template<typename ... T>
	inline static void fail(const char* fmt, T ... args) noexcept {
		if( enabled(level::fail) )
			appender::log(level::fail, fmt, args...);
	}
	template<typename Bool, typename ... T>
	inline static void warn_if(Bool cond, const char* fmt, T ... args)
			noexcept {
		if( enabled(level::warn) )
			if (cond)
				appender::log(level::warn, fmt, args...);
	}
	template<typename Bool, typename ... T>
	inline static void error_if(Bool cond, const char* fmt, T ... args)
			noexcept {
		if( enabled(level::error) )
			if( cond )
				appender::log(level::error, fmt, args...);
	}
	template<typename Bool, typename ... T>
	inline static void fail_if(Bool cond, const char* fmt, T ... args)
			noexcept {
		if( enabled(level::fail) )
			if( cond )
				appender::log(level::fail, fmt, args...);
	}
private:
	static constexpr unsigned char levels =
		ClassLogLevels<C,typename current::build>::value;
};
}

