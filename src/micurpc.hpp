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

#pragma once
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
 * - static method returning RPC-method name
 *
 * A set of RPC procedures (classes) is aggregated with an API class
 * which is then is placed in a µcuREST resource map for access vis HTTP
 *
 * On an RPC call, the following flow of events occurs:
 * - µcuREST application receives a request
 * - when processing the request URI, it finds µcuRPC API resource
 * - the API resource object calls RPC service for parsing JSON body
 * - the JSON body MUST contain member name 'method' an it MUST go before the
 *   params
 * - when the service reads member name params, it lookups the list
 *   of procedures and finds procedure corresponding to the method
 * - service calls factory to initialize the procedure object
 * - service procedure's read_params method to read the parameters
 * - service finishes reading the request object
 * - service calls procedure's run method
 * - the run method executes user code, which is expected to fill
 *   proprietary result structure and, on error the error parameter
 * - service starts writing response object
 * - when it comes to the result property it calls procedure's write_result
 *   method to write the result
 * - service finishes writing JSON-RPC response object
 * - µcuREST application finalizes HTTP response
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
 * Implementation limitations:
 * IL#1. only PUT and POST methods a supported
 * IL#2. error.data is not supported
 * IL#3. method property must precede params propery
 */

namespace micurpc {
using micurest::char_t;
using micurest::size_t;
using micurest::cstring;
using micurest::name;
using micurest::resource::node;
using micurest::details::message;
using micurest::media;

struct default_config {
	static constexpr size_t max_id_length = 16;
	static constexpr size_t max_method_length = 32;
};

struct config : default_config {

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
	id_is_too_long		= -32001,
	method_is_too_long	= -32003,
	string_is_too_long	= -32004,
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
 * Response object (both success and error)
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
	//result
	struct error_t {
		/**	A Number that indicates the error type that occurred.
		 *  This MUST be an integer.										*/
		error_code code = error_code::no_errors;
		/** A String providing a short description of the error.
		 *  The message SHOULD be limited to a concise single sentence.
		 */
		cstring message = static_cast<cstring>(nullptr);
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
	inline response(const char_t * _id) : id{_id} {};
};

/****************************************************************************
 * 							µcuRPC Implementations							*
 ****************************************************************************/

namespace details {

	using namespace cojson;
	using namespace cojson::details;
	using namespace micurest::details;

template<typename T>
struct any {
	inline constexpr any(T a) noexcept : v(a) {}
	inline constexpr bool of(T a) const noexcept { return v == a; }
	template<typename ... L>
	inline constexpr bool of(T a, L ... l) const noexcept {
		return v == a || of(l ...);
	}
private:
	T v;
};

struct procedure;
typedef procedure* (*factory)(void* space);

void copy(char* dst, const char* src, size_t n) noexcept;

/** JSON-RPC member names 											*/
template<typename=cojson::details::char_l>
struct literal;

template<>
struct literal<char> : cojson::details::literal {
	static inline constexpr const char* code() noexcept { return "code"; }
	static inline constexpr const char* error() noexcept { return "error"; }
	static inline constexpr const char* id() noexcept { return "id"; }
	static inline constexpr const char* jsonrpc() noexcept { return "jsonrpc"; }
	static inline constexpr const char* message() noexcept { return "message"; }
	static inline constexpr const char* method() noexcept { return "method"; }
	static inline constexpr const char* params() noexcept { return "params"; }
	static inline constexpr const char* result() noexcept { return "result"; }
	static inline constexpr const char* _2_0() noexcept { return "2.0"; }
	static inline constexpr const char* id_is_too_long() noexcept {
		return "Request id is too long";
	}
	static inline constexpr const char* method_is_too_long() noexcept {
		return "Method name is too long";
	}
	static inline constexpr const char*string_is_too_long() noexcept {
		return "String name is too long";
	}

	template<size_t N>
	static inline void copy(char (&dst)[N], const char *src) noexcept {
		details::copy(dst,src,N);
	}
};

#if __AVR__
#	include "micurpc_progmem.hpp"
#endif

template<typename T, typename ... L>
struct max_of {
	static constexpr size_t size =
		sizeof(T) >= max_of<L...>::size ?
		sizeof(T)  : max_of<L...>::size;
	static constexpr size_t align =
		alignof(T) >= max_of<L...>::align ?
		alignof(T)  : max_of<L...>::align;
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
const property<C> & PropertyStringOfParent() noexcept {
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
const property<C> & PropertyStringMemeber() noexcept {
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
const property<C> & PropertyStringPointerMethod() noexcept {
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
const property<C> & PropertyReferenceMethods() noexcept {
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

template<class C>
struct bityped_property : property<C> {
	bool read(C&, lexer&) const noexcept;
	bool write(const C&, ostream&) const noexcept;
};
/**
 * dual string/integer property via methods
 * for handling id which could be
 */
template<class C, name id>
const property<C> & PropertyDualStringIntegerRead() noexcept {
	static const struct local : bityped_property<C> {
		cstring name() const noexcept { return id(); }
		bool write(const C&, ostream&) const noexcept { return false; }
	} l;
	return l;
}

/**
 * dual string/integer property via methods
 * for handling id which could be
 */
template<class C, name id>
const property<C> & PropertyDualStringIntegerWrite() noexcept {
	static const struct local : bityped_property<C> {
		cstring name() const noexcept { return id(); }
		bool read(C&, lexer&) const noexcept { return false; }
	} l;
	return l;
}

/**
 * params - a set of class members mapped to json object or json array
 */
template<class C>
struct params : noncopyable {
	using char_t = cojson::char_t;
	typedef typename property<C>::node node;
	params(const node * n, size_t s) noexcept : nodes(n), size(s) { }
	bool read(C& obj, lexer& in) const noexcept {
		char_t chr;
		if( in.skipws(chr) ) {
			in.back(chr);
			switch(chr) {
			case literal<>::begin_object:
				return collection<indexer>::read(*this, obj, in);
			case literal<>::begin_array:
				return collection<>::read(*this, obj, in);
			}
		}
		in.error(error_t::bad);
		return false;
	}
	bool write(const C& obj, ostream& out) const noexcept {
		/* only reading is needed */
		return object::null(out);
	}
	static inline constexpr bool null(C&) noexcept {
		return false; /* must not be null */
	}
protected:
	friend class collection<indexer>;
	friend class collection<iterator>;
	inline bool read(C& obj, lexer& in, const char_t * name) const noexcept {
		for(size_t i = 0; i < size; ++i) {
			const property<C>& m(nodes[i]());
			if( m.match(name) ) {
				m.read(obj, in);
				return true;
			}
		}
		in.error(error_t::mismatch);
		return false;
	}
	/** read array item implementation, returns false when last item read */
	inline bool read(C& obj, lexer& in, size_t i) const noexcept {
		if( i < size ) {
			if( nodes[i]().read(obj, in) )
				return true;
			return in.skip(false);
		} else {
			in.error(error_t::mismatch);
			return false;
		}
	}
	const node * nodes;
	const size_t size;
};

/** Parameters
 * JSON object associated with a C++ class
 */
template<class C, typename cojson::details::property<C>::node ... L>
inline const params<C>& Parameters() noexcept {
	static constexpr typename cojson::details::property<C>::node list[] { L ... } ;
	static constexpr auto size = sizeof...(L);
	static const params<C> l(list,size);
	return l;
}

class service;
class procedure;
/**
 * procedure - an abstract remote procedure
 */
class procedure {
public:
	template<class P>
	static inline procedure* factory(void* space) noexcept {
		static_assert(std::is_base_of<procedure,P>::value,
				"Template argument is not derived from procedure class");
		return new (space) P(); /* placement new */
	}
protected:
	virtual ~procedure() noexcept {	}
	/**
	 * Called to read parameters from JSON input
	 */
	virtual bool read_params(lexer&) noexcept = 0;
	/**
	 * Called to write parameters as JSON
	 */
	virtual bool write_result(ostream& out) const noexcept {
		return object::null(out);
	}
	/**
	 * A template for writing user's result
	 */
	template<typename T>
	inline bool write(T& value, ostream& out) const noexcept {
		return cojson::Write(value, out);
	}
	/**
	 * Executes the user's code.
	 * On error set error.error to a non-zero value
	 */
	virtual void run(response::error_t& error) noexcept = 0;
	/**
	 * Factory for constructing a procedure of user's class P
	 */
	inline void* operator new(size_t, void* space) noexcept { return space; }
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
	static cstring method() noexcept = delete;
	friend class service;
	friend struct writer<procedure*>;
};

/**
 * Base class for RPC service implementation
 * Provides all methods, not depending on user's procedures
 */
class service {
protected:
	/**
	 * Type containing JSON RPC member names
	 */
	using lit = literal<>;

	/**
	 * JSON-RPC request implementation
	 */
	struct request : micurpc::request {
		constexpr inline request() noexcept
		  : micurpc::request{{0},{0},{0}}, proc(nullptr) {}
		char_t* _jsonrpc() noexcept { return jsonrpc; }
		char_t* _method() noexcept { return method; }
		procedure* proc;
		enum class id_is { bad, null, number, string } idtype = id_is::null;

		virtual bool find() noexcept = 0;

		bool read(response::error_t& err, istream& body) noexcept;
		inline bool readnullid(lexer& in) noexcept {
			bool success = in.skip(ctype::null);
			if( success ) idtype = id_is::null;
			else idtype = id_is::bad;
			return success;
		}
		inline bool readstringid(lexer& in) noexcept {
			bool success =
				reader<char_t*>::read(id, countof(id), in) && ! +in.error();
			if( success ) idtype = id_is::string;
			else idtype = id_is::bad;
			return success;
		}
		bool readintid(lexer& in) noexcept;
		/**
		 * cojson structure for JSON-RPC request object
		 */
		struct json : clas<request> {
			static constexpr size_t count = 3;
			static constexpr typename property<request>::node props[count] {
				PropertyStringOfParent<request, lit::jsonrpc,
					countof(&micurpc::request::jsonrpc), &request::_jsonrpc>,
				PropertyStringOfParent<request, lit::method,
					countof(&micurpc::request::method), &request::_method>,
				PropertyDualStringIntegerRead<request, lit::id>
			};
			inline json() : clas<request>(props, count) {}
			bool read(request& req, lexer& in) const noexcept {
				return collection<indexer>::read(*this,req,in);
			}
			/* call-back for indexer */
			inline bool read(request& req, lexer& in,
							 const char_t * name) const noexcept {
				if( match(lit::params(), name))
					return params(req, in);
				else
					return clas<request>::read(req, in, name);
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
				req.proc->read_params(in); //TODO fix value ignored
				return true;
			}
		};

	};

	/**
	 * JSON-RPC response implementation
	 */
	struct response : micurpc::response {
		inline response(const char* id_,const request::id_is& _idtype) noexcept
		  : micurpc::response(id_), proc(nullptr), idtype(_idtype) {
			lit::copy(jsonrpc, lit::_2_0());
		  }
		typedef request::id_is id_is;
		const char_t* _jsonrpc() const noexcept { return jsonrpc; }
		const char_t* _id() const noexcept { return id; }
		error_t& errorm() noexcept { return error; }
		procedure* proc;
		const id_is& idtype;

		inline bool read(lexer&) noexcept { return false; }
		inline bool readnullid(lexer&) noexcept { return false; }
		inline bool readstringid(lexer&) noexcept { return false; }
		inline bool readintid(lexer&) noexcept { return false; }

		inline bool write(ostream& out) const noexcept {
			response::json json(error.code==error_code::no_errors);
			return json.write(*this,out);
		}

		/**
		 * cojson structure for the JSON-RPC response
		 */
		struct json : clas<response> {
			static constexpr size_t count = 3;
			using node = typename property<response>::node;
			static const clas<response::error_t>& err() noexcept {
				return ObjectClass<response::error_t,
					PropertyScalarAccessor<response::error_t, lit::code,
						accessor::methods<response::error_t, short,
						&response::error_t::get, &response::error_t::set>>,
					PropertyConstString<response::error_t, lit::message,
						&response::error_t::message>
					>();
			}
			static constexpr node fail[count] = {
				PropertyStringPointerMethod<response, lit::jsonrpc, &response::_jsonrpc>,
				PropertyDualStringIntegerWrite<response, lit::id>,
				PropertyReferenceMethods<response, lit::error,
					response::error_t, &response::errorm, &json::err>
			};
			static constexpr node success[count] = {
				PropertyStringPointerMethod<response, lit::jsonrpc, &response::_jsonrpc>,
				PropertyDualStringIntegerWrite<response, lit::id>,
				PropertyScalarMember<response, lit::result, procedure*, &response::proc>
			};
			inline json(bool ok) noexcept
			  : clas<response>(ok?success:fail,count) {
				PropertyReferenceMethods<response, lit::error,
					response::error_t, &response::errorm, &json::err>();
				PropertyScalarMember<response, lit::result,
					decltype(response::proc), &response::proc>();
			}

		};
	};

	/**
	 * µcuREST node for placing the API among application URI's
	 */
	struct rpc_node : node {
		media::type mediatype() const noexcept {
			return media::json_rpc;
		}
		void get(message& msg) const noexcept {
			/* get is not supported */
			msg.error(micurest::status_t::Not_Implemented);
		}
		void post(message& msg) const noexcept {
			/* POST and PUT are handled the same way */
			put(msg);
		}

	};
	static void handle(message& msg,request&) noexcept;
	static bool handle(istream&,request&,response&) noexcept;
	static void write(message& msg, response& res) noexcept;
	static inline void run(procedure* proc, response::error_t& err) noexcept {
		proc->run(err);
	}
	static inline void dispose(procedure* proc) noexcept {
		procedure::dispose(proc);
	}
};

}
/****************************************************************************
 * 							µcuRPC Definitions								*
 ****************************************************************************/

using Procedure = details::procedure;
using details::Parameters;
using micurest::resource::node;

/**
 * Service - Main µcuRPC template
 * L is a list of classes implementing RPC calls
 * Each class must:
 * - be derived from micurpc::Procedure class,
 * - implement abstract methods and
 * - have a default constructor
 * - provide method 'method' returning the RPC method's name
 */

template<typename ... L>
class Service : details::service {
public:
	static constexpr size_t space_size = details::max_of<L...>::size;
	static constexpr size_t space_align = details::max_of<L...>::align;
	static constexpr size_t count = sizeof...(L);

	static const node& rpc() noexcept {
		static const rpc_node l;
		return l;
	}
	/**
	 * service method
	 * For use outside of µcuREST framework
	 */
	using ioerr_t = cojson::details::error_t;
	static inline void service(details::istream& in,
			details::ostream& out) noexcept {
		request req;
		response res(req.id);
		if( handle(in, req, res) )
			run(req.proc, res.error);
		res.write(out);
		if( req.proc )
			Procedure::dispose(req.proc);
		return;
	}
	static inline cstring nameof(size_t i) noexcept {
		static constexpr name names[] = { L::method ... };
		return i < count ? names[i]() : cstring(nullptr);
	}
private:
	struct request : details::service::request {
#	if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9 ))
		// fails with avr-g++ 4.8, compiles with g++ 4.9
		char space[space_size] alignas(space_align); //TODO configurable static
#	else
		char space[space_size] alignas(4); // workaround
		static_assert(space_align < 4, "space is misaligned");
#	endif

		bool find() noexcept {
			//TODO consider matching id within the factory itslef
			static details::factory list[] =  { L::template factory<L> ... };
			for(size_t i = 0; i < count; ++i)
				if( details::match(nameof(i), method)) {
					proc = list[i](space);
					return true;
				}
			return false;
		}

	};
	using details::service::response;

	struct rpc_node : details::service::rpc_node {
		void put(message& msg) const noexcept {
			request req;
			handle(msg, req);
			return;
		}

	};
};

/** Params::Names::FieldPointers::json
 *  A shortcut for defining parameters as member pointers
 */
template<class Proc, typename ... Type>
struct Params {
	using name = cojson::details::name;
	template<name ... Name>
	struct Names {
		template<Type Proc::* ... Pointer>
		struct FieldPointers {
			static inline const details::params<Proc>& json() noexcept {
				return Parameters<Proc,
					cojson::details::PropertyScalarMember<Proc,Name,Type,Pointer>...>();
			}
		};
	};
};


} //namespace micurpc

namespace cojson {
namespace details {
/**
 * writer for writing procedure results
 */
template<>
struct writer<micurpc::Procedure*> {
	static inline bool write(micurpc::Procedure* p, ostream& out) noexcept  {
		return p ? p->write_result(out) : object::null(out);
	}
};
template<>
struct reader<micurpc::Procedure*> {
	static bool read(micurpc::Procedure*, lexer& in) noexcept {
		in.error(error_t::noobject);
		return false;
	}
};

}}
