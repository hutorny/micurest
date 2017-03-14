/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurest_progmem.cpp - progmem storage for string literals
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License v2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

/*
 * This file provides implementation progmem methods and placements for
 * promem literals
 */
#include <avr/pgmspace.h>
#include "micurest.hpp"
namespace http_01 {
typedef literal<micurest::details::pstr> _;
constexpr const char _::_HTTP			[] __attribute__((progmem));
constexpr const char _::__1_1			[] __attribute__((progmem));
constexpr const char _::__1_0			[] __attribute__((progmem));
constexpr const char _::__0_1			[] __attribute__((progmem));
constexpr const char _::_GET			[] __attribute__((progmem));
constexpr const char _::_PUT			[] __attribute__((progmem));
constexpr const char _::_POST			[] __attribute__((progmem));
constexpr const char _::_DELETE			[] __attribute__((progmem));
constexpr const char _::_Accept			[] __attribute__((progmem));
constexpr const char _::_Connection		[] __attribute__((progmem));
constexpr const char _::_Content_Type	[] __attribute__((progmem));
constexpr const char _::_Content_Length	[] __attribute__((progmem));
constexpr const char _::_If_Match 		[] __attribute__((progmem));
constexpr const char _::_If_None_Match	[] __attribute__((progmem));
constexpr const char _::_ETag			[] __attribute__((progmem));
constexpr const char _::_keep			[] __attribute__((progmem));
constexpr const char _::_close			[] __attribute__((progmem));
constexpr const char _::media_type::_application_[] __attribute__((progmem));
constexpr const char _::media_type::_text_		 [] __attribute__((progmem));

constexpr const char _::media_type::application::_json		  [] __attribute__((progmem));
constexpr const char _::media_type::application::_javascript  [] __attribute__((progmem));
constexpr const char _::media_type::application::_octet_stream[] __attribute__((progmem));

constexpr const char _::media_type::text::_css	[] __attribute__((progmem));
constexpr const char _::media_type::text::_csv	[] __attribute__((progmem));
constexpr const char _::media_type::text::_html	[] __attribute__((progmem));
constexpr const char _::media_type::text::_plain[] __attribute__((progmem));
constexpr const char _::media_type::text::_xml	[] __attribute__((progmem));
constexpr const char _::media_type::text::_xsl	[] __attribute__((progmem));
}

namespace cojson {
namespace details {
template<>
progmem<char> progmem<progmem<char>>::read(const progmem<char>* ptr) {
	static_assert(sizeof(progmem<char>)==2,"Unexpected data size");
	return progmem<char>(
		reinterpret_cast<const char*>(pgm_read_word_near(ptr)));
}
}}
namespace micurest {
namespace details {

size_t strlen(pstr p) noexcept {
	return __strlen_P(static_cast<const char*>(p));
}

void copy(char* dst, pstr src, size_t n) noexcept {
	strncpy_P(dst,(const char*)src,n);
}

}
typedef details::literal<details::pstr> _;
constexpr const char _::_OK					[] __attribute__((progmem));
constexpr const char _::_Created			[] __attribute__((progmem));
constexpr const char _::_Deleted			[] __attribute__((progmem));
constexpr const char _::_Valid				[] __attribute__((progmem));
constexpr const char _::_Changed			[] __attribute__((progmem));
constexpr const char _::_Content			[] __attribute__((progmem));
constexpr const char _::_Accepted			[] __attribute__((progmem));
constexpr const char _::_No_Content			[] __attribute__((progmem));
constexpr const char _::_Not_Modified		[] __attribute__((progmem));
constexpr const char _::_Bad_Request		[] __attribute__((progmem));
constexpr const char _::_Bad_Option			[] __attribute__((progmem));
constexpr const char _::_Unauthorized		[] __attribute__((progmem));
constexpr const char _::_Forbidden			[] __attribute__((progmem));
constexpr const char _::_Not_Found			[] __attribute__((progmem));
constexpr const char _::_Conflict			[] __attribute__((progmem));
constexpr const char _::_Length_Required	[] __attribute__((progmem));
constexpr const char _::_Payload_Too_Large	[] __attribute__((progmem));
constexpr const char _::_URI_Too_Long		[] __attribute__((progmem));
constexpr const char _::_Upgrade_Required	[] __attribute__((progmem));
constexpr const char _::_Method_Not_Allowed	[] __attribute__((progmem));
constexpr const char _::_Not_Acceptable		[] __attribute__((progmem));
constexpr const char _::_Not_Implemented	[] __attribute__((progmem));
constexpr const char _::_Service_Unavailable[] __attribute__((progmem));
constexpr const char _::_Precondition_Failed[] __attribute__((progmem));
constexpr const char _::_Request_Entity_Too_Large[] __attribute__((progmem));
constexpr const char _::_Unsupported_Content_Format[] __attribute__((progmem));
constexpr const char _::_Internal_Server_Error[] __attribute__((progmem));

using __ = status_t;
const details::pstr _::map::message[+status_t::__count_of] __attribute__((progmem))={
	[+__::Unknown					]= Internal_Server_Error(),
	[+__::OK						]= OK(),
	[+__::Created					]= Created(),
	[+__::Deleted					]= No_Content(),
	[+__::Valid						]= Not_Modified(),
	[+__::Changed					]= No_Content(),
	[+__::Content					]= OK(),
	[+__::Accepted					]= Accepted(),
	[+__::No_Content				]= No_Content(),
	[+__::Bad_Request				]= Bad_Request(),
	[+__::Bad_Option				]= Bad_Request(),
	[+__::Unauthorized				]= Unauthorized(),
	[+__::Forbidden					]= Forbidden(),
	[+__::Not_Found					]= Not_Found(),
	[+__::Conflict					]= Conflict(),
	[+__::Length_Required			]= Length_Required(),
	[+__::Payload_Too_Large			]= Payload_Too_Large(),
	[+__::URI_Too_Long				]= URI_Too_Long(),
	[+__::Upgrade_Required			]= Upgrade_Required(),
	[+__::Method_Not_Allowed		]= Method_Not_Allowed(),
	[+__::Not_Acceptable			]= Not_Acceptable(),
	[+__::Precondition_Failed		]= Precondition_Failed(),
	[+__::Request_Entity_Too_Large	]= Payload_Too_Large(),
	[+__::Unsupported_Content_Format]= Unsupported_Content_Format(),
	[+__::Internal_Server_Error		]= Internal_Server_Error(),
	[+__::Not_Implemented			]= Not_Implemented(),
	[+__::Service_Unavailable		]= Service_Unavailable()
};

}
