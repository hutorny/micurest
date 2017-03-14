/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * cojson.cpp - main implementation file
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
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

#include "cojson.hpp"
#include <stdint.h>

namespace cojson {
namespace details {

/**
 * write value v as a 4HEXDIG char code
 */
static inline bool ashex(char16_t v, ostream& out) noexcept {
	/* rfc7159##section-7 allows only 4HEXDIG char codes */
	unsigned char n = sizeof(v)*8 - 4;
	while(n != 0 ) {
		char16_t c = (v >> n) & (char16_t)0xF;
		if( ! out.put(ashex(c)) ) return false;
		n -= 4;
	}
	return true;
} /* avr: 106 bytes */


bool writer<const char_t*>::write(char_t chr, ostream& out) noexcept {
	if( literal::is_control(chr) ) {
		char_t c = literal::replace_common(chr);
		if(c != chr ) {
			return
				out.put(literal::escape) &&
				out.put(c);
		} else {
			return
				out.put(literal::escape)   &&
				out.put(literal::hex_mark) &&
				ashex(chr, out);
		}
	} else {
		if( literal::is_escaped(chr) ) {
			out.put(literal::escape);
		}
		return out.put(chr);
	}
}

bool writer<const char_t*>::write(const char_t * str, ostream& out) noexcept {
	if( str == nullptr )
		return value::null(out);
	bool r = true;
	if( ! out.put(literal::quotation_mark) ) return false;
	while( *str && (r=write(*str++,out)) );
	return r && out.put(literal::quotation_mark);
} /* avr: 904 bytes */

bool reader<char_t*>::read(char_t* dst, size_t n, lexer& in) noexcept  {
	bool first = true;
	if( dst == nullptr || n == 0 ) {
		in.error(error_t::noobject);
		return in.skip_string(first);
	}
	ctype ct;
	while( n != 0 && (ct=in.string(*dst, first)) == ctype::string ) {
		++dst; --n; first = false;
	}
	if( n == 0 ) {
		*--dst = 0;
		in.error(error_t::overrun);
		return in.skip_string(first);
	}
	if( ct == ctype::eof ) {
		*dst = 0;
		return true;
	}
	if( *dst ) {
		in.error(error_t::bad);
		*dst = 0;
		return false;
	}
	return true;
} /* avr: 456 bytes */

template<typename T>
class maker {
public:
	inline maker(T& val) noexcept : value(val) { value = 0.0; }
	inline bool done() noexcept {
		if( sexp == 2 ) return false;
		if( sign < 0 ) value = -value;
		if( sexp )
			value *= exp_10<T>(exp * sexp);
		return true;
	}
	inline bool digit(char_t digit) noexcept {
		if( sexp ) exponent(digit);
		else if( frac ) fraction(digit);
		else significand(digit);
		return true;
	}
	inline bool minus() noexcept {
		if( sexp == 0 ) {
			if ( sign == 0 )
				sign = -1;
			else
				return false;
		} else
		if( (sexp & 1) != 0 )
			return false;
		else
			sexp = -1;
		return true;
	}
	inline bool plus() noexcept {
		if( sexp != 2 ) return false;
		sexp = 1;
		return true;
	}
	inline bool e() noexcept {
		if( sexp || ! sign ) return false;
		sexp = 2;
		return true;
	}
	inline bool dot() noexcept {
		if( frac ) return false;
		frac = -1;
		return true;
	}
private:
	inline void fraction(char_t digit) noexcept {
		pow /= 10.;
		value += digit * pow;
	}
	inline void significand(char_t digit) noexcept {
		if( ! sign ) sign = 1;
		value *= 10.;
		value += digit;
	}
	inline void exponent(char_t digit) noexcept {
		if( sexp == 2 ) sexp = 1;
		exp *= 10;
		exp += digit;
	}

private:
	T& value;
	T pow = 1.0;
	int_fast16_t exp = 0;
	int_fast8_t sign = 0;
	int_fast8_t sexp = 0;
	int_fast8_t frac = 0;
};

bool reader<double>::read(double& val, lexer& in) noexcept {
	char_t digit = 0;
	maker<double> maker(val);
	ctype ct;
	if( ! isvalid(in.value(ctype::numeric)) ) return false;
	while(true) {
		switch( ct = in.get(digit, ctype::numeric) ) {
		default:
			in.error(error_t::bad);
			return false;
		case ctype::eof:
		case ctype::delim:
			if( !isws(digit) ) in.back(digit);
			return maker.done();
		case ctype::digit:
			if( maker.digit(digit - literal::digit0) ) continue;
			break;
		case ctype::sign:
			if( (digit == literal::minus) ? maker.minus() : maker.plus() )
				continue;
			break;
		case ctype::exponent:
			if( maker.e() ) continue;
			break;
		case ctype::decimal:
			if( maker.dot() ) continue;
			break;
		}
		return in.skip(ctype::number);
	}
} /* avr: 1010 bytes (+ 700 for float procedures */

}
/******************************************************************************/
namespace details {

static constexpr bool is_null(char_t chr) noexcept {
	return chr == literal_strings<char_t>::null_l()[0];
}

struct bstack {
	enum sym : bool {
		array = false,
		object = true
	};
	inline bstack() : stack(0), count(0) {}
	inline bool push(sym s) noexcept {
		if( count ==  max ) return false;
		stack <<= 1;
		stack |= s == object;
		++count;
		return true;
	}
	inline bool pop(sym s) noexcept {
		if ( count == 0 ) return false;
		bool r = top() == s;
		stack >>= 1;
		--count;
		return r;
	}
	inline bool empty() const noexcept { return count == 0; }
	inline sym top() const noexcept {
		return (stack&1) ? object : array;
	}
private:
	static constexpr uint_fast8_t max = (sizeof(uint_fast8_t)*8)-1;
	uint_fast8_t stack;
	uint_fast8_t count;
}; /* avr: 20 bytes */

bool lexer::skip(bool list) noexcept {
if( config::config::mismatch != config::config::mismatch_is::error ) {
	bstack stack;
	char_t chr;
	ctype ct;
	if( list ) stack.push(stack.array);
	while(isvalid(ct=skip(chr, ctype::whitespace))) {
		if( hasbits(ct, (ctype::digit | ctype::sign)) ) {
			/* ignoring numbers and any number-looking garbage */
			if( ! isvalid(skip(chr, ctype::number)) )
				break;
			if( isws(chr) ) continue;
		}
		switch(chr) {
		case literal::begin_array:
			if( ! skipws(chr) ) goto abort;
			if( chr == literal::end_array ) break;
			if( ! stack.push(stack.array) )
				goto abort; /* stack depth exhausted, aborting 		*/
			back(chr);
			break;
		case literal::begin_object:
			if( ! skipws(chr) ) goto abort;
			if( chr == literal::end_object ) break;
			if( ! stack.push(stack.object) )
				goto abort; /* stack depth exhausted, aborting 		*/
			if( ! skip_member(chr != literal::quotation_mark) ) goto abort;
			break;
		case literal::end_array:
			if( ! stack.pop(stack.array) )
				goto abort;					/* unexpected ] 		*/
			if( stack.empty() ) goto done;
			break;
		case literal::end_object:
			if( ! stack.pop(stack.object) )
				goto abort; 				/* unexpected } 		*/
			if( stack.empty() ) goto done;
			break;
		case literal::value_separator:
			if( stack.empty() ) {
				back(chr);
				return true;				/* done 				*/
			}
			if( stack.top() == stack.object && ! skip_member(true) )
				goto abort;
			break;
		case literal::quotation_mark:
			if( ! skip_string(false) )
				goto abort;					/* malformed string 	*/
			break;
		case literal_strings<char_t>::true_l()[0]:
			if( ! literal(literal::true_l()+1) ) goto abort;
			break;
		case literal_strings<char_t>::false_l()[0]:
			if( ! literal(literal::false_l()+1) ) goto abort;
			break;
		case literal_strings<char_t>::null_l()[0]:
			if( ! literal(literal::null_l()+1) ) goto abort;
			break;
		default:
			goto abort;
		}
		if( stack.empty() ) return true;
	};
	if( chr == iostate::eos_c ) return stack.empty();
	done:
	if( stack.empty() ) {
		if( list ) back(chr);		/* do not skip closing ] */
		return true;				/* done 				*/
	}
	abort:
	error(error_t::bad);
}
	return false;
} /* avr: 102 bytes */

bool lexer::skip_string(bool first) noexcept {
	if( config::config::mismatch == config::config::mismatch_is::error )
		return false;
	else {
		char_t chr;
		while( string(chr, first) == ctype::string ) { first = false; }
		return chr == 0;
	}
}

bool lexer::skip_member(bool first) noexcept {
	char_t chr = 0;
	return
		skip_string(first) && skipws(chr) && chr == literal::name_separator;
}

static inline bool readable(const istream & in) noexcept {
	return error_t::noerror == (in.error() &
	  ((config::mismatch == config::mismatch_is::error) ?
		error_t::blocked : error_t::failed));
}

ctype lexer::get(char_t& chr) noexcept {
	if( ! readable(stream) ) return ctype::err;
	if( hold ) {
		chr = hold;
		hold = 0;
		return chartype(chr);
	} else {
		if( ! stream.get(chr) ) {
			return bad(chr);
		}
	}
	return chartype(chr);
}

inline ctype lexer::unhex(char_t& chr) noexcept {
	int n = 5;
	char_t v = 0;
	ctype ct;
	while( --n && isvalid(ct=get(chr)) ) {
		switch( ct & ctype::unhex ) { /* ct is valid here, so unsafe and used */
		case ctype::digit:
			v <<= 4;
			v |= (chr - literal::digit0);
			break;
		/* rfc7159#section-7: The hexadecimal letters A though F can be
		 * upper or lower case */
		case ctype::heX:
			v <<= 4;
			v |= (chr - literal::digitA + 10);
			break;
		case ctype::hex:
			v <<= 4;
			v |= (chr - literal::digita + 10);
			break;
		default:
			return bad();
		}
	}
	chr = v;
	return n == 0 ? ctype::string : bad();
}

inline ctype lexer::unescape(char_t& chr) noexcept {
	ctype ct;
	if( ! isvalid(ct=get(chr, ctype::special)) ) return ct;
	switch(chr) {
	case literal::escaped[0]:
	case literal::escaped[1]:
		return ctype::string;
	case literal::replacement [0]:
		return (chr = literal::common[0]), ctype::string;
	case literal::replacement [1]:
		return (chr = literal::common[1]), ctype::string;
	case literal::replacement [2]:
		return (chr = literal::common[2]), ctype::string;
	case literal::replacement [3]:
		return (chr = literal::common[3]), ctype::string;
	case literal::replacement [4]:
		return (chr = literal::common[4]), ctype::string;
	case literal::hex_mark:
		return unhex(chr);
	default:;
		return bad(chr);
	}
}
/*
 * return cases:
 * err, eof, string, end
 */
ctype lexer::string(char_t& chr, bool first) noexcept {
	if( first ) {
		if ( ! skipws(chr) ) return eos2eof(chr);
		if( chr != literal::quotation_mark ) return bad(chr);
	}
	if( !isvalid(get(chr)) ) return bad(chr);
	switch( chr ) {
	case literal::quotation_mark:
		chr = 0;
		return ctype::delim;
	case literal::escape:
		return unescape(chr);
	default:
		return ctype::string;
	}
} /* avr: 270 bytes (with unescape & unhex) */

bool lexer::member(char_t*& dst) noexcept {
	char_t chr;
	if( ! skipws(chr) ) { bad(chr); return false; }
	if( chr != literal::quotation_mark ) { bad(chr); return false; }
	back(chr);
	if( reader<char_t*>::read(name, name.size, *this) ) {
		if( ! skipws(chr) ) { bad(chr); return false; }
		if( chr == literal::name_separator ) {
			dst = name;
			return true;
		}
	}
	/* name was not read because of a bad character or eof */
	bad();
	return false;
} /*avr: 598 bytes */

char_t lexer::skip_bom() noexcept {
	char_t chr = 0;
	ctype ct;
	cstring bom = literal::bom();
	while( isvalid(ct=get(chr)) && *bom && *bom == chr) ++bom;
	switch(ct) {
	case ctype::err: return iostate::err_c;
	case ctype::eof: return iostate::eos_c;
	default:;
	}
	back(chr);
	return *bom == 0 ? literal::bom()[0] : 0;
} /* avr: 132 bytes */

//static inline constexpr bool match_value(int ct, int expected, char_t chr) noexcept {
//	/* ctype of quote appears to be not distinguishable from t,f,n	*/
//	return expected == ctype::string ? chr == literal::quotation_mark :
//			(ct & (int)ctype::value) && (ct & expected);
//}

ctype lexer::value(ctype expected) noexcept {
	ctype ct;
	char_t chr;
	if( !isvalid(ct=skip(chr,ctype::whitespace)) )
		return ct; /* not valid character */
	back(chr);
	if( (ct & ctype::value) == ctype::unknown ) {
		return bad();
	}
	switch(chr) {
	case literal::quotation_mark:
		return (expected & ctype::string) != ctype::unknown ?
				ctype::string : mismatch();
	case literal_strings<char_t>::null_l()[0]:
		return (expected & ctype::null) != ctype::unknown &&
				literal(literal::null_l()) ? ctype::null : mismatch();
	case literal_strings<char_t>::true_l()[0]:
		return (expected & ctype::boolean) != ctype::unknown &&
				literal(literal::true_l()) ?
					(ctype::boolean|ctype::value) : mismatch();
	case literal_strings<char_t>::false_l()[0]:
		return (expected & ctype::boolean) != ctype::unknown &&
				literal(literal::false_l()) ? ctype::boolean : mismatch();
	default:
		ct &= ~(int)ctype::string;
		ct &= (int)expected;
		return ct != ctype::unknown ? ct : mismatch();
	}
}

bool lexer::literal(cstring str) noexcept {
	char_t chr;
	ctype ct;
	while(*str && isvalid(get(chr))) {
		if(*str != chr ) break;
		++str;
	}
	if( *str == 0 && (hasbits(ct=get(chr), ctype::delim) || ct == ctype::eof)) {
		if( ct != ctype::eof )
			back(chr);
		return true;
	}
	error(error_t::bad);
	return false;
}

}
/******************************************************************************/
namespace details {
bool value::null(ostream& out) noexcept {
	return out.puts(literal::null_l());
}

bool ostream::_puts(const char_t* s) noexcept {
	while( *s && put(*s++));
	return *s == 0;
}

bool object::write(ostream& out) const noexcept {
	bool r = true;
	for(size_t i = 0; i<size && r ; ++i) {
		const member& m(nodes[i]());
		//TODO skip members with no value
		r = dlm(i==0, out) &&
			m.prolog(out)  &&
			m.writeval(out);
	}
	return r && end(out);
}

bool object::read(lexer& in, const char_t * name) const noexcept {
	for(size_t i = 0; i < size; ++i) {
		const member& m(nodes[i]());
		if( m.match(name) )
			return m.readval(in);
	}
	return false;
}

}}
