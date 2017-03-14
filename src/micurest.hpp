/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurest.hpp - µcuREST Library definitions
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
#ifndef MICUREST_HPP_
#define MICUREST_HPP_
#include <stdint.h>
#include "cojson.hpp"
#include "http_01.hpp"
#include "coap.hpp"

namespace micurest {
namespace http = http_01;
using cojson::char_t;
using cojson::size_t;
using cojson::details::ostream;
using cojson::details::istream;
using cojson::details::cstring;
using fstring = cojson::details::name;
using name = cojson::details::name;
using cojson::details::error_t;

namespace details {
using namespace cojson::details;
using pstr = progmem<char>;
size_t strlen(pstr p) noexcept;
size_t strlen(const char* s) noexcept;
}

/******************************************************************************
 *						configuration-related definitions					  *
 ******************************************************************************/

struct default_config {
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

struct config : default_config {
#	include "micurest.config"
};


namespace identity {
	/**
	 * identity - is an identifier non-statically named resources
	 * this namespace defines associated types and built in identity parsers
	 */
	/**
	 * type - storage data type for identity
	 */
	using type=config::identity_t;
	/** parsing dynamically allocated or indexed identities */
	typedef bool (*parser)(const char_t* str, type&);
	bool numeric(const char_t* str, type&) noexcept;
	bool hex(const char_t* str, config::identity_t&) noexcept;
	bool alpha(const char_t* str, config::identity_t&) noexcept;
}


/******************************************************************************
 *						Protocol-related definitions					  	  *
 ******************************************************************************/

/**
 * CoAP and HTTP have different set of statuses
 * This enum establishes protocol-neutral statuses
 * which then are mapped to appropriate protocol-specific values
 */
enum class status_t : uint_fast8_t {
	Unknown,
	OK,
	Created,
	Deleted,
	Valid,
	Changed,
	Content,
	Accepted,
	No_Content,
	Bad_Request, /* assumed to be the first bad status */
	Bad_Option,
	Unauthorized,
	Forbidden,
	Not_Found,
	Conflict,
	Length_Required,
	Payload_Too_Large,
	URI_Too_Long,
	Upgrade_Required,
	Method_Not_Allowed,
	Not_Acceptable,
	Precondition_Failed,
	Request_Entity_Too_Large,
	Unsupported_Content_Format,
	Internal_Server_Error,
	Not_Implemented,
	Service_Unavailable,
	__count_of,
	//TODO decide if these are needed
	Bad_Gateway,
	Gateway_Timeout,
	Proxying_Not_Supported,
};

static inline constexpr size_t operator+(status_t v) noexcept {
	return static_cast<size_t>(v);
}

/**
 * enumeration of supported protocols
 */
enum class protocol {
	UNKNOWN,
	COAP1_0,
	HTTP0_1,
	HTTP1_1,
};


/**
 * media struct encloses constants defining media types
 * Integral constants are used to enable support for both coap/http protocols
 */
struct media {
/* enum class is too restrictive, while the goal is to allow an application
 * defining additional types.
 * simple namespace-level enum is not used to avoid name collisions
 * therefore the enum is enclosed in the struct								*/
	typedef uint_fast8_t type;

	enum : type {
		unknown,
		any
	};
	enum application : type {
		anyapp = any + 1,
		octet_stream,
		json,
		javascript,
		__count_of
	};
	enum text : type {
		anytext = application::__count_of,
		css,
		csv,
		html,
		plain,
		xml,
		xsl,
		_count_of
	};
	static constexpr type count_of = text::_count_of;
	static inline constexpr bool is_text(type t) noexcept {
		return t >= application::__count_of;
	}
};

/* forward declaration */
class application;
namespace resource {
struct node;
struct entry;
}

namespace details {
using namespace resource;

/** implementation specific literal strings									*/
template<typename T = cstring>
struct literal;


/**
 * literal char, if defined as `const char`, an alternate literal
 * template is selected that places literal strings in progmem.
 */

template<>
struct literal<const char*> : http::literal<const char*> {
	/* Error messages */
	static inline constexpr const char* OK() noexcept { return "OK"; }
	static inline constexpr const char* Created() noexcept { return "Created"; }
	static inline constexpr const char* Deleted() noexcept { return "Deleted"; }
	static inline constexpr const char* Valid() noexcept { return "Valid"; }
	static inline constexpr const char* Changed() noexcept { return "Changed"; }
	static inline constexpr const char* Content() noexcept { return "Content"; }
	static inline constexpr const char* Accepted() noexcept { return "Accepted"; }
	static inline constexpr const char* No_Content() noexcept { return "No Content"; }
	static inline constexpr const char* Not_Modified() noexcept { return "Not Modified"; }
	static inline constexpr const char* Bad_Request() noexcept { return "Bad Request"; }
	static inline constexpr const char* Bad_Option() noexcept { return "Bad Option"; }
	static inline constexpr const char* Unauthorized() noexcept { return "Unauthorized"; }
	static inline constexpr const char* Forbidden() noexcept { return "Forbidden"; }
	static inline constexpr const char* Not_Found() noexcept { return "Not Found"; }
	static inline constexpr const char* Conflict() noexcept { return "Conflict"; }
	static inline constexpr const char* Length_Required() noexcept { return "Length Required"; }
	static inline constexpr const char* Payload_Too_Large() noexcept { return "Payload Too Large"; }
	static inline constexpr const char* URI_Too_Long() noexcept { return "URI Too Long"; }
	static inline constexpr const char* Upgrade_Required() noexcept { return "Upgrade Required"; }
	static inline constexpr const char* Method_Not_Allowed() noexcept { return "Method Not_Allowed"; }
	static inline constexpr const char* Not_Acceptable() noexcept { return "Not Acceptable"; }
	static inline constexpr const char* Not_Implemented() noexcept { return "Not Implemented"; }
	static inline constexpr const char* Service_Unavailable() noexcept { return "Service Unavailable"; }
	static inline constexpr const char* Precondition_Failed() noexcept { return "Precondition Failed"; }
	static inline constexpr const char* Request_Entity_Too_Large() noexcept { return "Request Entity Too Large"; }
	static inline constexpr const char* Unsupported_Content_Format() noexcept { return "Unsupported Content Format"; }
	static inline constexpr const char* Internal_Server_Error() noexcept { return  "Internal Server Error";}

	struct map {
		static const char* const message[+status_t::__count_of];
	};
	/* rfc7230#section-3.1.2 says that textual phrase describing the status code
	 * could be empty. in micurest it can be turned off with config.
	 */
	static const char* message(status_t s) {
		return
			config::empty_status_message ? "" :	map::message[+s];
	}

	static const char* content_type(media::type type) {
		return (type < media::application::__count_of) ?
			media_type::application_() :  media_type::text_();
	}
	static const char* content_subtype(media::type type) {
		switch( type ) {
		default:
		case media::unknown:
		case media::octet_stream:
			return media_type::application::octet_stream();
		case media::json:
			return media_type::application::json();
		case media::javascript:
			return media_type::application::javascript();
		case media::css:
			return media_type::text::css();
		case media::csv:
			return media_type::text::csv();
		case media::html:
			return media_type::text::html();
		case media::plain:
			return media_type::text::plain();
		case media::xml:
			return media_type::text::xml();
		case media::xsl:
			return media_type::text::xsl();
		}
	}
	static inline constexpr const char* Error_At() noexcept { return "Error at "; }
	static inline constexpr const char* Error_Near() noexcept { return  " near ";}
};

template<>
struct literal<cojson::details::progmem<char>>
  : http::literal<cojson::details::progmem<char>> {
	typedef cojson::details::progmem<char> pstr;
private:
	static constexpr const char _OK					[]= "OK";
	static constexpr const char _Created			[]= "Created";
	static constexpr const char _Deleted			[]= "Deleted";
	static constexpr const char _Valid				[]= "Valid";
	static constexpr const char _Changed			[]= "Changed";
	static constexpr const char _Content			[]= "Content";
	static constexpr const char _Accepted			[]= "Accepted";
	static constexpr const char _No_Content			[]= "No Content";
	static constexpr const char _Not_Modified		[]= "Not Modified";
	static constexpr const char _Bad_Request		[]= "Bad Request";
	static constexpr const char _Bad_Option			[]= "Bad Option";
	static constexpr const char _Unauthorized		[]= "Unauthorized";
	static constexpr const char _Forbidden			[]= "Forbidden";
	static constexpr const char _Not_Found			[]= "Not Found";
	static constexpr const char _Conflict			[]= "Conflict";
	static constexpr const char _Length_Required	[]= "Length Required";
	static constexpr const char _Payload_Too_Large	[]= "Payload Too Large";
	static constexpr const char _URI_Too_Long		[]= "URI Too Long";
	static constexpr const char _Upgrade_Required	[]= "Upgrade Required";
	static constexpr const char _Method_Not_Allowed	[]= "Method Not_Allowed";
	static constexpr const char _Not_Acceptable		[]= "Not Acceptable";
	static constexpr const char _Not_Implemented	[]= "Not Implemented";
	static constexpr const char _Service_Unavailable[]= "Service Unavailable";
	static constexpr const char _Precondition_Failed[]= "Precondition Failed";
	static constexpr const char _Request_Entity_Too_Large[]=
												"Request Entity Too Large";
	static constexpr const char _Unsupported_Content_Format[]=
												"Unsupported Content Format";
	static constexpr const char _Internal_Server_Error[]=
												"Internal Server Error";

public:
	static inline constexpr pstr nil() noexcept { return pstr(_OK+2); }
	static inline constexpr pstr OK() noexcept { return pstr(_OK); }
	static inline constexpr pstr Created() noexcept { return pstr(_Created); }
	static inline constexpr pstr Deleted() noexcept { return pstr(_Deleted); }
	static inline constexpr pstr Valid() noexcept { return pstr(_Valid); }
	static inline constexpr pstr Changed() noexcept { return pstr(_Changed); }
	static inline constexpr pstr Content() noexcept { return pstr(_Content); }
	static inline constexpr pstr Accepted() noexcept { return pstr(_Accepted); }
	static inline constexpr pstr No_Content() noexcept {
		return pstr(_No_Content);
	}
	static inline constexpr pstr Not_Modified() noexcept {
		return pstr(_Not_Modified);
	}
	static inline constexpr pstr Bad_Request() noexcept {
		return pstr(_Bad_Request); }
	static inline constexpr pstr Bad_Option() noexcept {
		return pstr(_Bad_Option);
	}
	static inline constexpr pstr Unauthorized() noexcept {
		return pstr(_Unauthorized);
	}
	static inline constexpr pstr Forbidden() noexcept {
		return pstr(_Forbidden);
	}
	static inline constexpr pstr Not_Found() noexcept {
		return pstr(_Not_Found);
	}
	static inline constexpr pstr Conflict() noexcept {
		return pstr(_Conflict);
	}
	static inline constexpr pstr Length_Required() noexcept {
		return pstr(_Length_Required);
	}
	static inline constexpr pstr Payload_Too_Large() noexcept {
		return pstr(_Payload_Too_Large);
	}
	static inline constexpr pstr URI_Too_Long() noexcept {
		return pstr(_URI_Too_Long);
	}
	static inline constexpr pstr Upgrade_Required() noexcept {
		return pstr(_Upgrade_Required);
	}
	static inline constexpr pstr Method_Not_Allowed() noexcept {
		return pstr(_Method_Not_Allowed);
	}
	static inline constexpr pstr Not_Acceptable() noexcept {
		return pstr(_Not_Acceptable);
	}
	static inline constexpr pstr Not_Implemented() noexcept {
		return pstr(_Not_Implemented);
	}
	static inline constexpr pstr Service_Unavailable() noexcept {
		return pstr(_Service_Unavailable);
	}
	static inline constexpr pstr Precondition_Failed() noexcept {
		return pstr(_Precondition_Failed);
	}
	static inline constexpr pstr Request_Entity_Too_Large() noexcept {
		return pstr(_Request_Entity_Too_Large);
	}
	static inline constexpr pstr Unsupported_Content_Format() noexcept {
		return pstr(_Unsupported_Content_Format);
	}
	static inline constexpr pstr Internal_Server_Error() noexcept {
		return pstr(_Internal_Server_Error);
	}


	static pstr message(http::status status);

	struct map {
		static pstr const message[+status_t::__count_of];
	};
	/* rfc7230#section-3.1.2 says that textual phrase describing the status code
	 * could be empty. in micurest it can be turned off with config.
	 */
	static pstr message(status_t s) {
		details::progmem<pstr> v(map::message);
		return config::empty_status_message ? nil() : v[+s];
	}

	static pstr content_type(media::type type) {
		return (type < media::application::__count_of) ?
			media_type::application_() :  media_type::text_();
	}
	static pstr content_subtype(media::type type) {
		switch( type ) {
		default:
		case media::unknown:
		case media::octet_stream:
			return media_type::application::octet_stream();
		case media::json:
			return media_type::application::json();
		case media::javascript:
			return media_type::application::javascript();
		case media::css:
			return media_type::text::css();
		case media::csv:
			return media_type::text::csv();
		case media::html:
			return media_type::text::html();
		case media::plain:
			return media_type::text::plain();
		case media::xml:
			return media_type::text::xml();
		case media::xsl:
			return media_type::text::xsl();
		}
	}
};


/**
 * header struct encloses field flags that indicate presence of
 * header field in the request/response
 */
struct header {
	typedef uint_fast8_t type;
	/* flags, indicating presence of header fields */
	enum field : type {
		accept   		= 0x01,
		content_type 	= 0x02,
		content_length	= 0x04,
		if_match		= 0x08,
		if_none_match	= 0x10,
		etag			= 0x20,
		connection		= 0x40,
	};
};

/*
 * Request/Response/Message classes rationales
 *
 * Serialization/deserialization depends on the underlying protocol
 * which can not be implemented on generic level.
 * On the other hand, it is desirable to (1) make applications
 * less tied to the protocol, (2) not lock-up the extension possibilities
 * and (3) still have all those abstractions at affordable cost
 * (resource-wise)
 * With the request class it is more-less straightforward,
 * httpmessage class parses the input stream and fills the
 * request data members, when it encounters the message body,
 * it passes control to the application.
 * With the response it is more complicated.
 * The response object should write status and header fields before
 * the response body, but the actual status is not know until
 * the application really starts writing the response body.
 * request response are designed as PDO
 * message class implements generic behavior
 * httpmessage, coapmessage implement protocol specific behavior and
 * serialization
 */

/**
 * Generic request class
 * Protocol specifics are implemented in concrete message class
 */
class request {
public: /* since it is a PDO, all members are public */
	using identity_t=config::identity_t;
	http::version	version			= http::version::UNKNOWN;
	size_t			content_length	= 0;
	http::method	method 			= http::method::UNKNOWN;
	identity_t 		identity		= {};
	media::type		content_type	= media::unknown;
	header::type 	fields 			= 0;
	uint16_t		accept_			= 0; /* bitmask */
	bool			keep			= false;
public:
	inline bool has(header::type field) const noexcept {
		return fields & field;
	}
	inline void accept(media::type type) noexcept {
		accept_ |= (1<<type);
	}
	bool accepts(media::type type) const noexcept {
		static_assert(media::count_of < sizeof(accept_)*8,
			"accept mask is not wide enough");
		return (accept_ & (1<<type)) != 0;
	}
};

/**
 * Generic response class
 * Data serialization depends
 */
class response {
public: /* since it is a PDO, all members are public */
	size_t 			content_length	= 0;
	status_t 		status 			= status_t::Unknown;
	media::type 	content_type	= media::unknown;
	header::type 	fields 			= 0;
};

/**
 * Protocol Message, aggregates request, response, streams and etag
 */
class message : details::noncopyable {
public:
	static constexpr size_t etag_size = config::etag_size;
	inline message(istream& in, ostream& out) noexcept
	  : input(in), output(out) {}
	/* read-ony access to request */
	inline const request& req() const noexcept {
		return requ;
	}
	/* read-ony access to response */
	inline const response& res() const noexcept {
		return resp;
	}
	inline identity::type & identity() noexcept {
		return requ.identity;
	}
	inline const identity::type & identity() const noexcept {
		return requ.identity;
	}

	inline ostream& add_filed(cstring field) noexcept {
		seal(false);
		write_field(field);
		return output;
	}
	inline istream& ibody() noexcept {
		//TODO ??? make stream unreadable if called too early?
		return input;
	}
	inline void bad_request() noexcept {
		error(status_t::Bad_Request);
	}
	inline void bad_content() noexcept {
		error(status_t::Unsupported_Content_Format);
	}
	inline status_t status() const noexcept { return resp.status; }
	void status(status_t st) noexcept {
		if( resp.status < status_t::Bad_Request )
			resp.status = st;
		if( resp.status == status_t::No_Content ||
			resp.status == status_t::Not_Found ) {
			set_content_type(media::unknown);
			resp.fields = 0;
		}
	}
	inline bool error(status_t st) {
		/* trying not override status with lower priority error */
		if( resp.status < status_t::Bad_Request ||
			st >= status_t::Internal_Server_Error)
			resp.status = st;
		set_content_type(media::unknown);
		resp.fields = 0;
		return false;
	}
	inline void set_content_type(media::type type) noexcept {
		resp.fields |= header::content_type;
		resp.content_type = type;
	}
	/** set content type if not set yet and indeed needed */
	inline void fix_content_type(media::type type) noexcept {
		if( resp.status <= status_t::OK
			&& resp.content_type == media::unknown
			&& type != media::unknown )
			set_content_type(type);
	}
	inline void not_found() noexcept {
		error(status_t::Not_Found);
	}
	inline bool is_nocontent() noexcept { /* returns true is status of no content type */
		return resp.status == status_t::No_Content;
	}
	void set_etag(const char_t* etag) noexcept;
	inline void set_content_length(size_t len) noexcept {
		resp.fields |= header::content_length;
		resp.content_length = len;
	}
	/** seals message envelop and makes it ready
	 * for extra headers (final = false) or or body (final = true)
	 */
	virtual void seal(bool = true) noexcept { }
	virtual ostream& obody() noexcept = 0;
	/**
	 * handles incoming trailing crlf in a protocol specific manner
	 */
	virtual bool consume_crlf() const noexcept { return true; }
	inline bool failed() const noexcept {
		return resp.status >= status_t::Bad_Request;
	}
	inline bool isgood() const noexcept {
		return resp.status >= status_t::OK
			&& resp.status < status_t::Bad_Request;
	}

	/*
	 * This struct addresses POST/PUT fragmentation, when a browser first sends
	 * header and then the payload. Between these two packets the message state
	 * can be saved in an externally provided storage and restored before
	 * processing the payload
	 */
	struct state_pdo {
		request  requ;
		/* response resp; //there is no need to save response    			*/
		char_t etag[etag_size];
	};

	inline void save(state_pdo& dst) const noexcept {
		dst.requ = requ;
		details::assign(dst.etag, etag);
	}

	inline void restore(const state_pdo& src) noexcept {
		requ = src.requ;
		details::assign(etag, src.etag);
	}
	inline bool is_ver_01() const noexcept {
		return requ.version == http::version::_0_1;
	}
protected:
	virtual void write_field(cstring) const noexcept {}
protected:
	istream& input;
	ostream& output;
	request  requ;
	response resp;
	/*TODO decide whether to move etag in request indeed */
	char_t etag[etag_size] = {}; /* etag can be shared to save RAM */
};

/**
 *
 */
class httpmessage : public message {
public:
	/** message states */
	enum class state_t : int_fast8_t {
		/*  request states													*/
		bad = -1,	/** an error occurred while processing this message		*/
		empty,		/** empty message 										*/
		method,		/** method expected next								*/
		target,		/** target expected next								*/
		version,	/** version expected next								*/
		header,		/** request header expected next						*/
		body,		/** message body expected next							*/
		/*  response states													*/
		status,		/** status sent											*/
		fields,		/** in process of sending fields						*/
		sealed,		/** headers sent, body may begin						*/
		began,		/** body started										*/
		done		/** body ended											*/
	};
	typedef char_t delimiters[3];
	struct state_pdo : message::state_pdo {
		state_t state;
		const node* target;
		inline void clear() noexcept {
			*this = {};
		}
		/* return true if empty */
		inline bool operator!() const noexcept {
			return state == state_t::empty;
		}
	};

protected:
	friend class micurest::application;
	using lit  = literal<cstring>;
	using lic  = literal<const char*>;
	using text = lit::media_type::text;
	using texc = lic::media_type::text;
	using app  = lit::media_type::application;
	using appc = lic::media_type::application;

	inline httpmessage(istream& in, ostream& out) noexcept
	  : message(in,out) {}

	state_t parse() noexcept;
	state_t bad_request() noexcept;
	inline bool is_hex(char_t&) noexcept;
	inline char_t pct_decode() noexcept;
	inline char_t read_target(bool start) noexcept;
	static constexpr const delimiters CSLF = { lit::COMMA, lit::SEMI, lit::LF };

protected:
	struct map {
		static const unsigned short status[+status_t::__count_of];
	};
	bool keep() const noexcept;
	inline bool crlf() const noexcept {
		return
			put(lit::CR) &&
			put(lit::LF);
	}
	void seal(bool final) noexcept {
		if( state >= state_t::sealed ) return;
		if( final && resp.status == status_t::Unknown )
			resp.status = status_t::OK;
		write_status();
		write_headers();
		if( final ) {
			state = state_t::sealed;
			if( is_nocontent() )
				state = state_t::done;
			else
				crlf();
		}
	}

	ostream& obody() noexcept {
		seal(true);
		state = state_t::began;
		return output;
	}

	inline bool hasbody() const noexcept {
		return state >= state_t::began;
	}

	inline bool sealed() const noexcept {
		return state >= state_t::sealed;
	}
	inline void write_version() noexcept {
		put(lit::HTTP::HTTP_());
		put(lit::TSEP);
		switch( requ.version ) {
		case http::version::_0_1: put(lit::HTTP::_0_1()); break;
		case http::version::_1_0: put(lit::HTTP::_1_0()); break;
		case http::version::UNKNOWN:
		case http::version::_1_1: put(lit::HTTP::_1_1()); break;
		}
		put(lit::SPACE);
	}

	inline void write_status() noexcept {
		if( state >= state_t::status ) return;
		state = state_t::status;
		write_version();
		cojson::details::writer<unsigned short>::write(
			map::status[+resp.status], output);
		put(lit::SPACE);
		put(micurest::details::literal<>::message(resp.status));
		crlf();
	}

	/*
	 rfc7230#section-3.3.3#7
	 7.  Otherwise, this is a response message without a declared message
	       body length, so the message body length is determined by the
	       number of octets received prior to the server closing the
	       connection.
	*/

	inline void write_content_length() noexcept {
		write_field(lit::Content_Length());
		cojson::details::writer<size_t>::write(
				static_cast<size_t>(resp.content_length), output);
		crlf();

	}

	inline void write_content_type() noexcept {
		if( resp.content_type == media::unknown ) return;
		write_field(lit::Content_Type());
		put(lit::content_type(resp.content_type));
		put(lit::TSEP);
		put(lit::content_subtype(resp.content_type));
		crlf();
	}

	inline void write_connection() noexcept {
		write_field(lit::Connection());
		put(keep() ? lit::keep() : lit::close() );
		crlf();
	}

	inline void write_headers() noexcept {
		if( state >= state_t::fields ) return;
		state = state_t::fields;
		if( resp.status != status_t::No_Content ) {
			if( resp.fields & header::content_type )
				write_content_type();
			if( resp.fields & header::content_length )
				write_content_length();
		}
		write_connection();
		if( resp.fields & header::etag ) {
			write_field(lit::ETag());
			put(etag);
			crlf();
		}
	}

	inline void write_field(cstring field) const noexcept {
		put(field);
		put(lit::COLON);
		put(lit::SPACE);
	}

	static inline constexpr bool equals(const httpmessage::delimiters& dlm,
									    char_t chr)	noexcept {
		static_assert(details::countof(dlm) == 3,"revisit implementation");
		return dlm[0] == chr || dlm[1] == chr || dlm[2] == chr;
	}
	static inline constexpr bool equals(char_t chr,
								 const httpmessage::delimiters& dlm) noexcept {
		return equals(dlm, chr);
	}
	inline void save(state_pdo& dst) const noexcept {
		message::save(dst);
		dst.state = state;
		dst.target = target;
	}

	inline void restore(const state_pdo& src) noexcept {
		message::restore(src);
		state  = src.state;
		target = src.target;
	}

private:
	inline state_t read_method() noexcept;
	state_t read_version() noexcept;
	inline state_t read_headers() noexcept;
	inline bool read_condition() noexcept;
	bool read_connection() noexcept;
	bool literal1(cstring, const delimiters& = {lit::LF}) noexcept;
	bool literal1_ic(cstring, const delimiters& = {lit::LF}) noexcept;
	unsigned char literals(cstring, cstring,
						   const delimiters& = {lit::LF}) noexcept;
	unsigned char literal3(cstring, cstring, cstring,
						   const delimiters& = {lit::LF}) noexcept;
	inline bool read_etag() noexcept;
	bool read_accept() noexcept;
	media::type read_mediatype() noexcept;
	inline media::type read_mediatypetext() noexcept;
	inline media::type read_mediatypeapp() noexcept;
	bool read_length() noexcept {
		details::lexer in(input);
		return details::reader<size_t>::read(requ.content_length, in);
	}

	bool expect_crlf() noexcept;
	bool consume_crlf() noexcept;

	inline bool get() noexcept {
		return input.get(curr);
	}
	inline bool put(char_t v) const noexcept {
		return output.put(v);
	}
	inline bool put(const char_t* v) const noexcept {
		return output.puts(v);
	}
	inline bool put(details::progmem<char> i) const noexcept {
		while(*i && output.put(*i++) );
		return output.isgood();
	}

	inline bool got(char_t& chr) noexcept {
		if( get() ) {
			chr = curr;
			return true;
		} else {
			bad_request();
			return false;
		}
	}

	void skip_ws() noexcept {
		while( get() && lit::is_space(curr) );
	}
	bool skip_line() noexcept {
		while(get() && lit::is_linear(curr));
		return expect_crlf();
	}
	void skip_type() noexcept {
		while(get() && lit::is_type(curr));
	}
protected:
	char_t curr   = 0;
	state_t state = state_t::method;
	const node* target;
	bool maykeep  = false;
};

/** Finds an CER entry with unknown status */
template<typename ... L>
struct find_unknown;

template<typename T, typename ... L>
struct find_unknown<T,L...> {
	static constexpr size_t index
		= T::status == status_t::Unknown
		? 0
		: 1 + find_unknown<L...>::index;
};

template<typename T>
struct find_unknown<T> {
	/* last element is the last resort too */
	static constexpr size_t index = 0;
};

template<typename T>
static inline bool convert(const char_t* str, T& dst) noexcept {
	T val = {};
	while(*str) {
		if( +(cojson::details::chartype(*str) & cojson::details::ctype::digit) ) {
			if( ! tenfold<decltype(val)>(val,*str-'0') ) return false;
//			if( val > 0xFF ) return false;
		} else {
			return false;
		}
		++str;
	}
	dst = val;
	return true;
}

/**
 * simple map K=V of fixed size N
 * V should have means for detecting its emptiness by operator!()==true
 * K should have means for comparing key1 == key2
 */
template<typename K, typename V, size_t N>
class map {
public:
	/* finds a slot corresponding to key or returns an empty slot */
	template<typename T>
	V* find(const T& key) noexcept {
		for(auto& i : tuples) {
			if( !!i.val && i.key == key )
				return &i.val;
		}
		for(auto& i : tuples) {
			if( ! i.val ) {
				i.key = key;
				return &(i.val);
			}
		}
		return nullptr;
	}
private:
	struct tuple {
		K key;
		V val;
	} tuples[N];
};
}

namespace resource {
/******************************************************************************
 *							Structure-related definition					  *
 ******************************************************************************/

/*
 * µcuREST implements facilitates a resource map for organizing
 * exposed application resources into a set of Uniformly Identified Resources
 * Definitions
 *  Resource  - a content that application provides on a request.
 *				This includes static (compile time generated) and
 *				dynamic (run-time generated) content
 *
 * µcuREST Resource Map has three main entities:
 *  Node 	  - a Resource Provider class implementing data access for a given
 *  			resource and serializing/deserializing data
 *				This includes static (compile time generated) and
 *	Entry	  - a named node
 *  Name      - a part of URI, associated with the entry
 *
 *	There are also two derived entities:
 *	File	  - Combination if an Entry and a Node
 *	Directory - Entry with a collection of sub-entries
 *				Also may provide resources for custom error responses
 *
 * µcuREST Resource Providers implement the following HTTP methods:
 * +------------+----------------+
 * | Directory  | GET            |
 * | Resource   | GET, POST, PUT |
 * +------------+----------------+
 *
 * HTTP/0.1 is an application-layer protocol
 * It does not imply presence of any lower-level protocols, such as TCP
 */


/**
 * fnode - unnamed resource with content
 */
typedef const node&  (*fnode)();

/**
 * inode - indexed resource with content
 */
typedef const node&  (*inode)(const config::identity_t&);


/**
 * fentry - a named node
 */
typedef const entry& (*fentry)();

/**
 * content provider
 */
typedef void (*provider)(const details::message&, ostream&);


/**
 * node_base : minimal base class for a node
 * By default nodes implement only GET and PUT methods
 */
struct node_base {
public:
	enum class type_t {
		file,
		dir
	};
	static constexpr bool handle_post_as_put_get=config::handle_post_as_put_get;
	/**
	 * processes message by dispatching method and returns
	 */
	void process(details::message& msg) const noexcept {
		switch(msg.req().method) {
		case http::method::PUT:
			put(msg);
			if( msg.status() == status_t::Unknown )
				msg.status(status_t::No_Content);
			return;
		case http::method::POST:
			if( ! handle_post_as_put_get ) break;
			put(msg);
			if( ! msg.isgood() ) return;
		case http::method::GET:
			get(msg);
			if( msg.status() == status_t::Unknown )
				msg.status(status_t::OK);
			return;
		default:;
		}
		msg.error(status_t::Method_Not_Allowed);
	}
	virtual const node* find(const char_t*, details::message&) const noexcept {
		return nullptr;
	}
	virtual media::type mediatype() const noexcept {
		return media::unknown;
	}
	virtual bool accept(media::type type) const noexcept {
		return mediatype() == type;
	}
	virtual void get(details::message& msg) const noexcept {
		msg.error(status_t::Not_Implemented);
	}
	virtual void put(details::message& msg) const noexcept {
		msg.error(status_t::Not_Implemented);
	}
	virtual type_t type() const noexcept { return type_t::file; }
	inline bool isdir() const noexcept { return type() == type_t::dir; }
};

/**
 * node_extended : minimal base class for a node
 * Configurable extension of node base to support POST/DELETE
 */
struct node_extended : node_base {
public:
	void process(details::message& msg) const noexcept {
		switch(msg.req().method) {
		case http::method::GET:
			get(msg);
			break;
		case http::method::PUT:
			put(msg);
			break;
		case http::method::POST:
			post(msg);
			break;
		case http::method::DELETE:
			del(msg);
			break;
		default:
			msg.error(status_t::Method_Not_Allowed);
		}
	}
	virtual void post(details::message& msg) const noexcept {
		put(msg);
		if( handle_post_as_put_get && msg.isgood() ) get(msg);
	}
	virtual void del(details::message& msg) const noexcept {
		msg.error(status_t::Method_Not_Allowed);
	}
};

/**
 * Resource Map Node - an abstract content provider
 */
struct node : std::conditional<
	config::extended_node, details::node_extended, details::node_base>::type {};

/**
 * Directory Node - implements resource feeding by content type
 * specified in Accept to a first matching node in the directory
 */
struct directory : node {
	virtual const node* find(const details::message&) const noexcept = 0;
	type_t type() const noexcept { return type_t::dir; }

	void get(details::message& msg) const noexcept {
		/* nothing can be offered if client has not specified
		 * accepted content type */
		if( ! msg.req().has(details::header::accept) )
			return msg.not_found();
		const node* anode = find(msg);
		if( anode != nullptr ) {
			msg.set_content_type(anode->mediatype());
			anode->get(msg);
			return;
		}
		msg.not_found();
	}
};

/**
 * Resource Map Entry
 */
struct entry {
public:
	virtual bool matches(const char_t*, details::message& msg) const noexcept = 0;
	virtual const node& getnode(const details::message& msg) const noexcept = 0;
};


/**
 * Custom Error Response Mapper
 * Provides mapping of status codes to nodes
 * Node with unknown status will be returned if no other candidate found
 * Usage:
 *   single entry   : cer::e<status_t::whatever, myfnode>
 *   list of entries: cer::l<cer::e<status_t::whatever, myfnode>,...>
 */
struct cer {
	/** List of CER entries */
	template<typename ... L>
	struct l {
		static const node* find(status_t status) noexcept {
			static constexpr const size_t unknown =
					details::find_unknown<L...>::index;
			static constexpr const size_t count = sizeof...(L);
			static constexpr const cer list[count] = { L() ... };
			for(size_t i = 0; i < count; ++i) {
				if( list[i].status == status )
					return &(list[i].anode());
			}
			return &(list[unknown].anode());
		}
	};
	/** CER entry */
	template<status_t S,details::fnode N>
	struct e {
		static constexpr status_t status = S;
		static constexpr details::fnode anode = N;
		constexpr e() {}
		static constexpr const node* find(status_t status) noexcept {
			return &N();
		}
	};
	template<typename T>
	constexpr cer(T&&) : status(T::status), anode(T::anode) {}
	const status_t status;
	const details::fnode anode;
};

/****************************************************************************
 * Implementation of content providers										*
 ****************************************************************************/
/**
 * text - a generic plain/text content provider
 * If content length is not specified, only one line
 * is processed by PUT
 */
struct text : node {
public:
	media::type mediatype() const noexcept {
		return media::plain;
	}

	void put(details::message& msg) const noexcept {
		if( msg.req().content_type != mediatype() ) {
			msg.bad_content();
			return;
		}
		size_t n;
		typedef std::make_unsigned<char_t>::type char_u;
		char_u eol = 0;
		if( msg.req().has(details::header::content_length) ) {
			n = msg.req().content_length;
			if( n >= size() ) {
				msg.error(status_t::Request_Entity_Too_Large);
				return;
			}
		} else {
			n = size();
			eol = ' ';
		}

		istream& in(msg.ibody());
		char_t data;
		size_t i;
		for(i = 0; i < n && in.get(data) &&
			(static_cast<char_u>(data) >= eol || data == '\t'); ++i)
			putc(i,data);
		/* too long sting read */
		if( i >= size() ) {
			msg.error(status_t::Request_Entity_Too_Large);
			return;
		}
		/* data ended unexpectingly or not properly */
		if(	 ! msg.consume_crlf()
			|| in.error() != error_t::noerror
			|| (i < n && msg.req().has(details::header::content_length)) )
			msg.bad_request();
		else {
			putc(++i,0);
			msg.status(status_t::No_Content);
		}
	}
	void get(details::message& msg) const noexcept {
		size_t n;
		msg.status(status_t::OK);
		msg.set_content_type(mediatype());
		msg.set_content_length(n = length());
		msg.seal();

		ostream& out(msg.obody());
		const char_t* buff = gets();
		if( buff ) 
			out.puts(buff);
		else
			for(size_t i = 0; i < n; ++i) out.put(getc(i));
	}
	/**
	 * Returns data size (in bytes) for GET
	 */
	virtual size_t length() const noexcept = 0;
	/**
	 * Returns maximal size (in bytes) available for PUT
	 */
	virtual size_t size() const noexcept = 0;
	/**
	 * Returns one byte from application data
	 */
	virtual char_t getc(size_t) const noexcept = 0;
	/**
	 * Returns zero-terminated buffer
	 */
	virtual const char_t* gets() const noexcept { return nullptr; }
	/**
	 * stores one byte to application data
	 */
	virtual void putc(size_t, char_t) const noexcept {}

	/**
	 * Called before sealing the message
	 * for setting additional header fields
	 */
	virtual void header(details::message&) const noexcept {}
};


/**
 * json - a generic JSON content provider
 */
struct json : node {
public:
	media::type mediatype() const noexcept {
		return media::json;
	}
};

/**
 * octets - a generic octet-stream content provider
 * On GET streams data out
 * On PUT reads data from the message and places it in the array
 *   Requires Content-Length to be specified
 */
struct octets : text {
public:
	media::type mediatype() const noexcept {
		return media::octet_stream;
	}
	virtual void length(size_t) const noexcept = 0;
	void put(details::message& msg) const noexcept {
		if( msg.req().content_type != mediatype() ) {
			msg.bad_content();
			return;
		}
		if( ! msg.req().has(details::header::content_length) ) {
			msg.error(status_t::Length_Required);
			return;
		}
		size_t n = msg.req().content_length;
		if( n > size() ) {
			msg.error(status_t::Request_Entity_Too_Large);
			return;
		}

		istream& in(msg.ibody());
		char_t data;
		size_t i;
		for(i = 0; i < n && in.get(data); ++i)
			putc(i,data);
		length(i);
		/* data ended unexpectingly */
		if( i < n )
			msg.bad_request();
		else
			msg.status(status_t::No_Content);
	}
};


/**
 * numeric - plain text content provider for primitive numeric data types
 * accessible via accessor X (cojson::accessor)
 * On GET serializes data as plain text
 * On PUT deserializes text to value,
 *   if S=OK calls GET, otherwise sets S as the response status
 */
template<class X>
struct numeric : json { /* numeric uses JSON lexer/reade/writer  */
	using T = typename X::type;
	void put(details::message& msg) const noexcept {
		if( msg.req().content_type != mediatype() ) {
			msg.bad_content();
			return;
		}
		details::lexer in(msg.ibody());
		if( X::canlref && X::has() ) {
			//TODO coap stream should be limited by field size to prevent
			//reader<T>::read from reading extra bytes
			if( ! details::reader<T>::read(X::lref(), in) /* || ! msg.consume_crlf() */ ) {
				msg.bad_content();
				return;
			}
		} else if( X::canset ) {
			T v;
			X::init(v);
			if( details::reader<T>::read(v, in) /* && msg.consume_crlf() */ ) {
				X::set(v);
			} else {
				msg.bad_content();
				return;
			}
		} else {
			msg.error(status_t::Method_Not_Allowed);
			return;
		}
		msg.status(status_t::No_Content);
	}
	void get(details::message& msg) const noexcept {
		if( X::has() ) {
			if( X::canrref ) {
				msg.status(status_t::OK);
				details::writer<T>::write(X::rref(), msg.obody());
				/* trailing crlf is protocol specific */
				return;
			} else if( X::canget ) {
				msg.status(status_t::OK);
				details::writer<T>::write(X::get(), msg.obody());
				return;
			}
		}
		msg.error(status_t::Method_Not_Allowed);
	}
};

/****************************************************************************
 *						Resource mapping templates							*
 ****************************************************************************/
/**
 * EntryGenericResource - entry, a generic named resource
 */
template<name id, details::fnode I>
inline const entry& EntryGenericResource() noexcept {
	static const struct local : entry {
		const node& getnode(const details::message&) const noexcept {
			return I();
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(),str);
		}
	} l;
	return l;
}

/** EntryIndexedResource
 *  entry, a dynamically allocated or indexed resource
 *  with homogeneous structure for all indexable resources
 */
template<identity::parser parser, details::fnode I>
inline const entry& EntryIndexedResource() noexcept {
	static const struct local : entry {
		const node& getnode(const details::message&) const noexcept {
			return I();
		}
		bool matches(const char_t* str, details::message& msg) const noexcept {
			return parser(str, msg.identity());
		}
	} l;
	return l;
}

/** EntryDynamicResource
 *  entry, a dynamically allocated or indexed resource
 *  with heterogeneous structure for indexable resources
 */
template<identity::parser parser, details::inode I>
inline const entry& EntryDynamicResource() noexcept {
	static const struct local : entry {
		const node& getnode(const details::message& msg) const noexcept {
			return I(msg.requ.identity);
		}
		bool matches(const char_t* str, details::message& msg) const noexcept {
			return parser(str, msg.requ.identity);
		}
	} l;
	return l;
}

/**
 * NodeTextFuction - a text file of type M with content returned by V
 */
template<media::type M, fstring V>
inline const node& NodeTextFuction() noexcept {
	static const struct local : details::text {
		media::type mediatype() const noexcept {
			return M;
		}
		void put(details::message& msg) const noexcept {
			msg.error(status_t::Method_Not_Allowed);
		}
		size_t length() const noexcept { return details::strlen(V()); }
		size_t size() const noexcept { return 0; }
		char_t getc(size_t i) const noexcept { return V()[i]; }
		const char_t* gets() const noexcept {
			return std::is_same<cstring,const char_t*>::value ?
				(const char_t*)V() : nullptr;
		}
	} l;
	return l;
}

/**
 * NodeProvider - a text file of type M with content written by P
 */
template<media::type M, provider P>
inline const node& NodeProvider() noexcept {
	static const struct local : node {
		media::type mediatype() const noexcept {
			return M;
		}
		void put(details::message& msg) const noexcept {
			msg.error(status_t::Method_Not_Allowed);
		}
		void get(details::message& msg) const noexcept {
			msg.set_content_type(M);
			msg.status(status_t::OK);
			P(msg, msg.obody());
		}
	} l;
	return l;
}

/**
 * NodeAction - an action target, accepting only put method with optional
 * text data of maximal length L
 */
template<status_t (*F)(const char_t *),  size_t L = 0>
inline const node& NodeAction() noexcept {
	static const struct local : node {
		media::type mediatype() const noexcept {
			return media::plain;
		}
		void put(details::message& msg) const noexcept {
			if( msg.req().content_type != mediatype() ) {
				msg.bad_content();
				return;
			}
			size_t n;
			typedef std::make_unsigned<char_t>::type char_u;
			char_u eol = 0;
			if( L && msg.req().has(details::header::content_length) ) {
				n = msg.req().content_length;
				if( n >= L ) {
					msg.error(status_t::Request_Entity_Too_Large);
					return;
				}
			} else {
				n = L;
				eol = ' ';
			}

			istream& in(msg.ibody());
			char_t buff[L];
			char_t *data = buff;
			char_t *end  = buff+L;
			for(; n && data < end && in.get(*data) && *data >= ' '; ++data, --n);
			/* too long sting read */
			if( data >= end ) {
				msg.error(status_t::Request_Entity_Too_Large);
				return;
			}
			*data = 0;
			msg.status(F(buff));
		}	} l;
	return l;
}

/**
 * an r/w JSON object
 */
template<cojson::details::item V>
inline const node& NodeJSON() noexcept {
	static const struct local : details::json {
		void get(details::message& msg) const noexcept {
			msg.status(status_t::OK);
			V().write(msg.obody());
		}
		void put(details::message& msg) const noexcept {
			if( msg.req().content_type == mediatype() ) {
				cojson::lexer in(msg.ibody());
				if( V().read(in) ) {
					msg.status(status_t::No_Content);
					return;
				}
			}
			msg.bad_content();
		}
	} l;
	return l;
}

/**
 * An indexable node bound to a vector via accessor X
 */
template<class X>
inline const node& NodeIndexedVector() noexcept {
	static_assert(X::is_vector, "X must be a vector or array");
	using writer = details::writer<typename X::type>;
	using reader = details::reader<typename X::type>;

	static const struct local : details::json {
		void get(details::message& msg) const noexcept {
			if( X::has(msg.identity()) ) {
				if( X::canrref ) {
					msg.status(status_t::OK);
					msg.set_content_type(mediatype());
					writer::write(X::rref(msg.identity()), msg.obody());
					return;
				} else if( X::canget ) {
					msg.status(status_t::OK);
					msg.set_content_type(mediatype());
					writer::write(X::get(msg.identity()), msg.obody());
					return;
				}
				msg.status(status_t::Method_Not_Allowed);
			} else {
				msg.status(status_t::Not_Found);
			}
		}
		void put(details::message& msg) const noexcept {
			if( msg.req().content_type != mediatype() ) {
				msg.bad_content();
				return;
			}
			cojson::details::lexer in(msg.ibody());
			if( ! X::has(msg.identity()) ) {
				msg.error(status_t::Not_Found);
				return;
			}
			if( X::canlref ) {
				if( ! reader::read(X::lref(msg.identity()), in) ) {
					msg.bad_content();
					return;
				}
				msg.status(status_t::No_Content);
			} else if( X::canset ) {
				typename X::type v;
				X::init(v);
				if( reader::read(v, in) ) {
					X::set(msg.identity(), v);
					msg.status(status_t::No_Content);
				} else {
					msg.bad_content();
					return;
				}
			} else {
				msg.error(status_t::Method_Not_Allowed);
				return;
			}
		}
	} l;
	return l;
}

/** FileAccessor
 * A text/plain file for reading/writing a numeric variable
 * accessible via accessor X.
 */
template<name id, class X>
inline const entry& FileAccessor() noexcept {
	static const struct local : entry {
		details::numeric<X> file;
		const node& getnode(const details::message&) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/** FileVariable
 * A text/plain file for reading/writing a numeric variable
 * accessible via pointer
 */
template<name id, typename T, T* V, status_t S = status_t::No_Content>
inline const entry& FileVariable() noexcept {
	static const struct local : entry {
		details::numeric<cojson::accessor::pointer<T,V>> file;
		const node& getnode(const details::message&) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/**
 * A text/plain file for reading/writing a numeric variable
 * accessible via functions get/put
 */
template<name id, typename T, T (*G)() noexcept, void (*P)(T) noexcept>
inline const entry& FileFunctions() noexcept {
	static const struct local : entry {
		details::numeric<cojson::accessor::functions<T,G,P>> file;
		const node& getnode(const details::message&) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/** FileConstString
 * A text/plain file for reading constant strings
 * accessible via function V.
 */
template<name id, fstring V, media::type M = media::plain>
inline const entry& FileConstString() noexcept {
	struct textfile : details::text {
		cstring str = V();
		size_t size() const noexcept { return 0; }
		size_t length() const noexcept {
			return details::strlen(str);
		}
		media::type mediatype() const noexcept {
			return M;
		}
		char_t getc(size_t i) const noexcept {
			return static_cast<char_t>(str[i]);
		}
	};
	static const struct local : entry {
		textfile file;
		const node& getnode(const details::message& msg) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message& msg) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/**
 * FileText - A text file
 * accessible via reference V.
 */
template<name id, size_t N, char_t (&V)[N]>
inline const entry& FileText() noexcept {
	struct text : details::text {
		size_t length() const noexcept { return details::strlen(V); }
		char_t getc(size_t i) const noexcept { return V[i]; }
		size_t size() const noexcept { return N; }
		void putc(size_t i , char_t c) const noexcept { V[i] = c ; }
	};
	static const struct local : entry {
		text file;
		const node& getnode(const details::message&) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/** FileBinary
 * A octet-stream file for reading/writing binary data
 * accessible via reference V.
 */
template<name id, size_t* L, size_t N, unsigned char (&V)[N]>
inline const entry& FileBinary() noexcept {
	struct binary : details::octets {
		size_t length() const noexcept { return *L; }
		char_t getc(size_t i) const noexcept { return V[i]; }
		size_t size() const noexcept { return N; }
		void length(size_t l) const noexcept { *L = l; }
		void putc(size_t i , char_t c) const noexcept { V[i] = c; }
	};
	static const struct local : entry {
		binary file;
		const node& getnode(const details::message&) const noexcept {
			return file;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}

/** Dir
 * a generic directory entry with a list of nested entries
 */
template<name id, details::fentry ... L>
inline const entry& Dir() noexcept {
	static constexpr const fentry entries[] = { L... };
	static constexpr const size_t count = sizeof...(L);
	static const struct dir : directory {
		const node* find(const char_t* name, details::message& msg) const noexcept {
			for(size_t i = 0; i < count; ++i) {
				if( entries[i]().matches(name, msg) )
					return &((entries[i])().getnode(msg));
			}
			return nullptr;
		} //TODO make optional entity lookup by content type in a directory request
		const node* find(const details::message& msg) const noexcept {
			const node* anode;
			for(size_t i = 0; i < count; ++i) {
				anode = &(entries[i]().getnode(msg));
				if( msg.req().accepts(anode->mediatype()) ) {
					return anode;
				}
			}
			return nullptr;
		}
	} dir;

	static const struct local : entry {
		const node& getnode(const details::message&) const noexcept {
			return dir;
		}
		bool matches(const char_t* str, details::message&) const noexcept {
			return details::match(id(), str);
		}
	} l;
	return l;
}
}

/****************************************************************************
 *						Resource mapping shortcuts							*
 ****************************************************************************/

/*
 * Template Naming Legend
 * E - entry (generic named resource)
 * N - node (unnamed resource)
 * F - file entry (named resource with specific content)
 * D - directory entry
 *
 * Unnamed nodes can be used in E or cer templates
 */


/**
 * E - entry, a generic named resource
 */
template<name id, details::fnode I>
const resource::entry& E() noexcept {
	return resource::EntryGenericResource<id,I>();
}

/**
 * E -  entry, a dynamically allocated or indexed resource
 *      with homogeneous structure for all indexable resources
 */
template<identity::parser parser, resource::fnode I>
const resource::entry& E() noexcept {
	return resource::EntryIndexedResource<parser, I>();
}

/**
 * E - entry, a dynamically allocated or indexed resource
 *     with heterogeneous structure for indexable resources
 */
template<identity::parser parser, resource::inode I>
const resource::entry& E() noexcept {
	return resource::EntryDynamicResource<parser, I>();
}
/**
 * N a text file of type M with content returned by V
 */
template<media::type M, fstring V>
const resource::node& N() noexcept {
	return resource::NodeTextFuction<M,V>();
}


/**
 * N - a text file of type M with content written by P
 */
template<media::type M, resource::provider P>
const resource::node& N() noexcept {
	return resource::NodeProvider<M,P>();
}

/**
 * N - an action target, accepting only put method with optional
 * text data of maximal length L
 */
template<status_t (*F)(const char_t *),  size_t L = 0>
const resource::node& N() noexcept {
	return resource::NodeAction<F,L>();
}
/**
 * an r/w JSON object
 */
template<cojson::details::item V>
const resource::node& N() noexcept {
	return resource::NodeJSON<V>();
}

/**
 * An indexable node bound to a vector via accessor X
 */
template<class X>
const resource::node& N() noexcept {
	return resource::NodeIndexedVector<X>();
}

/** F
 * A text/plain file for reading/writing a numeric variable
 * accessible via accessor X.
 */

template<name id, class X>
const resource::entry& F() noexcept {
	return resource::FileAccessor<id,X>();
}

/** F
 * A text/plain file for reading/writing a numeric variable
 * accessible via pointer
 */

template<name id, typename T, T* V, status_t S = status_t::No_Content>
const resource::entry& F() noexcept {
	return resource::FileVariable<id, T, V, S>();
}

/** F
 * A text/plain file for reading/writing a numeric variable
 * accessible via functions get/put
 */

template<name id, typename T, T (*G)() noexcept, void (*P)(T) noexcept>
const resource::entry& F() noexcept {
	return resource::FileFunctions<id,T,G,P>();
}

/** F
 * A text/plain file for reading constant strings
 * accessible via function V.
 */
template<name id, fstring V>
const resource::entry& F() noexcept {
	return resource::FileConstString<id,V>();
}

/**
 * A octet-stream file for reading/writing binary data
 * accessible via reference V.
 */
template<name id, size_t* L, size_t N, unsigned char (&V)[N]>
const resource::entry& F() noexcept {
	return resource::FileBinary<id,L,N,V>();
}

/**
 * A text file
 * accessible via reference V.
 */
template<name id, size_t N, char_t (&V)[N]>
const resource::entry& F() noexcept {
	return resource::FileText<id,N,V>();
}

/**
 * D - a generic directory entry with a list of nested entries
 */
template<name id, resource::fentry ... L>
const resource::entry& D() noexcept {
	return resource::Dir<id, L...>();
}

/**
 * Root - an unnamed directory
 */
template<resource::fentry ... L>
const resource::directory& Root() noexcept {
	static constexpr const resource::fentry entries[] = { L... };
	static constexpr const size_t count = sizeof...(L);
	static const struct local : resource::directory {
		const resource::node* find(const char_t* name,
										details::message& msg) const noexcept {
			for(size_t i = 0; i < count; ++i) {
				if( entries[i]().matches(name, msg) )
					return &((entries[i])().getnode(msg));
			}
			return nullptr;
		}
		const resource::node* find(const details::message& msg) const noexcept {
			const node* anode;
			for(size_t i = 0; i < count; ++i) {
				anode = &(entries[i]().getnode(msg));
				if( msg.req().accepts(anode->mediatype()) ) {
					return anode;
				}
			}
			return nullptr;
		}
	} l;
	return l;
}


/**
 * Root - a directory with a [list] of custom error responses
 * 	class E must implement:
 * 		static const node* find(status_t) noexcept;
 * 	Available implementations are: cer::l or cer::e
 */
template<class E, resource::fentry ... L>
const resource::directory& Root() noexcept {
	static constexpr const resource::fentry entries[] = { L... };
	static constexpr const size_t count = sizeof...(L);
	static const struct local : resource::directory {
		const resource::node* find(const char_t* name,
										details::message& msg) const noexcept {
			for(size_t i = 0; i < count; ++i) {
				if( entries[i]().matches(name, msg) )
					return &((entries[i])().getnode());
			}
			return nullptr;
		}
		const resource::node* find(const details::message& msg) const noexcept {
			if( msg.res().status >= status_t::Bad_Request  )
				return E::find(msg.res().status);
			const node* anode;
			for(size_t i = 0; i < count; ++i) {
				anode = &(entries[i]().getnode());
				if( msg.req().accepts(anode->mediatype()) ) {
					return anode;
				}
			}
			return nullptr;
		}
	} l;
	return l;
}


namespace accessor { //FIXME move to cojson
/**
 * Vector item accessor via getter/setter functions
 */
template<typename T,
	T (*G)(size_t) noexcept,		/* getter */
	void (*S)(size_t, T) noexcept,	/* setter */
	bool (*H)(size_t)>				/* tester */
struct vector {
	typedef T clas;
	typedef T type;
	static constexpr bool canget = true; /* array is accessible for reading */
	static constexpr bool canset = true;
	static constexpr bool canlref   = false;
	static constexpr bool canrref   = false;
	static constexpr bool is_vector = true;
	static constexpr inline bool has(size_t n) noexcept { return H(n); }
	static inline const T get(size_t i) noexcept { return G(i); }
	static T& lref(size_t i) noexcept;
	static const T& rref(size_t i) noexcept;
	static inline void set(size_t i, const T & v) noexcept { S(i, v);  }
	//static inline void init(size_t,T &) noexcept { }
	static inline void init(T&) noexcept {}
	static inline constexpr bool null(cojson::void_t) noexcept {
		return false;
	}
private:
	vector();
};
/**
 * Vector item accessor via getter function
 */
template<typename T,
	T (*G)(size_t) noexcept,		/* getter */
	bool (*H)(size_t)>				/* tester */
struct bunch : vector<T,G,nullptr,H>{
	static constexpr bool canset = false;
	static void set(size_t i, const T & v) noexcept;
private:
	bunch();
};

}

/*
 *TODO: use of etag
 *TODO: OPTIONS method for letting client to detect one of COAP/HTTP
 *TODO: add indication of response encoding for preencoded files
 */

/******************************************************************************
 *							Application-related definition					  *
 ******************************************************************************/
class application {
public:
	enum class result_t : uint_fast8_t {
		error	= 0, /* an i/o error encountered 					*/
		bad		= 1, /* bad request 								*/
		fragment= 2, /* only fragment of the message was processed	*/
		keep	= 3, /* success, advised to keep connection open 	*/
		close	= 4, /* success, requested to close connection   	*/
	};
//	typedef bool result_t; /* indicates if connection should be kept open 	 */
	static constexpr size_t max_section_size = 32;
	inline application(const resource::directory& dir,
			bool keepalive = false) noexcept : root(dir), maykeep(keepalive) {}
	/** services the request
	 * @param in  - input stream
	 * @param out - output stream
	 * @param pdo - optional state pdo that preserves message state between
	 *				two packets in a fragmented request.
	 *				Many browsers fragment POST and PUT on two 2 packets,
	 *				which then mayt come to micurest application separately	 */
	result_t service(istream& in, ostream& out,
			details::httpmessage::state_pdo* pdo = nullptr) const noexcept;
protected:
	using lit = details::literal<>;
	/** */
	const resource::node* read_target_and_find(details::httpmessage&) const noexcept;
	inline bool isroot(const resource::node* anode) const noexcept {
		return &root == anode;
	}
protected:
	const resource::directory& root;
	const bool maykeep;
};

static inline constexpr bool operator!(application::result_t v) noexcept {
	return v <= application::result_t::bad;
}

using resource::directory;

}
#endif
