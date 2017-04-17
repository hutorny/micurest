/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * cojson_detectors.hpp - helper classes for testing presence of various
 * functions
 *
 * This file is part of COJSON Library. http://hutorny.in.ua/projects/cojson
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
 * You should have received a copy of the GNU General Public License
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#pragma once
#include <type_traits>
#include <limits>
#include <stdint.h>

namespace cojson {
namespace detectors {
/****************************************************************************
 *				  presence detector for dependent features					*
 ****************************************************************************/
template<class>
struct sfinae_true : std::true_type{};

/* snprintf */
template<typename T, typename C>
static auto test_snprintf(int) -> sfinae_true<decltype(
	snprintf(std::declval<C*>(), 0,
		std::declval<const C*>(), std::declval<T>())
)>;
template<typename, typename>
static auto test_snprintf(long) -> std::false_type;
template<class T, typename C>
struct has_snprintf : decltype(test_snprintf<T,C>(0)){};

/* sprintf */
template<typename T, typename C>
static auto test_sprintf(int) -> sfinae_true<decltype(
	sprintf(std::declval<C*>(),
		std::declval<const C*>(), std::declval<T>())
)>;
template<typename, typename>
static auto test_sprintf(long) -> std::false_type;
template<class T, typename C>
struct has_sprintf : decltype(test_sprintf<T,C>(0)){};

/* swprintf */
template<typename T, typename C>
static auto test_swprintf(int) -> sfinae_true<decltype(
			swprintf(std::declval<C*>(), 0,
				std::declval<const C*>(), std::declval<T>())
)>;
template<typename, typename>
static auto test_swprintf(long) -> std::false_type;
template<class T, typename C>
struct has_swprintf : decltype(test_swprintf<T,C>(0)){};

/* exp10 */

template<typename T>
static auto test_exp10(int) -> sfinae_true<decltype(
	exp10(std::declval<T>())
)>;

template<typename>
static auto test_exp10(long) -> std::false_type;

template<class T>
struct has_exp10 : decltype(test_exp10<T>(0)){};

template<typename T>
static auto test_strcmp(int) -> sfinae_true<decltype(
	strcmp(std::declval<T>(),std::declval<T>())
)>;

template<typename>
static auto test_strcmp(long) -> std::false_type;

template<typename T>
struct has_strcmp : decltype(test_strcmp<T>(0)){};


template<class C, typename T>
static auto test_read(int) -> sfinae_true<
	decltype(std::declval<C&>().read(std::declval<T>()))>;
template<class C, typename T>
static auto test_read(long) -> std::false_type;
template<class C, typename T>
struct has_read : decltype(test_read<C,T>(0)){};

template<class C, typename T>
static auto test_write(int) -> sfinae_true<
	decltype(std::declval<C>().write(std::declval<T>()))>;
template<class C, typename T>
static auto test_write(long) -> std::false_type;
template<class C, typename T>
struct has_write : decltype(test_write<C,T>(0)){};

}
namespace details {
/******************************************************************************/
/* writer's helpers															  */

template<typename T>
struct numeric_helper_unsigned {
	/* U - unsigned implementation type, upsized if necessary */
	typedef typename std::conditional<(sizeof(T)<sizeof(unsigned int)),
		unsigned int, T>::type U;
	static constexpr T min = std::numeric_limits<T>::min();
	static constexpr inline bool is_negative(T) noexcept { return false; }
	static constexpr inline U abs(T v) noexcept { return v; }
};
template<typename T>
struct numeric_helper_signed {
	/* U - unsigned implementation type, upsized if necessary */
	typedef typename std::conditional<(sizeof(T)<sizeof(unsigned int)),
		unsigned int,typename std::make_unsigned<T>::type>::type U;
	static constexpr T min = std::numeric_limits<T>::min();
	static constexpr inline bool is_negative(T v) noexcept { return v < 0; }
	static constexpr inline U abs(T v) noexcept { return v >= 0 ? v : -v; }
	/** unsigned type without reference */
	typedef typename std::remove_reference<U>::type V;
};
template<typename T>
struct numeric_helper : std::conditional<std::is_signed<T>::value,
	numeric_helper_signed<T>, numeric_helper_unsigned<T>>::type {
	static_assert(std::is_integral<T>::value, "T is not of integral type");
	static constexpr T max = std::numeric_limits<T>::max();
	static inline constexpr T figure(unsigned long long v) noexcept  {
		return v > static_cast<unsigned long long>(max) / 10ULL ?
			static_cast<T>(v) : figure(v*10ULL);
	}
	/** unsigned type without reference */
	static constexpr T pot = figure(10ULL); /** highest power of ten */
};


template<typename T>
struct digitizer {
public:
	inline digitizer(T val) noexcept : value(val), divider(pot) {}
	bool get(uint_fast8_t& digit, bool erase = false) noexcept {
		if( divider == 0 ) return false;
		digit = (value / divider) % 10;
		if( erase ) value -= digit * divider;
		divider /= 10;
		return true;
	}
	inline operator bool() const noexcept { return value != 0; }
	inline void skip(bool two) noexcept {
		divider /= two ? 100 : 10;
	}
private:
	static constexpr T pot = numeric_helper<T>::pot; /* highest power of ten */
	T value;
	T divider;
};

/**
 * helper for soft dependency on exp10
 */
template<typename T>
T exp_10(short) noexcept;

template<typename T>
class progmem {
public:
	explicit inline constexpr progmem(const T* str) noexcept : ptr(str) {}
	explicit inline constexpr operator const T*() noexcept { return ptr; }
	inline T operator*() const noexcept { return read(ptr); }
	inline T operator[](unsigned i) const noexcept { return read(ptr+i); }
	inline progmem operator++(int) noexcept { return progmem(ptr++); }
	inline constexpr progmem operator+(unsigned off) const noexcept {
		return progmem(ptr+off);
	}
	inline constexpr bool operator !=(std::nullptr_t) const noexcept {
		return ptr != nullptr;
	}
	inline constexpr bool operator !=(progmem that) const noexcept {
		return ptr != that.ptr;
	}
	inline constexpr bool operator ==(std::nullptr_t) const noexcept {
		return ptr == nullptr;
	}
	inline progmem& operator++() noexcept { ++ptr; return *this; }
private:
	static T read(const T *ptr) noexcept;
	const T *ptr;
};

template<typename A, typename B>
bool match(A a, B b) noexcept;

template<>
bool match<const char *, const char*>(const char *, const char*) noexcept;

template<>
bool match<const wchar_t*, const wchar_t*>(
		   const wchar_t*, const wchar_t*) noexcept;
template<>
bool match<const char16_t*, const char16_t*>(
		   const char16_t*, const char16_t*) noexcept;
template<>
bool match<const char32_t*, const char32_t*>(
		   const char32_t*, const char32_t*) noexcept;

template<>
inline bool match<char const*, char*>(char const* a, char* b) noexcept {
	return match<char const*, const char*>(a,b);
}

template<>
bool match<progmem<char>, char const*>(progmem<char> a, char const* b) noexcept;

template<>
inline bool match<progmem<char>,char*>(progmem<char> a, char* b) noexcept {
	return match<progmem<char>, char const*>(a,b);
}

} // namespace details
} //namespace cojson
