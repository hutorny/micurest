/*
 * Copyright (C) 2015, 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * cojson_float.hpp - floating conversion to string
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
#ifndef COJSON_FLOAT_HPP_
#define COJSON_FLOAT_HPP_
#include <limits.h>
#include <math.h>
#ifndef COJSON_HELPERS_HPP_
#	include "cojson_helpers.hpp"
#endif

/*
 * Motivation
 *
 * In v1 cojson uses sprintf for serializing double values.
 * However, on some platforms sprintf is too heavy
 * This file provides own implementation of floating serialization
 */

namespace cojson {
namespace floating {

typedef uint_fast16_t ushort;
typedef int_fast16_t  sshort;
typedef uint_fast8_t  uchar;
#ifdef __BYTE_ORDER__
#	if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		static constexpr bool target_is_le = true;
#	elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		static constexpr bool target_is_le = false;
#	else
#		error Unknown target endianess ##__BYTE_ORDER__
#	endif
#else
#	error Undefined target endianess ##__BYTE_ORDER__
#endif


/*
 * some platforms (avr, arm, masp) have ieeefp.h
 * others (i686, pic) have ieee754.h
 * to make it target independent, own definition for
 * ieee754 bits is added
 */

template<unsigned bits, bool little_endian = target_is_le>
struct ieee754_bits;

template<>
struct ieee754_bits<32, true> {
	static constexpr sshort bias = 127;
	ushort mantissa0 : 16;
	ushort mantissa1 :  7;
	ushort exponent  :  8;
	ushort sign      :  1;
};

template<>
struct ieee754_bits<64, true> {
	static constexpr sshort bias = 1023;
	ushort mantissa0 : 16;
	ushort mantissa1 : 16;
	ushort mantissa2 : 16;
	ushort mantissa3 :  4;
	ushort exponent  : 11;
	ushort sign      :  1;
};

template<>
struct ieee754_bits<32, false> {
	static constexpr sshort bias = 127;
	ushort sign      :  1;
	ushort exponent  :  8;
	ushort mantissa1 :  7;
	ushort mantissa0 : 16;
};

template<>
struct ieee754_bits<64, false> {
	static constexpr sshort bias = 1023;
	ushort sign      :  1;
	ushort exponent  : 11;
	ushort mantissa3 :  4;
	ushort mantissa2 : 16;
	ushort mantissa1 : 16;
	ushort mantissa0 : 16;
};

using namespace cojson;
using namespace details;

template<typename T, typename ulong>
struct floating {
	/* Leading zeros are not kept in the fraction because they eat precision */
	enum notation : int_fast8_t {
		/* the following cases possible              		*/
		scientific =  2,  /* scientific notation 1.2e6		*/
		fixed      =  1,  /* fixed notation		12.5		*/
		integral   =  0,  /* integral 			12			*/
		zero_1	   = -1,  /* leading zero 		 0.012		*/
		zero_2	   = -2,  /* two leading zeros	 0.0012		*/
		zero_3	   = -3,  /* three leading zeros 0.00012	*/
		zero_4	   = -4,  /* four leading zeros  0.000012	*/
	};
	/* Primary worker method for IEEE 754, returns selected notation	*/
	inline notation ieee754(const T& val, uchar precision) noexcept {
		lab_union lab;
		fract = 0;
		lab.value = (neg = (val < 0)) ? -val : val;
		exp = (lab.ieee754.exponent - lab.ieee754.bias) * ln2;
		adjust(lab.value);
		if( exp == (sshort)precision ) return _integral(lab.value);
		if( exp >= (sshort)precision || exp <-4 ) return _scientific(precision);
		if( exp < 0 ) return _zeroed(precision);
		return _fixed(lab.value, precision);
	}
	/* part getters, made via reference since on low end platforms address
	 * is not longer 32 bits, perhaps this saves some bytes	 			*/
	inline const ulong& mantissa() const noexcept {	return mantiss;	}
	inline const ulong& fraction() const noexcept {	return fract;	}
	inline const sshort& exponent() const noexcept{	return exp;		}
	inline bool negative()  const noexcept 		  {	return neg; 	}
private:
	inline void adjust(const T& val) noexcept {
		/* Due to rounding error division by exponent may not result a
		 * normalized value. To fix this the, value is force-normalized
		 * to the range [1..10[ and the exponent adjusted correspondingly	*/
		value = val / exp_10<T>(exp);
		if( value >= 10 ) {
			value /= 10.0;
			++exp;
		}
		if( value < 1 ) {
			value *= 10.0;
			--exp;
		}
	}

	inline notation _zeroed(uchar precision) noexcept {
		sshort _exp = exp;
		exp = 0;
		mantiss = 0;
		fraction(precision);
		return static_cast<notation>(_exp);
	}

	inline notation _fixed(const T& val, uchar precision) noexcept {
		sshort _exp = exp;
		exp = 0;
		mantiss = val;
		value = val - mantiss;
		fraction(precision - _exp);
		return notation::fixed;
	}

	inline notation _integral(const T& val) noexcept {
		mantiss = round(val);
		fract = 0;
		exp = 0;
		return notation::integral;
	}

	inline notation _scientific(uchar precision) noexcept {
		mantiss = value;
		value -= mantiss;
		fraction(precision);
		return notation::scientific;
	}
	inline void fraction(uchar precision) noexcept {
		--precision;
		fract = round(value  *  exp_10<T>(precision)) *
								exp_10<T>(maxprecision-precision);
		/* in case if  fraction rounded up to the mantiss digit,
		 * pot/10 because of digits10-1, see below							*/
		unsigned char ovf;
		mantiss += ovf = fract / pot10;
		if( ovf && exp ) fract %= pot10;
	}

private:
	//on some platforms log10 is not constexpr (yet?)
	static constexpr T ln2 = //log10(2.0);
			0.3010299956639811952256464283594894482121162582188844680786132812;
	/* all count of digits10 can not be used because the highest digit
	 * has limited range  													*/
	static constexpr uchar maxprecision=std::numeric_limits<ulong>::digits10-1;
	/* power of ten value for detecting fraction overflow to mantissa		*/
	static constexpr ulong pot10 = numeric_helper<ulong>::pot/10;
	union lab_union {
		T value;
		ieee754_bits<sizeof(T)*8> ieee754;
	};
	static_assert(sizeof(lab_union)==sizeof(T),"lab_union misaligned");
	/* adjusted value kept separately because original is needed too */
	T value;
	ulong mantiss;
	ulong fract;
	sshort exp;
	bool neg;
};

template<class S, typename ulong = uint32_t>
bool serialize(double val, S& out, sshort precision = 6) noexcept {
	using floating = floating<double,ulong>;
	using digitizer = digitizer<ulong>;
	floating dbl;
	uchar digit = 0;
	bool was = false;
	if( val == 0 ) {
		out.put('0');
		return true;
	}
	if( isnan(val) || isinf(val) )
		return false;
	auto notation = dbl.ieee754(val,precision);
	if( dbl.negative() ) out.put('-');
	if( notation <= floating::notation::zero_1 ) {
		out.put('0');
	} else {
		digitizer mantissa(dbl.mantissa());
		while( mantissa.get(digit) ) {
			if( digit || was ) {
				out.put('0' + digit);
				was = true;
				--precision;
			}
		}
		if( notation == floating::notation::integral ) return true;
	}
	if( dbl.fraction() != 0 ) {
		digitizer fraction(dbl.fraction());
		if( fraction )
			out.put('.');
		switch(notation) {
		case floating::notation::zero_4:
			out.put('0'); /* no break */
		case floating::notation::zero_3:
			out.put('0'); /* no break */
		case floating::notation::zero_2:
			out.put('0'); /* no break */
		case floating::notation::zero_1:
		default:
			if( fraction == 0 && notation == floating::notation::fixed )
				return true;
			fraction.skip(notation > floating::notation::zero_1);
			was = false;
			while( precision > 0 && fraction && fraction.get(digit,true) ) {
				if( digit || precision > 1 )
					out.put('0' + digit);
				if( (was = was || digit) ) --precision;
			}
		}
	} else
		if( notation == floating::notation::fixed )
			return true;

	if( notation == floating::notation::scientific ) {
		sshort exp = dbl.exponent();
		out.put('e');
		if( exp < 0 ) {
			out.put('-');
			exp = -exp;
		}
		digitizer exponent(exp);
		was = false;
		while( exponent.get(digit) ) {
			if( digit || was ) {
				out.put('0' + digit);
				was = true;
			}
		}
	}
	return true;
}


}
}
#endif // COJSON_FLOAT_HPP_
