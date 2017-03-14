/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
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

#ifndef TOOLS_TEST_HPP_
#define TOOLS_TEST_HPP_
#include <string.h>
#include "common.hpp"

namespace cojson { namespace test {
template<class C, typename T, T (C::*G)() const noexcept>
struct methods1 {
	typedef C clas;
	typedef T type;
	static constexpr bool canget = true;
	static constexpr bool canset = false;
	static constexpr bool canlref   = false;
	static constexpr bool canrref   = false;
	static constexpr bool is_vector = false;
	static inline constexpr bool has() noexcept { return true; }
	static inline T get(const C& o) noexcept { return (o.*G)(); }
	static T& lref(C&) noexcept;// not possible
	static const T& rref(const C&) noexcept;// not possible
	static void set(C&, T) noexcept;
	static inline void init(T&) noexcept { }
	static inline constexpr bool null() noexcept { return false; }
private:
	methods1();
};

template<class C, typename T, void (C::*S)(T) noexcept>
struct methods0 {
	typedef C clas;
	typedef T type;
	static constexpr bool canget = false;
	static constexpr bool canset = true;
	static constexpr bool canlref   = false;
	static constexpr bool canrref   = false;
	static constexpr bool is_vector = false;
	static inline constexpr bool has() noexcept { return false; }
	static T& lref(C&) noexcept;// not possible
	static const T& rref(const C&) noexcept;// not possible
	static T get(const C&) noexcept;
	static inline void set(C& o, const T& v) noexcept { (o.*S)(v); }
	static inline void init(T&) noexcept { }
	static inline constexpr bool null() noexcept { return false; }
private:
	methods0();
};


/**
 * Accessor for a (static) variable via a pair of functions
 */
template<typename T, T (*G)() noexcept>
struct functions1 {
	typedef void clas;
	typedef T type;
	static constexpr bool canget = true;
	static constexpr bool canset = false;
	static constexpr bool canlref   = false;
	static constexpr bool canrref   = false;
	static constexpr bool is_vector = false;
	static inline constexpr bool has() noexcept { return canget; }
	static T& lref() noexcept;// not possible
	static const T& rref() noexcept;// not possible
	static inline T get() noexcept { return G(); }
	static inline void set(const T&) noexcept {}
	static inline void init(T&) noexcept { }
	static inline constexpr bool null() noexcept { return false; }
private:
	functions1();
};

/**
 * Accessor for a (static) variable via a pair of functions
 */
template<typename T, void (*S)(T) noexcept>
struct functions0 {
	typedef void clas;
	typedef T type;
	static constexpr bool canget = false;
	static constexpr bool canset = true;
	static constexpr bool canlref   = false;
	static constexpr bool canrref   = false;
	static constexpr bool is_vector = false;
	static inline constexpr bool has() noexcept { return canget; }
	static T get() noexcept;
	static T& lref() noexcept;// not possible
	static const T& rref() noexcept;// not possible
	static inline void set(const T& v) noexcept { S(v); }
	static inline void init(T&) noexcept { }
	static inline constexpr bool null() noexcept { return false; }
private:
	functions0();
};

/**
 * member - a virtual variable with getter/setter
 */
template<name id, typename T, T (*G)()noexcept, void (*S)(T)noexcept>
inline const member& M() noexcept {
	return cojson::M<id, accessor::functions<T,G,S>>();
}

/**
 * member - a virtual variable with getter only
 */
template<name id, typename T, T (*G)()noexcept>
inline const member& M() noexcept {
	return cojson::M<id, functions1<T,G>>();
}

/**
 * member - a virtual variable with setter only
 */
template<name id, typename T, void (*S)(T)noexcept>
inline const member& M() noexcept {
	return cojson::M<id, functions0<T,S>>();
}

/**
 * value - a virtual variable with getter
 */
template<typename T, T (*G)() noexcept>
inline const value& V() noexcept {
	return cojson::V<functions1<T,G>>();
}

/**
 * value - a virtual variable with setter
 */
template<typename T, void (*S)(const T) noexcept>
inline const value& V() noexcept {
	return cojson::V<functions0<T,S>>();
}

template<class C, name id, typename T,
	T (C::*G)() const noexcept, void (C::*S)(T) noexcept>
inline const property<C> & Q() noexcept {
	return cojson::P<C,id,accessor::methods<C,T,G,S>>();
}

template<class C, name id, typename T, T (C::*G)() const noexcept>
inline const property<C> & Q() noexcept {
	return cojson::P<C,id,methods1<C,T,G>>();
}

template<class C, name id, typename T, void (C::*S)(T) noexcept>
inline const property<C> & Q() noexcept {
	return cojson::P<C,id,methods0<C,T,S>>();
}

template<typename T>
struct integer_limits_names;

template<>
struct integer_limits_names<const char*> {
	static inline const char* min() { return "min"; }
	static inline const char* max() { return "max"; }
	static inline const char* pot() { return "pot"; }
};

template<>
struct integer_limits_names<const wchar_t*> {
	static inline const wchar_t* min() { return L"min"; }
	static inline const wchar_t* max() { return L"max"; }
	static inline const wchar_t* pot() { return L"pot"; }
};

template<>
struct integer_limits_names<const char16_t*> {
	static inline const char16_t* min() { return u"min"; }
	static inline const char16_t* max() { return u"max"; }
	static inline const char16_t* pot() { return u"pot"; }
};
template<>
struct integer_limits_names<const char32_t*> {
	static inline const char32_t* min() { return U"min"; }
	static inline const char32_t* max() { return U"max"; }
	static inline const char32_t* pot() { return U"pot"; }
};

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


template<typename T>
struct integer_limits : integer_limits_names<cstring> {
	typedef T type;
	static T _min;
	static T _max;
	static T _pot;

	static inline T& minr() { return _min; }
	static inline T* minp() { return &_min; }
	static inline T  ming() { return _min; }

	static inline T& maxr() { return _max; }
	static inline T* maxp() { return &_max; }
	static inline T  maxg() { return _max; }

	static inline T& potr() { return _pot; }
	static inline T* potp() { return &_pot; }
	static inline T  potg() { return _pot; }
};

template<typename T>
T integer_limits<T>::_min = std::numeric_limits<T>::min();
template<typename T>
T integer_limits<T>::_max = std::numeric_limits<T>::max();
template<typename T>
T integer_limits<T>::_pot = numeric_helper<T>::pot;

template<typename T, int any = 0>
struct named_static {
	typedef T type;
	static T value;

	named_static(cstring name, T initial) noexcept {
		value = initial;
		sname = name;
	}

	operator T() const noexcept { return value; }
	T operator=(T val) noexcept { return value = val; }

	static inline T& ref() noexcept { return value; }
	static inline T* ptr() noexcept { return &value; }
	static inline T  get() noexcept { return value; }
	static inline void  set(T v) noexcept { value = v; }
	static inline cstring name() noexcept { return sname; }
private:
	static cstring sname;
};

template<typename T, int any>
T named_static<T,any>::value;
template<typename T, int any>
cstring named_static<T,any>::sname {nullptr};

template<typename T, int N, int M = 0>
struct static_string {
	typedef T type;
	static constexpr int size = N;
	static const T* get() noexcept { return data; }
	static T* ptr() noexcept { return data; }
	static T data[N];
	static T * arr(unsigned n) noexcept {
		return n < static_cast<int>(N) ? data + n : nullptr;
	}

};

template<typename T, int N, int M>
T static_string<T,N,M>::data[N];

template<item I, class M>
static result_t runx(const Environment& env, cstring inp, const M &m,
		error_t expect = error_t::noerror) noexcept {
	bool r = I().read(json(inp));
	bool e = m.run(env);
	return combine2(r, e, Test::expected(json().error(), expect));
}

template<class M, const clas<M>& I()>
static result_t runx(const Environment& env, cstring inp, const M &m, M&& t,
		error_t expect = error_t::noerror)
	noexcept {
	bool r = I().read(t, json(inp));
	bool e = m.run(env,&t);
	return combine2(r, e, Test::expected(json().error(), expect));
}

template<int M=0>
struct sstr : static_string<char_t, 32, M> {
	using static_string<char_t, 32, M>::size;
	typedef sstr<M> self;
	//using static_string<char, 32, M>::data;
	static void fill(char_t c = '_') noexcept {
		for(int i = 0; i < (size-1); ++i) self::data[i] = c;
		self::data[size-1] = 0;
	}
	static bool match(const char_t * m) noexcept {
		return details::match(m, self::data);
	}
	static result_t run(const Environment& env, cstring inp,
			const char_t* answer, error_t expected = error_t::noerror) noexcept {
		bool r,m;
		self::fill();
		r = cojson::V<size, &self::ptr>().read(json(inp));
		error_t err = Test::expected(json().error(), expected);
		m = self::match(answer) && err == error_t::noerror;
		/* in some wchar cases may produce no output */
		env.out(r && m, fmt<decltype(self::get())>(), self::get());
		env.msg(LVL::debug, fmt<decltype(answer)>(), answer);
		return combine2(r, m, json().error() xor expected);
	}

	static result_t run0(const Environment& env, cstring inp,
			const char_t* answer) noexcept {
		bool r, m;
		self::fill();
		r = cojson::V<&self::get>().read(json(inp));
		env.out(m = self::match(answer), "got: %s\nexp: %s\n",
				self::get(), answer);
		return combine2(r, m,
				Test::expected(json().error(),error_t::noobject));
	}

};


template<typename T, unsigned N>
struct arr {
	typedef arr<T,N> self;
	static T data[N];

	static bool match(const T (&answer)[N]) noexcept {
		return memcmp(self::data,answer, sizeof(data)) == 0;
	}
	static result_t run(const Environment& env, cstring inp,
			const T (&answer)[N], error_t expected =error_t::noerror) noexcept {
		bool r = cojson::V<T, self::item>().read(json(inp));
		error_t err = Test::expected(json().error(), expected);
		bool m = self::match(answer) && err == error_t::noerror;
		self::dump(env, r && m);
		r = r != ((expected & error_t::bad)==error_t::bad);
		return combine2(r, m, json().error() xor expected);
	}

	static result_t run2(const Environment& env, cstring inp,
			const T (&answer)[N], error_t expected =error_t::noerror) noexcept {
		bool r = cojson::V<T, N, self::data>().read(json(inp));
		error_t err = Test::expected(json().error(), expected);
		bool m = self::match(answer) && err == error_t::noerror;
		self::dump(env, r && m);
		r = r != ((expected & error_t::bad)==error_t::bad);
		return combine2(r, m, json().error() xor expected);
	}

	static T* item(unsigned i) noexcept {
		if( i < 0 ) return nullptr;
		return static_cast<unsigned>(i) < N ? data + i : nullptr;
	}

	static inline void dump(const Environment& env, bool success) noexcept {
		const char * dlm = "[ ";
		for(unsigned i = 0; i < N; ++i ) {
			env.out(success, dlm );
			env.out(success, fmt<T>(), data[i]);
			dlm = ", ";
		}
		env.out(success, " ]\n");
	}
};

template<typename C = char_t>
struct register_specifier { };

extern "C" int register_char16_specifier(void);
extern "C" int register_char32_specifier(void);

template<>
struct register_specifier<char16_t> {
	inline register_specifier() noexcept {
		register_char16_specifier();
	}
};

template<>
struct register_specifier<char32_t> {
	inline register_specifier() noexcept {
		register_char32_specifier();
	}
};

}}

#define _T_ (_T_ has to be defined in the <test>.cpp file)
#ifdef CSTRING_PROGMEM
#define _M_(n) template<> const char_t pstring<(_T_+n)>::str[] __attribute__((progmem))
#else
#define _M_(n) template<> const char_t pstring<(_T_+n)>::str[]
#endif
#define _P_(n) cstring(pstring<(_T_+n)>::str)

#ifdef COJSON_TEST_OMIT_NAMES
#	define OMIT(s) tstring(nullptr)
#else
#	ifdef CSTRING_PROGMEM
#		define OMIT(s) TSTR(s)
#	else
#		define OMIT(s) (s)
#	endif
#endif

#endif /* TOOLS_TEST_HPP_ */
