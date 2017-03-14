/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * cojson_libdep.cpp - methods' implementations with soft dependencies on libs
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
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

/* any of the includes below could be replaced with an empty stub 			*/
#include <stdio.h>
#include <string.h>
#include <wchar.h>
/* end of 'any of the includes'												*/
#include "cojson.hpp"
#include "cojson_float.hpp"

namespace cojson {
using namespace detectors;

namespace details {

template<typename T, bool has = has_strcmp<T>::value>
struct string_helper {
	static inline bool match(T a, T b) noexcept {
		while( *a && *b && *a == *b ) { ++a; ++b; }
		return *a == *b;
	}
};

template<typename T>
struct string_helper<T,true> {
	static inline bool match(T a, T b) noexcept {
		return strcmp(a, b) == 0;
	}
};

template<>
bool match<const wchar_t*,   const wchar_t*>(
		   const wchar_t* a, const wchar_t* b) noexcept {
	return string_helper<const wchar_t*>::match(a,b);
}

template<>
bool match<const char16_t*,   const char16_t*>(
		   const char16_t* a, const char16_t* b) noexcept {
	return string_helper<const char16_t*>::match(a,b);
}

template<>
bool match<const char32_t*,   const char32_t*>(
		   const char32_t* a, const char32_t* b) noexcept {
	return string_helper<const char32_t*>::match(a,b);
}

template<>
bool match<const char*, const char*>(const char* a, const char* b) noexcept {
	return string_helper<const char*>::match(a,b);
}

template<>
bool match<const wchar_t*, wchar_t*>(const wchar_t * a, wchar_t* b) noexcept {
	return string_helper<const wchar_t*>::match(a,b);
}

template<>
bool match<const char16_t*, char16_t*>(const char16_t * a, char16_t* b) noexcept {
	return string_helper<const char16_t*>::match(a,b);
}

template<>
bool match<const char32_t*, char32_t*>(const char32_t * a, char32_t* b) noexcept {
	return string_helper<const char32_t*>::match(a,b);
}


template<typename C, typename T,
	bool hasswprintf = has_swprintf<T,C>::value,
	bool hassnprintf = has_snprintf<T,C>::value,
	bool hassprintf = has_sprintf<T,C>::value
	>
struct any_printf  {
	static constexpr bool present = hasswprintf || hassnprintf || hassprintf;
	/* no version of printf available */
	static int gfmt(C* dst, size_t s, T val) noexcept;
};

template<typename T, bool a, bool b>
struct any_printf<wchar_t, T, true, a, b> {
	static constexpr bool present = true;
	static inline int gfmt(wchar_t* dst, size_t s, T val) noexcept {
		return swprintf(dst, s, L"%.*g", config::write_double_precision,val);
	}
};
template<typename T, bool a>
struct any_printf<char, T, false, true, a> {
	static constexpr bool present = true;
	static inline int gfmt(char* dst, size_t s, T val) noexcept {
		return snprintf(dst, s, "%.*g", config::write_double_precision, val);
	}
};
template<typename T>
struct any_printf<char, T, false, false, true> {
	static constexpr bool present = true;
	static inline int gfmt(char* dst, size_t s, T val) noexcept {
		return sprintf(dst, "%.*g", config::write_double_precision, val);
	}
};
static constexpr bool with_sprintf =
		config::write_double_impl == config::write_double_impl_is::with_sprintf;
template<typename C,
	bool=with_sprintf && any_printf<C, double>::present,
	bool=with_sprintf && any_printf<char, double>::present>
struct any {
	static inline bool gfmt(C* dst, size_t size, double val) noexcept;
//	static inline bool gfmt(C* dst, size_t size, double val) noexcept {
//		int r = any_printf<C,double>::gfmt(dst, size, val);
//		return r >= 0 && r < (int)size;
//	}
};

template<typename C, bool B>
struct any<C,true,B> {
	static inline bool gfmt(C* dst, size_t size, double val) noexcept {
		int r = any_printf<C,double>::gfmt(dst, size, val);
		return r >= 0 && r < (int)size;
	}
};

template<typename C>
struct any<C,false,true> {
	static inline bool gfmt(C* dst, size_t size, double val) noexcept {
		char* tmp = reinterpret_cast<char*>(dst);
		int r = any_printf<char,double>::gfmt(tmp, size, val);
		if( r < 0 && r >= (int)size ) return false;
		dst[r] = 0;
		while( r-- ) dst[r] = tmp[r];
		return true;
	}
};

template<typename C>
struct any<C,false,false> {
	static bool gfmt(C* dst, size_t size, double val) noexcept;
};


template<typename T, bool has = has_exp10<T>::value>
struct exp10_helper {
	static T calc(short n) noexcept {
		T v = 1.;
		while( n >=  6 ) { v *= 1e+6;  n -= 6; }
		while( n <= -6 ) { v *= 1e-6;  n += 6; }
		switch( n ) {
		case -5: return v * 1e-5;
		case -4: return v * 1e-4;
		case -3: return v * 1e-3;
		case -2: return v * 1e-2;
		case -1: return v * 1e-1;
		case  1: return v * 1e+1;
		case  2: return v * 1e+2;
		case  3: return v * 1e+3;
		case  4: return v * 1e+4;
		case  5: return v * 1e+5;
		}
		return v;
	}
};

template<typename T>
struct exp10_helper<T,true> {
	static inline T calc(short n) noexcept {
		return exp10((T)n);
	}
};

template<>
double exp_10<double>(short n) noexcept {
	return exp10_helper<double>::calc(n);
}

template<>
float exp_10<float>(short n) noexcept {
	return exp10_helper<float>::calc(n);
}
template<config::write_double_impl_is = config::write_double_impl>
static bool write_double_impl(const double& val, ostream& out) noexcept;

template<>
inline bool write_double_impl<config::write_double_impl_is::internal>(
		const double& val, ostream& out) noexcept {
	return floating::serialize<ostream,config::write_double_integral_type>(
			val, out, config::write_double_precision);
}

template<>
inline bool write_double_impl<config::write_double_impl_is::with_sprintf>(
		const double& val, ostream& out) noexcept {
	temporary tmp;
	if( ! any<char_t>::gfmt(tmp.buffer, tmp.size, val) ) {
		out.error(error_t::overrun);
		return false;
	}
	return out.puts(tmp.buffer);
}

bool writer<double>::write(const double& val, ostream& out) noexcept {
		return write_double_impl<>(val,out);
}

}}


