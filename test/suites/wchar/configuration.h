#pragma once
#include "cojson.ccs"

//#pragma message "wchar configuration in use"

namespace configuration {
	namespace build { struct Test;	}
	template<>
	struct Configuration<cojson::config, target::All, build::Test>
		:  Configuration<cojson::config, target::All, build::Default> {
#			ifdef TEST_WCHAR_T
				typedef wchar_t char_t;
#			endif
#			ifdef TEST_CHAR16_T
				typedef char16_t char_t;
#			endif
#			ifdef TEST_CHAR32_T
				typedef char32_t char_t;
#			endif
	};
	template<>
	struct Selector<cojson::config> {
		typedef build::Test  build;
		typedef target::All  target;
	};
}
