#pragma once
#include "cojson.ccs"

namespace configuration {
	struct Current : Is<target::All, build::Default> {};
	template<>	struct Selector<cojson::config> : Current {};
	template<>	struct Selector<cojson::details::lexer> : Current {};
}
