/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurpc.cpp - JSON RPC implementation
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
 * You should have received a copy of the GNU General Public License
 * along with the µcuREST Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */
#include <string.h>
#include "micurpc.hpp"
namespace micurpc {
namespace details {

void copy(char* dst, progmem<char> src, size_t n) noexcept;
void copy(char* dst, const char* src, size_t n) noexcept {
	strncpy(dst,src,n);
}

typedef cojson::details::literal_strings<char> lic;

template<class C>
bool bityped_property<C>::read(C& obj, lexer& in) const noexcept {
	char_t chr;
	if( ! in.skipws(chr) ) return false;
	in.back(chr);
	switch(chr) {
	case lic::null_l()[0]:
		return obj.readnullid(in);
	case literal<>::quotation_mark:
		return obj.readstringid(in);
	case literal<>::minus:
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		return obj.readintid(in);
	//case '0': json integers do not start with 0
	default:
		in.error(error_t::bad);
		return false;
	}
}

template<class C>
bool bityped_property<C>::write(const C& obj, ostream& out) const noexcept {
	switch( obj.idtype ){
	default:
	case C::id_is::bad:
	case C::id_is::null:
		return out.puts(literal<>::null_l());
	case C::id_is::number:
		return out.puts(obj.id);
	case C::id_is::string:
		return writer<const char_t*>::write(obj.id, out);
	}
}

bool service::request::read(response::error_t& err, istream& body) noexcept {
	request::json json;
	lexer in(body);
	if( ! json.read(*this, in) ) {
		err.code = error_code::parse_error;
		return body.error() != error_t::eof;
	} else {
		if( ! match(lit::_2_0(), jsonrpc) || method[0] == 0 ) {
			err.code = error_code::invalid_request;
		} else {
			if( error_t::noerror != ( in.error() & error_t::overrun ) ){
				if( idtype == id_is::bad ) {
					err.code = error_code::id_is_too_long;
					err.message = literal<>::id_is_too_long();
					idtype = id_is::null;
				} else
				if( ! proc ) {
					err.code = error_code::method_is_too_long;
					err.message = literal<>::method_is_too_long();
				} else {
					//FIXME extra parameters cause overrun too
					err.code = error_code::string_is_too_long;
					err.message = literal<>::string_is_too_long();
				}
			}
		}

	}
	return true;
}

bool service::request::readintid(lexer& in) noexcept {
	using cojson::details::ctype;
	size_t i = 0;
	ctype ct = ctype::digit | ctype::sign; /* allow leading sign */
	while( (i < countof(id) ) && +in.get(id[i++],ct) )
		ct = ctype::digit; /* all others are only digits*/
	in.back(id[i-1]);
	id[i-1] = 0;
	if( i == countof(id) ) {
		in.error(error_t::overrun);
		idtype = id_is::bad;
		return in.skip(ct);
	}
	idtype = id_is::number;
	return true;
}

void service::write(message& msg, response& res) noexcept {
/* 	http://www.jsonrpc.org/historical/json-rpc-over-http.html
	3.6.2   Errors													*/
	switch(res.error.code) {
	case error_code::no_errors:
		if( res.idtype == response::id_is::null) {
			/* for JSON-RPC Notification requests, a success response
			 * MUST be an HTTP status code: 204.					*/
			msg.status(micurest::status_t::No_Content);
			return;
		}
		msg.status(micurest::status_t::OK);
		break;
	case error_code::invalid_request:/* 400 -32600 Invalid Request.	*/
		msg.error(micurest::status_t::Bad_Request);
		break;
	case error_code::method_not_found:/*404 -32601 Method not found.*/
		//msg.not_found() not applicable here
		msg.error(micurest::status_t::Not_Found);
		break;
	case error_code::parse_error:	 /* 500 -32700 	Parse error.	*/
	case error_code::invalid_params: /* 500 -32602 Invalid params.	*/
	case error_code::internal_error: /* 500 -32603 Internal error.	*/
	case error_code::server_error:
	default:
		msg.error(micurest::status_t::Internal_Server_Error);
		break;
	}
	msg.set_content_type(media::json);
	res.write(msg.obody());
}

using ioerr_t = cojson::details::error_t;

bool service::handle(istream& in,request& req,response& res) noexcept {
	if( req.read(res.error, in) ) {
		if( nullptr != (res.proc = req.proc) ) {
			if( ioerr_t::noerror == (in.error() &
				( ioerr_t::overrun | ioerr_t::mismatch )) ) {
				return true;
			} else {
				res.error.code = error_code::invalid_params;
			}
		} else {
			res.error.code = error_code::method_not_found;
		}
	} else {
		res.error.code = error_code::parse_error;
	}
	return false;
}

void service::handle(message& msg, request& req) noexcept {
	response res(req.id, req.idtype);
	/* Content-Type SHOULD be 'application/json-rpc' but MAY be
	 * 'application/json' or 'application/jsonrequest'						*/
	//			dbg("\ncontent_type=%d\n",msg.req().content_type);
	if( details::any<media::type>(msg.req().content_type).of(
		media::json_rpc, media::json, media::jsonrequest) ) {
		if( handle(msg.ibody(), req, res) ) {
			run(req.proc, res.error); /********  RUN  ***********************/
		} else {
			// FIXME fragment handling
			if( msg.ibody().eof() && msg.ibody().isgood() ) {
				return;
			}
			/* 3.6.2   Errors  												*/
			switch(res.error.code) {
			default:
			case error_code::parse_error:
			case error_code::invalid_params:
				msg.error(micurest::status_t::Internal_Server_Error);
				break;
			case error_code::method_not_found:
				msg.not_found();
				break;
			}
		}
	} else {
		msg.bad_request();
		res.error.code = error_code::invalid_request;
	}
	write(msg, res);
	if( req.proc ) dispose(req.proc);
}

constexpr typename property<service::request>::node
	service::request::json::props[service::request::json::count];
constexpr typename property<service::response>::node
	service::response::json::success[service::response::json::count];
constexpr typename property<service::response>::node
	service::response::json::fail[service::response::json::count];
}
}
