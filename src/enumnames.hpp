/**
 *  Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *  zero-dependency allocation free mapping of enum values to names
 */
#pragma once
namespace enumnames {

typedef const char* (*name)();
bool match(const char*, const char*) noexcept; /* to be provided by the user */

template<typename T, T K, name V>
struct tuple {
	typedef T key_t;
	static constexpr T key = K;
	static constexpr name val = V;
};

template<typename ... L>
struct map;

template<typename L>
struct map<L> {
	static constexpr typename L::key_t get(const char*) noexcept {
		return L::key; /* always returning last element */
	}
	static constexpr const char* get(typename L::key_t key) noexcept {
		return (L::val != nullptr) && (key == L::key) ? L::val() : "";
	}
	static constexpr const char* get(unsigned i) noexcept {
		return (L::val != nullptr) && i == 0 ? L::val() : nullptr;
	}
};

template<typename L, typename ... R>
struct map<L,R...> {
	static typename L::key_t get(const char* val) noexcept {
//		static_assert(L::val != nullptr, "Only last element may have null name");
		return match(val, L::val()) ? L::key : map<R...>::get(val);
	}
	static constexpr const char* get(typename L::key_t key) noexcept {
		return (key == L::key) ? L::val() : map<R...>::get(key);
	}
	static constexpr const char* get(unsigned i) noexcept {
		return i == 0 ? L::val() : map<R...>::get(i-1);
	}
};

template<typename T, typename ... L>
struct names {
	typedef T type;
	static T get(const char* nam) noexcept {
		return M::get(nam);
	}
	static constexpr const char* get(T key) noexcept {
		return M::get(key);
	}
	static constexpr const char* get(unsigned i) noexcept {
		return M::get(i);
	}
private:
	typedef map<L...> M;
};
}
