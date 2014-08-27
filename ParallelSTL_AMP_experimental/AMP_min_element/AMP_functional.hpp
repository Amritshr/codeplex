// Here we include some utility functors, which we employ when sleight of hand is
// needed to circumvent lack of a proper restrict(amp) functor coming in from upstream.
#pragma once
#ifndef _AMP_FUNCTIONAL_HPP_BUMPTZI
#define _AMP_FUNCTIONAL_HPP_BUMPTZI

namespace experimental
{
	template<typename T = void>
	struct greater {
		bool operator()(const T& x0, const T& x1) const restrict(amp) { return (x0 > x1); };
		//constexpr bool operator()(const T& x0, const T& x1) const restrict(cpu, amp) { return (x0 > x1); };
	};

	template<>
	struct greater<void> {
		template<typename T1, typename T2>
		bool operator()(T1&& x0, T2&& x1) const restrict(amp) { return (x0 > x1); };

		//template<typename T1, typename T2>
		//constexpr auto operator()(const T1& x0, const T2& x1) const restrict(amp) { return (x0 > x1); };
	};

	template<typename T = void>
	struct less {
		bool operator()(const T& x0, const T& x1) const restrict(cpu, amp) { return (x0 < x1); };
	};

	template<>
	struct less<void> {
		template<typename T1, typename T2>
		bool operator()(T1&& x0, T2&& x1) const restrict(cpu, amp) { return (x0 < x1); };

		//template<typename T1, typename T2>
		//constexpr auto operator()(T1&& x0, T2&& x1) const restrict(amp) { return (x0 < x1); };
	};
}	   // Namespace experimental.
#endif // _AMP_FUNCTIONAL_HPP_BUMPTZI