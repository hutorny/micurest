/*
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micurpc.cpp - JSON RPC implementation
 *
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

#ifndef MICURPC_HPP_
#define MICURPC_HPP_
#include "micurest.hpp"
/*
 * RPC call is a POST or PUT method containing a JSON object in its body
 * and expecting a JSON object as a response
 *
 * JSON-RPC http://www.jsonrpc.org/specification
 * JSON-RPC over HTTP: http://www.jsonrpc.org/historical/json-rpc-over-http.html
 * defines structure where method name is embedded in the request
 * with method specific params.
 * To use cojson approach, the params structure must be selected after the
 * method name is read, before the parsing completes.
 *
 * In µcuRPC design, each RPC-callable procedure is implemented as a C++
 * class that includes:
 * - method implementation
 * - call-specific structures for params and result,
 * - with their cojson structures
 * ? static method returning RPC-method name
 * - static factory method
 *
 * A set of RPC procedures is aggregated in an API object
 * which is then must be placed in a µcuREST hierarchy.
 *
 * On an rpc call, the following flow of events occurs:
 * - µcuREST application receives a request
 * - when processing the request URI, an µcuRPC API object found and called
 * - the API object starts parsing JSON body
 * - the JSON body MUST contain member name 'method' that MUST go before the
 *   params
 * - when the API object reads member name params, it lookups the list
 *   of procedures and finds corresponding procedure
 * - API object calls procedure's factory to initialize the procedure object
 * - API object calls procedure's read method to read the parameters
 * - API object finishes reading the request object
 * - API object calls procedure's run method
 * - the run method executes user code, which is expected to fill
 *   proprietary result structure and, on error the error parameter
 * - API object starts writing response object
 * - when the API object writes member result it calls procedure's write method
 *   to write the result
 * - API object finishes writing response object
 * - µcuREST application finishes writing response
 *
 * Because the procedure object gets control in three phases, its state
 * has to be saved and kept between phases. Use of static storage for
 * each procedure would increase RAM demands.
 * To solve this design challenge, µcuRPC allocates space in the stack
 * for a largest procedure object and uses placed new to initialize the
 * current procedure object. The procedure may still use static storage
 * if params and result are declared static (per procedure) or µcuRPC
 * configured to use shared static storage.
 *
 *
 * Implementation limitations:
 * IL#1. request.id MUST be a string or null
 * IL#2. error.data is not supported
 * IL#3. params by order not recommended
 */

namespace micurpc {
using micurest::char_t;
using micurest::size_t;
using micurest::cstring;
using micurest::name;
using micurest::resource::node;
using micurest::details::message;
using micurest::media;

struct config {
	static constexpr size_t max_id_length = 32;
	static constexpr size_t max_method_length = 32;
};

/****************************************************************************
 * 							JSON RPC Standard								*
 ****************************************************************************/
enum error_code : short {
	no_errors			= 0,
	/** Parse error
	 * Invalid JSON was received by the server.
	 * An error occurred on the server while parsing the JSON text.		*/
	parse_error			= -32700,
	/** Invalid Request
	  * The JSON sent is not a valid Request object.					*/
	invalid_request		= -32600,
	/** Method not found
		The method does not exist / is not available.					*/
	method_not_found	= -32601,
	/** Invalid params
		Invalid method parameter(s).									*/
	invalid_params		= -32602,
	/** Internal error
		Internal JSON-RPC error.										*/
	internal_error		= -32603,
	/** Server error
		Reserved for implementation-defined server-errors.				*/
	server_error		= -32000,
	server_error99		= -32099
};

/**
 * Request object
 */
struct request {
	/** A String specifying the version of the JSON-RPC protocol.
	 *  MUST be exactly "2.0".												*/
	char_t jsonrpc[4];
	/** A String containing the name of the method to be invoked.			*/
	char_t method[config::max_method_length];
	/** A Structured value that holds the parameter values
	 *  to be used during the invocation of the method.
	 *  This member MAY be omitted.											*/
	/* params */
	/** An identifier established by the Client that MUST contain a String,
	 *  Number, or NULL value if included. If it is not included it is assumed
	 *  to be a notification.
	 *  The value SHOULD normally not be Null and
	 *  Numbers SHOULD NOT contain fractional parts							*/
	char_t id[config::max_id_length];
};

/**
 * Response object (success)
 */
struct response {
	/** A String specifying the version of the JSON-RPC protocol.
	 * MUST be exactly "2.0".
	 */
	char_t jsonrpc[4];
	/**
	 * This member is REQUIRED on success.
	 * This member MUST NOT exist if there was an error invoking the method.
   	 * The value of this member is determined by the method invoked on
   	 * the Server.
	 */
	//TODO result
	struct error_t {
		/**	A Number that indicates the error type that occurred.
		 *  This MUST be an integer.										*/
		error_code code;
		/** A String providing a short description of the error.
		 *  The message SHOULD be limited to a concise single sentence.
		 */
		cstring message;
		/* A Primitive or Structured value that contains additional information
		 * about the error. This may be omitted.
		 *  The value of this member is defined by the Server
		 *  (e.g. detailed error information, nested errors etc.).			*/
		//data
		short get() const noexcept { return static_cast<short>(code); }
		void set(short) noexcept { /* left blank intentionally */ }
	};

	/**	This member is REQUIRED on error.
	 *	This member MUST NOT exist if there was no error triggered
	 *	during invocation.
	 *	The value for this member MUST be an Object as defined in section 5.1.
	 */
	error_t error;

	/** This member is REQUIRED on success
	  * It MUST be the same as the value of the id member in the Request Object.
	  * If there was an error in detecting the id in the Request object
	  * (e.g. Parse error/Invalid Request), it MUST be Null.				*/
	const char_t * id;
};

/****************************************************************************
 * 							µcuRPC Implementations							*
 ****************************************************************************/

namespace details {

	using namespace cojson;
	using namespace cojson::details;
	using namespace micurest::details;

	struct procedure;
	typedef procedure* (*factory)(void* space);

	void copy(char* dst, progmem<char> src, size_t n) noexcept;
	void copy(char* dst, const char* src, size_t n) noexcept;
	//void copy(char* dst, const char* src, size_t n) noexcept {
	//	strncpy(dst,src,n);
	//}



	/** JSON-RPC member names 											*/
	template<typename=cstring>
	struct literal;

	template<>
	struct literal<const char*> {
		static inline constexpr const char* code() noexcept { return "code"; }
		static inline constexpr const char* error() noexcept { return "error"; }
		static inline constexpr const char* id() noexcept { return "id"; }
		static inline constexpr const char* jsonrpc() noexcept { return "jsonrpc"; }
		static inline constexpr const char* message() noexcept { return "message"; }
		static inline constexpr const char* method() noexcept { return "method"; }
		static inline constexpr const char* params() noexcept { return "params"; }
		static inline constexpr const char* result() noexcept { return "result"; }
		static inline constexpr const char* _2_0() noexcept { return "2.0"; }
		//TODO consider moving to micurest::literal or cojson::literal
		static inline void copy(char* dst, const char * src, size_t n) noexcept{
			strncpy(dst,src,n);
		}

		template<size_t N>
		static inline void copy(char (&dst)[N], const char *src) noexcept {
			copy(dst,src,N);
		}
	};
	template<>
	struct literal<progmem<char>> {
		typedef progmem<char> pgmstr;
		static inline pgmstr code() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "code";
			return pgmstr(s);
		}
		static inline pgmstr error() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "error";
			return pgmstr(s);
		}
		static inline pgmstr id() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "id";
			return pgmstr(s);
		}
		static inline pgmstr jsonrpc() noexcept {
			static constexpr const char s[] __attribute__((progmem))= "jsonrpc";
			return pgmstr(s);
		}
		static inline pgmstr message() noexcept {
			static constexpr const char s[] __attribute__((progmem))= "message";
			return pgmstr(s);
		}
		static inline pgmstr method() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "method";
			return pgmstr(s);
		}
		static inline pgmstr params() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "params";
			return pgmstr(s);
		}
		static inline pgmstr result() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "result";
			return pgmstr(s);
		}
		static inline const pgmstr _2_0() noexcept {
			static constexpr const char s[] __attribute__((progmem)) = "2.0";
			return pgmstr(s);
		};

		template<size_t N>
		static inline void copy(char (&dst)[N], pgmstr src) noexcept {
			copy(dst,src,N);
		}

	};

	template<typename T, typename ... L>
	struct max_of {
		static constexpr size_t size =
			sizeof(T) >= max_of<L...>::size ?
			sizeof(T)  : max_of<L...>::size;
		static constexpr size_t align =
			sizeof(T) >= max_of<L...>::align ?
			sizeof(T)  : max_of<L...>::align;
	};

	template<typename T>
	struct max_of<T> {
		static constexpr size_t size  = sizeof(T);
		static constexpr size_t align = alignof(T);
	};


	/**
	 * string class property via a method
	 * to workaround problem with accessing properties of the base class
	 */
	template<class C, name id, size_t N, char_t* (C::*M)()>
	const property<C> & PS() noexcept {
		static const struct local : property<C> {
			cstring name() const noexcept { return id(); }
			bool read(C& obj, lexer& in) const noexcept {
				return reader<char_t*>::read((obj.*M)(), N, in);
			}
			bool write(const C& obj, ostream& out) const noexcept {
				return writer<const char_t*>::write(
					(const_cast<C&>(obj).*M)(), out);
			}
		} l;
		return l;
	}

	/**
	 * constant string class property via a pointer to member
	 */
	template<class C, name id, cstring C::*M>
	const property<C> & PM() noexcept {
		static const struct local : property<C> {
			cstring name() const noexcept { return id(); }
			bool read(C&, lexer& in) const noexcept {
				in.error(details::error_t::noobject);
				return false;
			}
			bool write(const C& obj, ostream& out) const noexcept {
				return writer<cstring>::write(obj.*M, out);
			}
		} l;
		return l;
	}

	/**
	 * constant string class property via a pointer to method returning string
	 * to workaround problem with accessing properties of the base class
	 */
	template<class C, name id, const char_t* (C::*M)() const>
	const property<C> & PG() noexcept {
		static const struct local : property<C> {
			cstring name() const noexcept { return id(); }
			bool read(C&, lexer& in) const noexcept {
				in.error(details::error_t::noobject);
				return false;
			}
			bool write(const C& obj, ostream& out) const noexcept {
				return writer<const char_t*>::write(((obj).*M)(), out);
			}
		} l;
		return l;
	}

	template<class C,name id,class T,T& (C::*M)(),const clas<T>& S()>
	const property<C> & PR() {
		static const struct local : property<C> {
			cstring name() const noexcept { return id(); }
			bool read(C& obj, lexer& in) const noexcept {
				return S().read((obj.*M)(), in);
			}
			bool write(const C& obj, ostream& out) const noexcept {
				return S().write((const_cast<C&>(obj).*M)(), out);
			}
		} l;
		return l;
	}

/**
 * procedure - an abstract remote procedure
 */
struct procedure {
	virtual ~procedure() noexcept {
	}
	/**
	 * Called to read the parameters
	 */
	virtual bool read(lexer&) noexcept = 0;
	/**
	 * Executes the user's code.
	 * On error set error.error to a non-zero value
	 */
	virtual void run(response::error_t& error) noexcept = 0;
	/**
	 * Called to write the result
	 */
	virtual bool write(ostream&) const noexcept = 0;
	/**
	 * Factory for constructing a procedure of user's class P
	 */
	template<class P>
	static procedure* factory(void* space) noexcept {
		static_assert(std::is_base_of<procedure,P>::value,
				"Template argument is not derived from procedure class");
		return new (space) P; /* placement new */
	}
	/**
	 * Disposes factory-allocated procedure
	 */
	static inline void dispose(procedure* proc) noexcept {
		proc->~procedure();
	}
	/**
	 * Returns name of the RPC method
	 * Must be replaced in the descendant class
	 */
	static cstring method() noexcept;
};

/**
 * Base class for API implementation
 * Provides all methods, not depending on user's procedures
 */
class api {
protected:
	/**
	 * Type containing JSON RPC member names
	 */
	using lit = details::literal<cstring>;
	/**
	 * JSON-RPC request implementation
	 */
	struct request : micurpc::request {
		constexpr inline request() noexcept
		  : micurpc::request({{0},{0},{0}}), proc(nullptr) {}
		char_t* _jsonrpc() noexcept { return jsonrpc; }
		char_t* _method() noexcept { return method; }
		char_t* _id() noexcept { return id; }
		procedure* proc;

		virtual bool find() noexcept = 0;

		inline bool read(response::error_t& err, istream& body) noexcept {
			static request::json json; //TODO better static or on stack?
			lexer in(body);
			if( ! json.read(*this, in) ) {
				err.code = error_code::parse_error;
			} else {
				//TODO check for overrun and other errors
				if( ! match(lit::_2_0(), jsonrpc) || method[0] == 0 ) {
					err.code = error_code::invalid_request;
				} else
					return true;
			}
			return false;
		}

		/**
		 * cojson structure for JSON-RPC request object
		 */
		struct json : clas<request> {
			static constexpr size_t count = 3;
			static constexpr typename property<request>::node props[count] {
				PS<request, lit::jsonrpc,
					countof(&micurpc::request::jsonrpc), &request::_jsonrpc>,
				PS<request, lit::method,
					countof(&micurpc::request::method), &request::_method>,
				PS<request, lit::id,
					countof(&micurpc::request::id), &request::_id>
			};
			inline json() : clas<request>(props, count) {}
			bool read(request& req, lexer& in) const noexcept {
				return collection<indexer>::read(*this,req,in);
			}
			/* call-back for indexer */
			inline bool read(request& req, lexer& in,
							 const char_t * n) const noexcept {
				if( match(lit::params(), n))
					return params(req, in);
				else
					return clas<request>::read(req, in, n);
			}
			/**
			 * JSON-RPC params reader
			 * Lookups registered method factory via derived class
			 * Instantiates procedure via factory
			 * Calls procedure to read the parameters
			 */
			static inline bool params(request& req, lexer& in) {
				if(  req.method[0] == 0 ||  /* no method specified */
				   !( req.find()) ) /* no method found */
					return in.skip();
				req.proc->read(in);
				return true;
			}
		};

	};

	/**
	 * JSON-RPC response implementation
	 */
	struct response : micurpc::response {
		inline response(const char* id_) noexcept
		  : micurpc::response{{},{no_errors,cstring(nullptr)},{id_}},
			proc(nullptr) {
			lit::copy(jsonrpc, lit::_2_0());
		  }
		const char_t* _jsonrpc() const noexcept { return jsonrpc; }
		const char_t* _id() const noexcept { return id; }
		error_t& errorm() noexcept { return error; }
		procedure* proc;

		inline void write(ostream& out) const noexcept {
			response::json json(error.code==error_code::no_errors);
			json.write(*this,out);
		}

		/**
		 * cojson structure for the JSON-RPC response
		 */
		struct json : clas<response> {
			static constexpr size_t count = 3;
			using node = typename property<response>::node;
			static const clas<response::error_t>& err() noexcept {
				return O<response::error_t,
					P<response::error_t, lit::code,
						accessor::methods<response::error_t, short,
						&response::error_t::get, &response::error_t::set>>,
					PM<response::error_t, lit::message,
						&response::error_t::message>
					>();
			}
			static constexpr node fail[count] = {
				PG<response, lit::jsonrpc, &response::_jsonrpc>,
				PG<response, lit::id, &response::_id>,
				PR<response, lit::error,
					response::error_t, &response::errorm, &json::err>
			};
			static constexpr node success[count] = {
				PG<response, lit::jsonrpc, &response::_jsonrpc>,
				PG<response, lit::id, &response::_id>,
				P<response, lit::result, procedure*, &response::proc>
			};
			inline json(bool ok) noexcept
			  : clas<response>(ok?success:fail,count) {
				PR<response, lit::error,
					response::error_t, &response::errorm, &json::err>();
				P<response, lit::result,
					decltype(response::proc), &response::proc>();
			}

		};
	};

	/**
	 * µcuREST node for placing the API among application URI's
	 */
	struct rest_node : node {
		media::type mediatype() const noexcept {
			return media::json;
		}
		void get(message& msg) const noexcept {
			/* get is not supported */
			msg.error(micurest::status_t::Not_Implemented);
		}
		void post(message& msg) const noexcept {
			/* POST and PUT are handled the same way */
			put(msg);
		}

		static inline void write(message& msg, response& res) noexcept {
		/* 	http://www.jsonrpc.org/historical/json-rpc-over-http.html
			3.6.2   Errors													*/
			switch(res.error.code) {
			case error_code::no_errors:
				if( res.id[0] == 0 ) {
					/* for JSON-RPC Notification requests, a success response
					 * MUST be an HTTP status code: 204.					*/
					msg.status(micurest::status_t::No_Content);
					msg.seal();
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
			res.write(msg.obody());
		}
	};
};

}
/****************************************************************************
 * 							µcuRPC Definitions								*
 ****************************************************************************/

using details::procedure;
using micurest::resource::node;

/**
 * api - Main µcuRPC template
 * L is a list of classes implementing RPC calls
 * Each class must:
 * - be derived from micurpc::procedure class,
 * - implement abstract methods and
 * - have a default constructor
 * - provide method 'method' returning the RPC method's name
 */

template<typename ... L>
class api : details::api {
public:
	static constexpr size_t space_size = details::max_of<L...>::size;
	static constexpr size_t space_align = details::max_of<L...>::align;
	static const node& rest() noexcept {
		static const rest_node l;
		return l;
	}
	/**
	 * service method
	 * For use outside of µcuREST framework
	 */
	static inline void service(details::istream& in,
			details::ostream& out) noexcept {
		request req;
		response res(req.id);
		if( req.read(res.error, in) ) {
			if( nullptr == (res.proc = req.proc) ) {
				res.error.code = error_code::internal_error;
			} else {
				req.proc->run(res.error);
			}
		}
		res.write(out);
		if( req.proc )
			procedure::dispose(req.proc);
		return;

	}
private:
	struct request : details::api::request {
//		alignas(space_align)
		char space[space_size]; //TODO configurable static

		bool find() noexcept {
			//TODO consider matching id within the factory itslef
			static details::factory list[] =  { L::template factory<L> ... };
			static constexpr name names[] = { L::method ... };
			static constexpr size_t  n = sizeof...(L);

			for(size_t i = 0; i < n; ++i)
				if( details::match(names[i](), method)) {
					proc = list[i](space);
					return true;
				}
			return false;
		}

	};
	using details::api::response;

	struct rest_node : details::api::rest_node {
		void put(message& msg) const noexcept {
			if( msg.req().content_type == mediatype() ) {
				request req;
				response res(req.id);
				if( req.read(res.error, msg.ibody()) ) {
					if( nullptr == (res.proc = req.proc) ) {
						res.error.code = error_code::internal_error;
					} else {
						req.proc->run(res.error);
					}
				}
				write(msg, res);
				if( req.proc )
					procedure::dispose(req.proc);
				return;
			}
			msg.bad_content();
		}

	};
};

} //namespace micurpc

namespace cojson {
namespace details {
/**
 * writer for writing procedure results
 */
template<>
struct writer<micurpc::procedure*> {
	static inline bool write(micurpc::procedure* p, ostream& out) noexcept  {
		return p ? p->write(out) : object::null(out);
	}
};
template<>
struct reader<micurpc::procedure*> {
	static bool read(micurpc::procedure*, lexer& in) noexcept {
		in.error(error_t::noobject);
		return false;
	}
};

}}

#endif /* MICURPC_HPP_ */
