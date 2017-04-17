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

#include "miculog.hpp"
#include <cstdio>
#include <cstdarg>
#include "logging.h"

namespace miculog {
inline constexpr unsigned char operator+(level lvl) noexcept {
	return static_cast<unsigned char>(lvl);
}
namespace details {

static const LogLevel level_map[+level::none]  = {
	LogLevel::TRACE_LEVEL,
	LogLevel::DEBUG_LEVEL,
	LogLevel::INFO_LEVEL,
	LogLevel::WARN_LEVEL,
	LogLevel::ERROR_LEVEL,
	LogLevel::PANIC_LEVEL
};

void default_appender::log(level lvl, const char* fmt, ...) noexcept {
	char buf[128];
	LogLevel loglevel = LogLevel::LOG_LEVEL_NONE;
	if( lvl < level::none )
		loglevel = level_map[+lvl];
	va_list args;
	va_start(args, fmt);
	log_printf_v(loglevel,"micurest", nullptr, fmt, args);
	va_end(args);
}

}}
