/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurest.cpp - µcuREST library implementation
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
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#include <string.h>
#include "micurest.hpp"
#include "miculog.hpp"

namespace micurest {
const miculog::Log<application> log;
/*
 * 	TODO CoAP support for immediate ACK
 * 	TODO CoAP message ID handling and dedups
 */
namespace details {
void message::set_etag(const char_t* tag) noexcept {
	strncpy(etag, tag, etag_size);
	resp.fields |= header::etag;
}

constexpr const httpmessage::delimiters httpmessage::CSLF;

bool httpmessage::keep() const noexcept {
	/*	https://tools.ietf.org/html/rfc7230#section-6.1
		A server that does not support persistent connections MUST send the
		"close" connection option in every response message that does not
		have a 1xx (Informational) status code.								*/
	/*	micurest does not support 1XX status codes,
	 *  neither it can recover after an errors								*/
	return maykeep && requ.keep && isgood();
}

httpmessage::state_t httpmessage::parse() noexcept {
	switch(state) {
	case state_t::method:
		return state = read_method();
	case state_t::target:
	case state_t::version:
		return state = read_version();
	case state_t::header:
		return state = read_headers();
	default:
		return state;
	}
}

inline bool httpmessage::expect_crlf() noexcept {
	/*
	 * https://tools.ietf.org/html/rfc7230#section-3.5
	 * a recipient MAY recognize a single LF as a line terminator
	 * and ignore any preceding CR.
	 */
	bool res =  curr == lit::LF ||
		(  curr == lit::CR && get() && curr == lit::LF );
	return res;
}

bool httpmessage::consume_crlf() noexcept {
	return get() && expect_crlf();
}


httpmessage::state_t httpmessage::bad_request() noexcept {
	error(status_t::Bad_Request);
	return state = state_t::bad;
}

inline char_t httpmessage::pct_decode() noexcept {
	char_t chr[2];
	if( ! got(chr[1]) ) return 0;
	if( chr[1] == lit::PCT ) return chr[1];
	if( ! is_hex(chr[1]) ) return 0;
	if( ! got(chr[0]) ) return 0;
	if( ! is_hex(chr[0]) ) return 0;
	return chr[1] << 4 | chr[0];
}

inline bool httpmessage::is_hex(char_t& chr) noexcept {
	char_t hex = lit::is_hex(chr);
	if( ! hex ) { bad_request(); return false; }
	chr -= hex;
	return true;
}

inline char_t httpmessage::read_target(bool start) noexcept {
	if( start ) skip_ws();
	else get();
	switch(curr) {
	default:
		return curr;
	case lit::SPACE:
	case lit::HTAB:
		state = state_t::version;
		return 0;
	case lit::CR:
	case lit::LF:
		if( expect_crlf() )
			state = state_t::header;
		else
			bad_request();
		return 0;
	case lit::PCT:
		return pct_decode();
	}

}

httpmessage::state_t httpmessage::read_version() noexcept {
	if( literal1(lit::HTTP::HTTP_(), {lit::TSEP}) && get())
	switch(curr) {
	case lic::HTTP::_0_1()[0]:
		if( literal1(lit::HTTP::_0_1()+1) ) {
			requ.version = http::version::_0_1;
			return state_t::header;
		}
		break;
	case lic::HTTP::_1_0()[0]:
		switch( literals(lit::HTTP::_1_0()+1, lit::HTTP::_1_1()+1) ) {
		case 1:
			requ.version = http::version::_1_0;
			return state_t::header;
		case 2:
			requ.version = http::version::_1_1;
			return state_t::header;
		}
	}
	skip_line();
	return bad_request();
}


inline httpmessage::state_t httpmessage::read_method() noexcept {
	/* no whitespace before method */
	if( ! get() ) return state_t::empty;
	switch( curr ) {
	case lic::GET()[0]:
		if( literal1(lit::GET()+1, {lit::SPACE}) ) {
			requ.method = http::method::GET;
			return state_t::target;
		}
		break;
	case lic::PUT()[0]:
		switch(literals(lit::POST()+1, lit::PUT()+1, {lit::SPACE})) {
		case 1:
			requ.method = http::method::POST;
			return state_t::target;
		case 2:
			requ.method = http::method::PUT;
			return state_t::target;
		}
		break;
	case lic::DELETE()[0]:
		if( literal1(lit::DELETE()+1, {lit::SPACE}) ) {
			requ.method = http::method::DELETE;
			return state_t::target;
		}
		break;
	case lic::CR:
	case lic::LF:
		return state_t::empty;
	}
	error(status_t::Not_Implemented);
	return state_t::bad;
}


inline media::type httpmessage::read_mediatypetext() noexcept {
	if( get() )
	switch( curr ) {
	case lic::media_type::any()[0]:
		return media::anytext;
	case texc::css()[0]:
		switch( literals(text::css()+1, text::csv()+1, CSLF)) {
		case 1: return media::css;
		case 2: return media::csv;
		}
		break;
	case texc::html()[0]:
		if( literal1(text::html()+1, CSLF) )
			return media::html;
		break;
	case texc::plain()[0]:
		if( literal1(text::plain()+1, CSLF) )
			return media::plain;
		break;
	case texc::xml()[0]:
		switch( literals(text::xml()+1, text::xsl()+1, CSLF) ) {
		case 1: return media::xml;
		case 2: return media::xsl;
		}
		break;
	}
	if( config::strict_http01 ) if( is_ver_01() ) error(status_t::Bad_Option);
	return media::unknown;
}

inline media::type httpmessage::read_mediatypeappj() noexcept {
	if( get() )
	switch( curr ) {
	case appc::javascript()[1]: return literal1(app::javascript()+2, CSLF);
	case appc::json()[1]:
		switch(literal3(app::jsonrequest()+2,app::json_rpc()+2,
				app::json()+2, CSLF)) {
		case 1: return media::jsonrequest;
		case 2: return media::json_rpc;
		case 3: return media::json;
		}
	}
	if( config::strict_http01 ) if( is_ver_01() ) error(status_t::Bad_Option);
	return media::unknown;
}

inline media::type httpmessage::read_mediatypeapp() noexcept {
	if( get() )
	switch( curr ) {
	case lic::media_type::any()[0]:
		return media::anyapp;
	case appc::javascript()[0]:
		return httpmessage::read_mediatypeappj();
	case appc::octet_stream()[0]:
		if( literal1(app::octet_stream()+1, CSLF) )
			return media::octet_stream;
		break;
	}
	//TODO application/xhtml+xml
	if( config::strict_http01 ) if( is_ver_01() ) error(status_t::Bad_Option);
	return media::unknown;
}

bool httpmessage::read_accept() noexcept {
//	skip_ws();
	while( lit::is_linear(curr) ) {
		media::type type = httpmessage::read_mediatype();
		if( type != media::unknown ) requ.accept(type);
		if( lit::is_type(curr) ) skip_type();
		if( lit::is_space(curr) ) skip_ws();
	}
	switch( curr ) {
		case lit::CR:
		case lit::LF:
			expect_crlf();
			break;
		default:
			skip_line();
	}
	return true;
}

media::type httpmessage::read_mediatype() noexcept {
	skip_ws();
	switch( curr ) {
	case lic::media_type::any()[0]:
		skip_type();
		return media::any;
	case lic::media_type::text_()[0]:
		if( literal1(lit::media_type::text_()+1, {lit::TSEP}) )
			return httpmessage::read_mediatypetext();
		break;
	case lic::media_type::application_()[0]:
		if( literal1(lit::media_type::application_()+1, {lit::TSEP}) )
			return httpmessage::read_mediatypeapp();
		break;
	}
	if( config::strict_http01 && is_ver_01() ) error(status_t::Bad_Option);
	else skip_type();
	return media::unknown;
}

httpmessage::state_t httpmessage::read_headers() noexcept {
	while( get() ) {
		switch( curr ) {
		case lic::SPACE:
		case lic::HTAB:
			return bad_request();
		case lic::LF:
			return state_t::body;
		case lic::CR:
			return expect_crlf() ? state_t::body : bad_request();
		case lic::Accept()[0]:
			if( literal1(lit::Accept()+1,{lit::COLON}) ) {
				if( read_accept() ) {
					requ.fields |= header::accept;
					continue;
				}
			}
			break;
		case lic::Content_Length()[0]:
			switch( literal3(lit::Content_Length()+1, lit::Content_Type()+1,
					lit::Connection()+1, {lit::COLON})) {
			case 1:
				if( read_length() ) {
					requ.fields |= header::content_length;
					consume_crlf();
					continue;
				}
				break;
			case 2:
				if( (requ.content_type = read_mediatype()) ) {
					requ.fields |= header::content_type;
					expect_crlf();
					continue;
				}
				break;
			case 3:
				if( read_connection() ) {
					requ.fields |= header::connection;
					expect_crlf();
					continue;
				}
			}
			break;
		case lic::If_Match()[0]:
			if( read_condition() ) continue;
			break;
		default:;
			//TODO if version (0/1) bad option
		}
		if( skip_line() ) continue;
		error(status_t::Bad_Option);
		return state_t::bad;
	}
	error(status_t::Bad_Request);
	return state_t::bad;
}

bool httpmessage::read_condition() noexcept {
	switch(literals(lit::If_Match()+1,lit::If_None_Match()+1, {lit::COLON})) {
	case 1: return read_etag() && (requ.fields |= header::if_match);
	case 2: return read_etag() && (requ.fields |= header::if_none_match);
	default:
		return error(status_t::Bad_Request);
	}
}

bool httpmessage::read_etag() noexcept {
	//FIXME Etag is quoted with ""
	size_t i = 0;
	skip_ws();
	etag[i++] = curr > lit::SPACE ? curr : 0;
	do {
		switch(curr) {
		case lit::SPACE:
			etag[i++] = 0;
			return skip_line();
		case lit::CR:
			if( ! expect_crlf() ) {
				bad_request();
				return false;
			}
		case lit::LF:
			etag[i++] = 0;
			return true;
		default:
			etag[i++] = curr;
		}
	} while( i < etag_size && get() );
	if( i >= etag_size )
		error(status_t::Request_Entity_Too_Large);
	return false;
}

bool httpmessage::literal1_ic(cstring  lit,
						   const delimiters& dlm) noexcept {
	while( *lit && get() && lit::same(curr,*lit) ) ++lit;
	return *lit == 0 && get() && isdlmcrlf(dlm);
}


bool httpmessage::literal1(cstring  lit,
						   const delimiters& dlm) noexcept {
	while( *lit && get() && curr == *lit ) ++lit;
	return *lit == 0 && get() && isdlmcrlf(dlm);
}

unsigned char httpmessage::literals(cstring lit1, cstring lit2,
									const delimiters& dlm) noexcept {
	while( *lit1 == *lit2 && get() && curr == *lit1 )
		++lit1, ++lit2;
	if( ! get() ) return 0;
	//FIXME if *lit1 == 0
	if( *lit1 == curr ) return literal1(++lit1, dlm) ? 1 : 0;
	if( *lit2 == curr ) return literal1(++lit2, dlm) ? 2 : 0;
	return 0;
}

/** lit3 must have the shortest common start sequence */
unsigned char httpmessage::literal3(cstring lit1, cstring lit2, cstring lit3,
									const delimiters& dlm) noexcept {
	while( *lit3 == *lit1 && *lit3 == *lit2 && get() && curr == *lit3 )
		++lit1, ++lit2, ++lit3;
	if( ! get() ) return 0;
	if( *lit3 == 0 && isdlmcrlf(dlm) ) return 3;
	if( *lit3 == curr ) return literal1(++lit3, dlm) ? 3 : 0;
	return literals(++lit1, ++lit2, dlm);
}

bool httpmessage::read_connection() noexcept {
	skip_ws();			/* https://tools.ietf.org/html/rfc7232#section-2.3  */
	switch( lit::lower(curr) ) { /* case insensitive, IE sends Keep-Alive   */
	case lic::keep()[0]:
		if( literal1_ic(lit::keep()+1) ) {
			requ.keep = true;
			return true;
		}
		break;
	case lic::close()[0]:
		if( literal1_ic(lit::close()+1) ) {
			requ.keep = false;
			return true;
		}
	}
	return error(status_t::Bad_Option);
}
size_t strlen(const char* s) noexcept {
	return ::strlen(s);
}
}
using details::httpmessage;
using details::header;
using namespace resource;

const node* application::read_target_and_find(httpmessage& msg) const noexcept {
	char_t chr;
	char_t section[max_section_size];
	size_t i = 0;
	const node* dir = &root;
	chr = msg.read_target(true);
	if( chr != lit::PSEP ) {
		msg.bad_request();
		return nullptr;
	}
	while( i < max_section_size ) {
		if( (chr = msg.read_target(false)) == lit::PSEP || (chr == 0) ) {
			if( msg.failed() ) return nullptr;
			section[i] = 0;
			if( i ) dir = dir->find(section, msg);
			if( dir == nullptr ) {
				msg.not_found();
				return nullptr;
			}
			if( chr == 0 ) return dir;
			i = 0;
		} else
			section[i++] = chr;
	}
	msg.error(status_t::URI_Too_Long);
	return nullptr;
}

application::result_t application::service(istream& in, ostream& out,
					httpmessage::state_pdo* pdo) const noexcept {
	httpmessage msg(in, out);
	if( pdo && pdo->state > httpmessage::state_t::empty ) {
		msg.restore(*pdo);
		pdo = nullptr;
	}
	msg.maykeep = maykeep;
	while(true) {
		switch( msg.parse() ) {
		default: continue;
		case httpmessage::state_t::empty:
			return result_t::keep;
		case httpmessage::state_t::target:
			msg.target = read_target_and_find(msg);
			break;
		case httpmessage::state_t::body:
			//TODO return payload too large if content length exceeds capabilities
			if( msg.target ) {
				if( ! msg.req().has(header::accept) && msg.target->isdir() ) {
				/* assuming html if accept is not specified and target is a dir
				 */
					msg.requ.accept(media::html);
				}
				if( ! msg.failed() ) {
					msg.fix_content_type(msg.target->mediatype());
					msg.target->process(msg);
				}
				/* FIXME currently there is no indication that the message body
				has bee successfully processed. Therefore a fragmented message
				(header only) and a successfully processed message can be
				distinguished only by in.eof(). If the message body is not
				fixed by length, target reads it till the end triggering EOF.
				Because of this issue json messages require extra CRLF
				in the end													*/
				//log.debug("eof=%d good=%d state=%d status=%d\n", in.eof(), in.isgood(), msg.state, msg.resp.status );
				if( in.eof() && in.isgood() && msg.state == httpmessage::state_t::body) {
					if( pdo ) {
						msg.save(*pdo);
						return result_t::fragment;
					}
					log.info("no saved session, cannot continue with a fragment\n");
					msg.error(status_t::Service_Unavailable);
				}
				if( pdo ) pdo->clear();
				if( msg.failed() ) {
					if( ! msg.hasbody() ) {
						msg.obody().puts(lit::message(msg.res().status));
						msg.crlf();
					}
					return in.isgood() && out.isgood() ?
						result_t::bad : result_t::error;
				}
				msg.seal(true);
				return msg.keep() ? result_t::keep : result_t::close;
			}
			msg.not_found();
		case httpmessage::state_t::bad:
			msg.set_content_type(media::plain);
			msg.obody().puts(lit::message(msg.res().status));
			msg.crlf();
			return result_t::bad;
		}
	}
}

namespace identity {
	bool numeric(const char_t* str, type& identity) noexcept {
		return details::convert(str, identity);
	}
}

using _ = status_t; /* some shortcuts for composite identifiers */
using __ = http::status;
const unsigned short httpmessage::map::status[+status_t::__count_of]={
	[+_::Unknown					]= +__::Internal_Server_Error,
	[+_::OK							]= +__::OK,
	[+_::Created					]= +__::Created,
	[+_::Deleted					]= +__::No_Content,
	[+_::Valid						]= +__::Not_Modified,
	[+_::Changed					]= +__::No_Content,
	[+_::Content					]= +__::OK,
	[+_::Accepted					]= +__::Accepted,
	[+_::No_Content					]= +__::No_Content,
	[+_::Bad_Request				]= +__::Bad_Request,
	[+_::Bad_Option					]= +__::Bad_Request,
	[+_::Unauthorized				]= +__::Unauthorized,
	[+_::Forbidden					]= +__::Forbidden,
	[+_::Not_Found					]= +__::Not_Found,
	[+_::Conflict					]= +__::Conflict,
	[+_::Length_Required			]= +__::Length_Required,
	[+_::Payload_Too_Large			]= +__::Payload_Too_Large,
	[+_::URI_Too_Long				]= +__::URI_Too_Long,
	[+_::Upgrade_Required			]= +__::Upgrade_Required,
	[+_::Method_Not_Allowed			]= +__::Method_Not_Allowed,
	[+_::Not_Acceptable				]= +__::Not_Acceptable,
	[+_::Precondition_Failed		]= +__::Precondition_Failed,
	[+_::Request_Entity_Too_Large	]= +__::Payload_Too_Large,
	[+_::Unsupported_Content_Format	]= +__::Unsupported_Media_Type,
	[+_::Internal_Server_Error		]= +__::Internal_Server_Error,
	[+_::Not_Implemented			]= +__::Not_Implemented,
	[+_::Service_Unavailable		]= +__::Service_Unavailable,
};

const char* const details::literal<const char*>::map::message[+status_t::__count_of]={
	[+_::Unknown					]= Internal_Server_Error(),
	[+_::OK							]= OK(),
	[+_::Created					]= Created(),
	[+_::Deleted					]= No_Content(),
	[+_::Valid						]= Not_Modified(),
	[+_::Changed					]= No_Content(),
	[+_::Content					]= OK(),
	[+_::Accepted					]= Accepted(),
	[+_::No_Content					]= No_Content(),
	[+_::Bad_Request				]= Bad_Request(),
	[+_::Bad_Option					]= Bad_Request(),
	[+_::Unauthorized				]= Unauthorized(),
	[+_::Forbidden					]= Forbidden(),
	[+_::Not_Found					]= Not_Found(),
	[+_::Conflict					]= Conflict(),
	[+_::Length_Required			]= Length_Required(),
	[+_::Payload_Too_Large			]= Payload_Too_Large(),
	[+_::URI_Too_Long				]= URI_Too_Long(),
	[+_::Upgrade_Required			]= Upgrade_Required(),
	[+_::Method_Not_Allowed			]= Method_Not_Allowed(),
	[+_::Not_Acceptable				]= Not_Acceptable(),
	[+_::Precondition_Failed		]= Precondition_Failed(),
	[+_::Request_Entity_Too_Large	]= Payload_Too_Large(),
	[+_::Unsupported_Content_Format	]= Unsupported_Content_Format(),
	[+_::Internal_Server_Error		]= Internal_Server_Error(),
	[+_::Not_Implemented			]= Not_Implemented(),
	[+_::Service_Unavailable		]= Service_Unavailable(),
};

}
