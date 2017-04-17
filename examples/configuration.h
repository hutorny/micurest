#pragma once

/* I.   Place a copy of this file on the include path
 * II.  REMOVE options to be left at default state
 * III. Adjust options as needed
 * IV.  Change selector from Default to User or any other used here
 * V.   Multiple configurations can be kept in one file and switched by
 * 		the selector
 */

#include "cojson.ccs"
#include "micurest.ccs"

namespace configuration {
namespace build { struct User;	}

struct Current : Is<target::All,build::Default> {};//User configuration is NOT active
//struct Current : Is<target::AVR,build::User> {}; //User configuration is ACTIVE

template<> struct Selector<cojson::config> : Current {};
template<> struct Selector<cojson::details::lexer> : Current {};


/************************************************************************/
/*							cojson configuration						*/
/************************************************************************/

/**
 * Configuration symbols in cojson configuration
 * Symbol 		| Values		| When specified...
 * -------------+---------------+-----------------------------------------------
 * overflow 	| saturate		| numbers are saturated on overflow
 * 				| error			| overflow causes an error
 * 				| ignore		| overflow condition silently ignored
 * -------------+---------------+-----------------------------------------------
 * mismatch		| skip			| reader makes best efforts to skip such values
 * 				| error			| any mismatch in size or data type
 * 				|			  	| is treated as an error
 * -------------+---------------+-----------------------------------------------
 * null			| skip			| skip nulls by default
 * 				| error			| default handling for null is an error
 * -------------+---------------+-----------------------------------------------
 * iostate		| _notvirtual	| stream's error method are not virtual
 * 				| _virtual		| stream's error method are virtual,
 * 				|				| needed if a class implements both
 * 				|				| cojson::istream and cojson::ostream
 * -------------+---------------+-----------------------------------------------
 * temporary	| _static		| temporary buffer is implemented static
 * 				| _automatic	| temporary buffer is implemented automatic
 * -------------+---------------+-----------------------------------------------
 * temporary_size				| overrides temporary buffer size
 * -------------+---------------+-----------------------------------------------
 */

template<>
struct Configuration<cojson::config, target::All, build::User>
	:  Configuration<cojson::config, target::All, build::Default> {
	/* REMOVE items to be left at default state 						*/


	/** use of wchar_t 													*/
	//typedef wchar_t char_t;

	/** controls behavior on integral overflow 							*/
	static constexpr auto overflow 	= overflow_is::error;
	/** controls implementation of iostate::error						*/
	static constexpr auto iostate   = iostate_is::_virtual;

	/** controls default null handling.									*/
	static constexpr auto null = null_is::error;

	/** controls implementation of temporary buffer, writing floating
	 * data types with sprintf											*/
	static constexpr temporary_is sprintf_buffer = temporary_is::_static;
	/** controls size of temporary buffer								*/
	static constexpr unsigned sprintf_buffer_size = 32;
};

template<>
struct Configuration<cojson::details::lexer, target::All, build::User>
  :    Configuration<cojson::details::lexer, target::All, build::Default> {
	/* REMOVE items to be left at default state 						*/

	/** controls behavior on read encountered element mismatching
	 *	targed data type												*/
	static constexpr auto mismatch =
			cojson::default_config::mismatch_is::error;

	/** controls size of lexer's temporary buffer						*/
	static constexpr unsigned temporary_size = 32;

	/** controls implementation of lexer's temporary buffer, used for
	 * reading names  													*/
	static constexpr auto temporary_placement =
			cojson::default_config::temporary_is::_static;

};

/************************************************************************/
/*							micurest configuration						*/
/************************************************************************/
namespace configuration {
	template<>
	struct Configuration<micurest::config, target::All, build::User>
	  : Configuration<micurest::config, target::All, build::Default> {
	/** When set to true, response status message is not written,
	 *  only status code is  */
	static constexpr bool empty_status_message = false;

	/** When set to true, POST is handled as sequence of PUT and GET */
	static constexpr bool handle_post_as_put_get = true;

	/** When set to true, node gets extra virtual methods: post, del
	 * for handling HTTP POST and DELETE 							*/
	static constexpr bool extended_node = false;

	/** When set to true, requests started with HTTP/0.1
	 * will fail with error if contain unsupported header.
	 * in other cases unsupported headers are ignored  				*/
	static constexpr bool strict_http01 = false;

	/** identity_t data type used for non-statically named resources */
	typedef uint32_t identity_t;

	/** sets size of ETag */
	static constexpr size_t etag_size = 16;
	};
}

}
