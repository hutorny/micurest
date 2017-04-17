/*
 * rpc.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: Eugene
 */


#include "micurpc.hpp"
#include "miculog.hpp"

using cojson::details::clas;
using cojson::O;
using cojson::P;
using cojson::V;
using namespace cojson::details;
using namespace micurpc;
using namespace miculog;

#define NAME(s) static inline constexpr cstring s() noexcept {return #s;}
#define ALIAS(f,s) static inline constexpr cstring f() noexcept {return #s;}

namespace name {
	NAME(numeric)
	NAME(logical)
	NAME(rpc)
	NAME(MyProc)
}

class MyProc : public Procedure {
public:
	bool read_params(lexer& in) noexcept {
		return
		Params<MyProc,decltype(numeric), decltype(logical)>::
		Names<name::numeric, name::logical>::
		FieldPointers<&MyProc::numeric,&MyProc::logical>::json().
		read(*this, in);
	}
	bool write_result(ostream& out) const noexcept {
		return write(result, out);
	}
	void run(response::error_t& error) noexcept {
		log.trace("\r\n%s(%d,%d)\n", method(), numeric, logical);
		if( logical && numeric < 0 ) error = response::error_t{
			error_code::invalid_params, "numeric must be positive when logical is true"};
		if( logical ) result = numeric * numeric;
		result *= (numeric + 1);
	}
	static cstring method() noexcept { return name::MyProc(); }
private:
	int  numeric = 0;
	bool logical = false;
	int  result = 0;
	static Log<MyProc> log;
};

using micurest::resource::node;

const micurest::resource::node& rpc_node() noexcept { return Service<MyProc>::rpc(); }

cstring rpc_list(size_t i) noexcept {
	return Service<MyProc>::nameof(i);
}

cstring rpc_constants() noexcept {
	return "{}"; // no constants in this example
}
