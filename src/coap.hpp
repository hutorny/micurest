/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * coap.hpp - CoAP 1.0 definitions
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

namespace coap_10 {
/*
 * CoAP
 * 	http://tools.ietf.org/html/rfc7252
 */
	/** literal for the coap codes X.XX						*/
	constexpr unsigned operator "" _c(long double v) {
		return
			static_cast<unsigned>(v) * 0x20 -
			static_cast<unsigned>(v) * 100	+
			static_cast<unsigned>(v*100);
	}

	enum class method {
		/* http://tools.ietf.org/html/rfc7252#section-12.1.1 */
        GET 	= 0.01_c,
        POST    = 0.02_c,
        PUT     = 0.03_c,
        DELETE  = 0.04_c,
	};
	enum class status {
		/* http://tools.ietf.org/html/rfc7252#section-12.1.2 */
        Created                      = 2.01_c,
        Deleted                      = 2.02_c,
        Valid                        = 2.03_c,
        Changed                      = 2.04_c,
        Content                      = 2.05_c,
        Bad_Request                  = 4.00_c,
        Unauthorized                 = 4.01_c,
        Bad_Option                   = 4.02_c,
        Forbidden                    = 4.03_c,
        Not_Found                    = 4.04_c,
        Method_Not_Allowed           = 4.05_c,
        Not_Acceptable               = 4.06_c,
        Precondition_Failed          = 4.12_c,
        Request_Entity_Too_Large     = 4.13_c,
        Unsupported_Content_Format   = 4.15_c,
        Internal_Server_Error        = 5.00_c,
        Not_Implemented              = 5.01_c,
        Bad_Gateway                  = 5.02_c,
        Service_Unavailable          = 5.03_c,
        Gateway_Timeout              = 5.04_c,
        Proxying_Not_Supported       = 5.05_c,
	};

	enum class option {
        _Reserved_		=  0,
        If_Match        =  1,
        Uri_Host        =  3,
        ETag            =  4,
        If_None_Match   =  5,
        Uri_Port        =  7,
        Location_Path   =  8,
        Uri_Path        = 11,
        Content_Format  = 12,
        Max_Age         = 14,
        Uri_Query       = 15,
        Accept          = 17,
        Location_Query  = 20,
        Proxy_Uri       = 35,
        Proxy_Scheme    = 39,
        Size1           = 60,
	};

	/* http://tools.ietf.org/html/rfc7252#section-12.3 */
	struct content_format {
		enum class text {
			plain		= 0,
		};
		enum class application {
		   link_format  = 40,
		   xml          = 41,
		   octet_stream = 42,
		   exi          = 47,
		   json         = 50,
		};
	};


	/* http://tools.ietf.org/html/rfc7252#section-3 */
	struct designator {
		unsigned version:2;
		unsigned type:2;
		unsigned tkl:4; /* token length (0..8) */
	};

	struct header : designator {
		unsigned char code; /* a 3-bit class and a 5-bit detail				*/
		unsigned short  id; /* 16-bit unsigned integer in network byte order */
	};

	struct option_byte1 {
		unsigned length : 4;
		unsigned delta  : 4;
	};

	template<unsigned N>
	struct uint_;

	struct message : header {
		unsigned char token[8];
		/* Options (if any) ... */
		/* 0xFF					*/
		/* Payload (if any) ... */
	};

}


