/*
 * Copyright (C) 2015 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * coop.hpp - cojson tests, definitions of command line options lib
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

#include <cstring>

namespace coop {

	/** biname - a function returning full or short option name 	*/
	typedef const char* (*biname)(bool full);

#	ifndef BINAME
	/** this macro can be used for option names which are valid C++ identifiers*/
#		define BINAME(f,s) static inline const char* f(bool full) noexcept \
				{ return full ? #f : #s; }
#	else
#	pragma message("BINAME already defined, skipping")
#	endif

	struct summary {
		const char * unknown;	/** unknown option */
		const char * missing;	/** name of the first missing
														mandatory argument	  */
		short first_missing;	/** first missing mandatory argument (ordinal)*/
		short count; 			/** count of processed arguments 			  */
	};

	namespace details {

	/** string to type conversion templates */
	template<typename T>
	bool string_to(T&, const char*) noexcept;
	int string_find(const char* l[], const char *s) noexcept;


	class noncopyable {
	private:
		noncopyable(const noncopyable&);
		noncopyable& operator=(const noncopyable&);
	public:
		noncopyable() { }
	};

	/**
	 * abstract argument
	 */
	struct argument : noncopyable {
		virtual ~argument() noexcept {}
		virtual bool set(const char*) const noexcept = 0;
		virtual const char * name(bool)  const noexcept { return nullptr; }
		virtual bool unknown() const noexcept { return false; }
		virtual bool ordinal() const noexcept { return true; }
		virtual bool mandatory() const noexcept { return false; }
		inline bool present() const noexcept { return wasset; }
		virtual bool apply(const char*, bool full) const noexcept = 0;
		static const char* prefixes[2]; /* - / -- */
		static const char* separator; 	/* = */
		static size_t match(const char*, const char*, bool, bool) noexcept;
	protected:
		mutable bool wasset;
		mutable bool tried;
	};

	class helper : noncopyable {
	public:
		static summary run(int, const char**, const argument& (*)(int), int) noexcept;
	protected:
		static bool is_prefixed(const char * src, bool& full) noexcept;
		static bool match_prefix(const char * str, bool full) noexcept;
		static void validate_prefixes() noexcept;
	private:
		helper();
	};

	/**
	 * template for a typed argument
	 */

	template<typename T, bool (*S)(const T&)>
	struct argoftype : argument {
		bool set(const char* str) const noexcept {
			T val;
			tried = true;
			return wasset = string_to(val, str) && S(val);
		}
		bool apply(const char* str, bool) const noexcept {
			if( tried ) return false;
			return set(str);
		}
	};

	template<biname N, typename T, bool (*S)(const T&)>
	struct namedarg : argoftype<T,S> {
		const char * name(bool full)  const noexcept {
			return N(full);
		}
		bool ordinal() const noexcept { return false; }
		bool apply(const char* str, bool full) const noexcept {
			if( this->tried ) return false;
			size_t off = argument::match(N(full), str, true, full);
			return off && this->set(str+off);
		}
	};

	template<biname N, bool (*S)()>
	struct option : argument {
		bool ordinal() const noexcept { return false; }
		bool set(const char*) const noexcept {
			tried = true;
			return wasset = S();
		}
		bool apply(const char* str, bool full) const noexcept {
			if( this->tried ) return false;
			size_t off = argument::match(N(full), str, false, full);
			return off && this->set(str+off);
		}
		const char * name(bool full)  const noexcept {
			return N(full);
		}
	};

	template<>
	inline bool string_to<const char*>(const char*& dst, const char* src) noexcept {
		return (dst = src) != nullptr;
	}

	template<>
	inline bool string_to<char*>(char*& dst, const char* src) noexcept {
		return (dst = strdup(src)) != nullptr;
	}
	} //namespace details
	using namespace details;

	typedef const argument& (*item)();

	/** ordinal optional argument of type T */
	template<typename T, bool (*S)(const T&)>
	const argument& O() noexcept {
		static argoftype<T,S> l;
		return l;
	}

	/** ordinal mandatory argument of type T */
	template<typename T, bool (*S)(const T&)>
	const argument& M() noexcept {
		static struct local : argoftype<T,S> {
			bool mandatory() const noexcept { return true; }
		} l;
		return l;
	}

	/** named optional argument of type T */
	template<biname N, typename T, bool (*S)(const T&)>
	const argument& O() noexcept {
		static namedarg<N,T,S> l;
		return l;
	}

	/** named option toggled by presence */
	template<biname N, bool (*S)()>
	const argument& O() noexcept {
		static option<N,S> l;
		return l;
	}

	/** named mandatory argument of type T */
	template<biname N, typename T, bool (*S)(const T&)>
	const argument& M() noexcept {
		static struct local : namedarg<N,T,S> {
			bool mandatory() const noexcept { return true; }
		} l;
		return l;
	}

	/** unknown option handler, processing stops if F returns false  */
	template<bool (*F)(const char*)>
	const argument& unknown() noexcept {
		static struct local : argument {
			void set(const char* str) const noexcept { }
			bool apply(const char* str) const noexcept {
				wasset = true;
				return F(str);
			}
			bool is_unknown() const noexcept { return true; }
		} l;
		return l;

	}

	/**
	 * parses argument vector, calls the callbacks
	 * returns summary of processing efforts
	 */

	template<item ... I>
	summary arguments(int argc, char** argv) noexcept {
		static constexpr item items[] { I ... };
		static constexpr int len = sizeof...(I);
		struct iterator {
			static const argument& get(int i) noexcept {
				return items[i]();
			}
		};

		return helper::run(argc,
				const_cast<const char**>(argv), &iterator::get, len);
	}

}


