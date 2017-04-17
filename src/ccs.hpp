/************************************************************************/
/* ccs.hpp 				core configuration  definitions						*/
/************************************************************************/
/* please visist this link for motivations and details					*/
/* http://hutorny.in.ua/research/cascaded-configuration-sets-for-c1y	*/
#pragma once
namespace configuration {
	/* predefined configuration set identifiers */

	namespace build {
		class Default;
		class Debug;
	}

	namespace target {
		class All;
	}

	/* sugar for easy selector definition 								*/
	template<typename Target, typename Build>
	struct Is {
		typedef Target target;
		typedef Build  build;
	};

	/** selector of active configuration set for UserClass				*/
	template<class UserClass>
	struct Selector : Is<target::All, build::Default> {};

	/* Declaration of main configuration template */
	template<class UserClass,
		typename = typename Selector<UserClass>::target,
		typename = typename Selector<UserClass>::build>
	struct Configuration;
}
