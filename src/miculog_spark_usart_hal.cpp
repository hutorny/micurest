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

#include <configuration.h>
#include "miculog_spark_usart_hal.ccs"
#include <cstdio>
#include <cstdarg>
#include "miculog.hpp"
#include <usart_hal.h>
#include <usb_hal.h>

namespace miculog {
inline constexpr unsigned char operator+(level lvl) noexcept {
	return static_cast<unsigned char>(lvl);
}
namespace details {

static const char  names[1 + +level::none][9] = {
	"!TRACE: ",
	"!DEBUG: ",
	"!INFO : ",
	"!WARN : ",
	"!ERROR: ",
	"!FAIL : ",
	"!?NONE: "
};

using config = configuration::Configuration<default_appender>;

inline void usart_write(const char* str) noexcept {
	while(*str)
		if( HAL_USB_USART_Serial(config::serial) == HAL_USB_USART_SERIAL ) {
			if( config::blocking || HAL_USB_USART_Available_Data_For_Write(
					HAL_USB_USART_Serial(config::serial)) > 0 )
				HAL_USB_USART_Send_Data(
						HAL_USB_USART_Serial(config::serial), *str++);
		} else {
			if( config::blocking || HAL_USART_Available_Data_For_Write(
					HAL_USART_Serial(config::serial-1)) > 0 )
				HAL_USART_Write_Data(
						HAL_USART_Serial(config::serial-1), *str++);
		}
}

void default_appender::log(level lvl, const char* fmt, ...) noexcept {
	char buff[config::buffer_size] = {};
	if( lvl <= level::none )
		usart_write(names[+lvl]);
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, sizeof(buff), fmt, args);
	va_end(args);
	usart_write(buff);
}

}}
