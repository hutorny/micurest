/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * http_01.hpp - HTTP/0.1 definitions
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

/**
 * HTTP/0.1 transfer protocol
 * A subset of HTTP/1.1 necessary and sufficient for implementing µcurest
 * Normative references:
 * 	http://tools.ietf.org/html/rfc7230
 * 	http://tools.ietf.org/html/rfc7231
 *  http://tools.ietf.org/html/rfc3986
 *
 * https://tools.ietf.org/html/rfc7230#appendix-B
 * simplified version
 * HTTP-message = start-line *( header-field CRLF ) CRLF [message-body]
 * start-line = request-line / status-line
 * request-line = method SP request-target [SP HTTP-version] CRLF
 * request-target = absolute-paths
 * status-line = HTTP-version SP status-code SP reason-phrase CRLF
 */
#pragma once
namespace http_01 {
/**
 * Response Status Codes, subset of HTTP/1.1 status codes
 * Normative reference: https://tools.ietf.org/html/rfc7231#section-6.1
 */
enum class status {
	Unknown					= 0,
	OK 						= 200,	/* rfc7231#section-6.3.1 	*/
	Created					= 201,	/* rfc7231#section-6.3.2 	*/
	Accepted 				= 202,	/* rfc7231#section-6.3.3 	*/
	No_Content				= 204,	/* rfc7231#section-6.3.4 	*/
	Not_Modified			= 304,	/* rfc7232#section-4.1		*/
	Bad_Request 			= 400,	/* rfc7231#section-6.5.1 	*/
	Unauthorized			= 401,	/* rfc7235#section-3.1	 	*/
	Forbidden				= 403,	/* rfc7231#section-6.5.3 	*/
	Not_Found				= 404,	/* rfc7231#section-6.5.4 	*/
	Method_Not_Allowed		= 405,	/* rfc7231#section-6.5.5 	*/
	Not_Acceptable			= 406,	/* rfc7231#section-6.5.6 	*/
	Conflict				= 409,	/* rfc7231#section-6.5.8 	*/
	Length_Required			= 411,	/* rfc7231#section-6.5.10	*/
	Precondition_Failed		= 412,	/* rfc7232#section-4.2		*/
	Payload_Too_Large		= 413,	/* rfc7231#section-6.5.11	*/
	URI_Too_Long			= 414,	/* rfc7231#section-6.5.12	*/
	Unsupported_Media_Type	= 415,	/* rfc7231#section-6.5.13	*/
	Upgrade_Required		= 426,	/* rfc7230#section-6.7		*/
	Internal_Server_Error	= 500,	/* rfc7231#section-6.6.1	*/
	Not_Implemented			= 501,	/* rfc7231#section-6.6.2	*/
	Service_Unavailable		= 503,	/* rfc7231#section-6.6.4	*/
};

static inline constexpr unsigned short operator+(status v) noexcept {
	return static_cast<unsigned short>(v);
}

/**
 * Request method constants
 * https://tools.ietf.org/html/rfc7231#section-4.1
 */
enum class method {
	UNKNOWN				= 0,
	GET					= 1,
	POST				= 2,
	PUT					= 3,
	DELETE				= 4,
};

/**
 * Protocol version constants
 */

enum class version {
	UNKNOWN				= 0,
	_0_1				= 1,
	_1_0				= 2,
	_1_1				= 3,
};

/**
 * Protocol specific literal strings
 */
template<typename = const char*>
struct literal;

/**
 * default literal template is selected
 */
template<>
struct literal<const char*> {
	/* Protocols and versions */
	struct HTTP {
		static inline constexpr const char* HTTP_() noexcept {return "HTTP";}
		static inline constexpr const char*  _1_1() noexcept {return "1.1";}
		static inline constexpr const char*  _1_0() noexcept {return "1.0";}
		static inline constexpr const char*  _0_1() noexcept {return "0.1";}
	};
	/* Methods (supported)					}								*/
	static inline constexpr const char* GET()    noexcept { return "GET";   }
	static inline constexpr const char* PUT()    noexcept { return "PUT";   }
	static inline constexpr const char* POST()   noexcept { return "POST";  }
	static inline constexpr const char* DELETE() noexcept { return "DELETE";}
	static constexpr const char   COLON		= ':';
	static constexpr const char   SPACE		= ' ';
	static constexpr const char   PCT		= '%';
	static constexpr const char   HTAB		= '\t';
	static constexpr const char	  CR		= '\r';
	static constexpr const char	  LF		= '\n';
	static constexpr const char	  COMMA		= ',';
	static constexpr const char	  SEMI		= ';';
	static constexpr const char	  PSEP		= '/'; /* path separator */
	static constexpr const char	  TSEP		= '/'; /* token separator */
	/* Fields (of interest)													*/
	static inline constexpr const char* Accept	() noexcept { return "Accept";}
	static inline constexpr const char* Connection() noexcept {
		return "Connection";
	}
	static inline constexpr const char* keep() noexcept {
		return "keep-alive";
	}
	static inline constexpr const char* close() noexcept {
		return "close";
	}
	static inline constexpr const char* Content_Type() noexcept {
		return "Content-Type";
	}
	static inline constexpr const char* Content_Length() noexcept {
		return "Content-Length";
	}
	/* https://tools.ietf.org/html/rfc7232#section-3.1 						*/
	static inline constexpr const char* If_Match() noexcept {return "If-Match";}
	static inline constexpr const char* If_None_Match() noexcept {
		return "If-None-Match";
	}
	/* https://tools.ietf.org/html/rfc7232#section-2.3						*/
	static inline constexpr const char* ETag() noexcept { return "ETag"; }

	/** Common media types, some of
	 * http://www.iana.org/assignments/media-types/media-types.xhtml		*/
	struct media_type {
		static inline constexpr const char* application_() noexcept {
			return "application";
		}
		static inline constexpr const char* any() noexcept {
			return "*";
		}
		struct application {
			static inline constexpr const char* json() noexcept {
				return "json";
			}
			static inline constexpr const char* json_rpc() noexcept {
				return "json-rpc";
			}
			static inline constexpr const char* jsonrequest() noexcept {
				return "jsonrequest";
			}
			static inline constexpr const char* javascript() noexcept {
				return "javascript";
			}
			static inline constexpr const char* octet_stream() noexcept {
				return "octet-stream";
			}
		};
		static inline constexpr const char* text_() noexcept { return "text"; }
		struct text {
			static inline constexpr const char* css() noexcept { return "css";}
			static inline constexpr const char* csv() noexcept { return "csv";}
			static inline constexpr const char* html() noexcept {return "html";}
			static inline constexpr const char* plain() noexcept {
				return "plain";
			}
			static inline constexpr const char* xml() noexcept { return "xml";}
			/* not registered at iana.org									*/
			static inline constexpr const char* xsl() noexcept { return "xsl";}
		};
	};
	static constexpr bool is_space(char chr) noexcept {
		return chr == SPACE || chr == HTAB;
	}
	static constexpr bool is_linear(char chr) noexcept {
		return chr >= SPACE || chr == HTAB;
	}
	static constexpr bool is_type(char chr) noexcept {
		return chr >= SPACE && chr != COMMA;
	}
	static constexpr char is_hex(char chr) noexcept {
		return
			(chr >= '0' && chr <= '9') ? '0' - 0x00 :
			(chr >= 'A' && chr <= 'F') ? 'A' - 0x0A :
			(chr >= 'a' && chr <= 'f') ? 'a' - 0x0a :
			0;
	}
	static inline constexpr char lower(char chr) noexcept {
		return chr >= 'A' && chr <= 'Z' ? chr + ('a' - 'A') : chr;
	}
	/** ignore-case-comparison */
	static inline constexpr bool same(char a, char b) noexcept {
		return lower(a) == lower(b);
	}
};

typedef cojson::details::progmem<char> pstr;
template<>
struct literal<pstr> : literal<const char*> {
private:
	static constexpr const char _HTTP			[] = "HTTP";
	static constexpr const char __1_1			[] = "1.1";
	static constexpr const char __1_0			[] = "1.0";
	static constexpr const char __0_1			[] = "0.1";
	static constexpr const char _GET			[] = "GET";
	static constexpr const char _PUT			[] = "PUT";
	static constexpr const char _POST			[] = "POST";
	static constexpr const char _DELETE			[] = "DELETE";
	static constexpr const char _Accept			[] = "Accept";
	static constexpr const char _Connection		[] = "Connection";
	static constexpr const char _Content_Type	[] = "Content-Type";
	static constexpr const char _Content_Length	[] = "Content-Length";
	static constexpr const char _If_Match 		[] = "If-Match";
	static constexpr const char _If_None_Match	[] = "If-None-Match";
	static constexpr const char _ETag			[] = "ETag";
	static constexpr const char _keep			[] = "keep-alive";
	static constexpr const char _close			[] = "close";
public:
	/* Protocol versions 		*/
	struct HTTP {
		static inline constexpr pstr HTTP_() noexcept {return pstr(_HTTP);}
		static inline constexpr pstr  _1_1() noexcept {return pstr(__1_1);}
		static inline constexpr pstr  _1_0() noexcept {return pstr(__1_0);}
		static inline constexpr pstr  _0_1() noexcept {return pstr(__0_1);}
	};
	static inline constexpr pstr GET() noexcept { return pstr(_GET);   }
	static inline constexpr pstr PUT() noexcept { return pstr(_PUT);   }
	static inline constexpr pstr POST() noexcept { return pstr(_POST);  }
	static inline constexpr pstr DELETE() noexcept { return pstr(_DELETE); }
	static inline constexpr pstr Accept	() noexcept { return pstr(_Accept); }
	static inline constexpr pstr Connection() noexcept {
		return pstr(_Connection);
	}
	static inline constexpr pstr Content_Type() noexcept {
		return pstr(_Content_Type);
	}
	static inline constexpr pstr Content_Length() noexcept {
		return pstr(_Content_Length);
	}
	static inline constexpr pstr If_Match() noexcept { return pstr(_If_Match); }
	static inline constexpr pstr If_None_Match() noexcept {
		return pstr(_If_None_Match);
	}
	static inline constexpr pstr ETag() noexcept { return pstr(_ETag); }
	static inline constexpr pstr keep() noexcept {
		return pstr(_keep);
	}
	static inline constexpr pstr close() noexcept {
		return pstr(_close);
	}

	/** Common media types, some of
	 * http://www.iana.org/assignments/media-types/media-types.xhtml		*/
	struct media_type {
	private:
		static constexpr const char _application_[] = "application";
		static constexpr const char _text_[] = "text";
	public:
		static inline constexpr const pstr application_() noexcept {
			return pstr(_application_);
		}
		struct application {
		private:
			static constexpr const char _json[]	= "json";
			static constexpr const char _json_rpc[]	= "json-rpc";
			static constexpr const char _jsonrequest[]	= "jsonrequest";
			static constexpr const char _javascript[] = "javascript";
			static constexpr const char _octet_stream[] = "octet-stream";
		public:
			static inline constexpr pstr json() noexcept { return pstr(_json); }
			static inline constexpr pstr json_rpc() noexcept {
				return pstr(_json_rpc);
			}
			static inline constexpr pstr jsonrequest() noexcept {
				return pstr(_jsonrequest);
			}
			static inline constexpr pstr javascript () noexcept {
				return pstr(_javascript);
			}
			static inline constexpr pstr octet_stream () noexcept {
				return pstr(_octet_stream);
			}
		};
		static constexpr const pstr text_() noexcept { return pstr(_text_); }
		struct text {
		private:
			static constexpr const char _css	[] = "css";
			static constexpr const char _csv	[] = "csv";
			static constexpr const char _html	[] = "html";
			static constexpr const char _plain	[] = "plain";
			static constexpr const char _xml	[] = "xml";
			static constexpr const char _xsl	[] = "xsl";
		public:
			static inline constexpr pstr css() noexcept { return pstr(_css);  }
			static inline constexpr pstr csv() noexcept { return pstr(_csv);  }
			static inline constexpr pstr html() noexcept { return pstr(_html); }
			static inline constexpr pstr plain() noexcept {return pstr(_plain);}
			static inline constexpr pstr xml() noexcept { return pstr(_xml);  }
			static inline constexpr pstr xsl() noexcept { return pstr(_xsl);  }
		};
	};
};
}
