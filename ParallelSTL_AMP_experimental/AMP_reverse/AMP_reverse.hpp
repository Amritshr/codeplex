// This is a straightforward implementation of reverse. We also provide a(n overloaded)
// version for restrict(amp) contexts.
#pragma once
#ifndef _AMP_REVERSE_HPP_BUMPTZI
#define _AMP_REVERSE_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename T0, typename T1>
	inline void swap(T0& a, T1& b) restrict(amp)
	{
		auto t = a;
		a = b;
		b = t;
	}

	template<typename I0, typename I1>
	inline void iter_swap(I0 x, I1 y) restrict(amp)
	{
		swap(*x, *y);
	}

	template<typename I>
	inline void reverse(I f, I l, unsigned int off = 1u) restrict(amp)
	{
		while (f < (l - 1)) {
			iter_swap(f, l - 1);
			f += off;
			l -= off;
		}
	}

	template<typename AV>
	inline void reverse(const AV& r)
	{
		concurrency::parallel_for_each(r.extent / 2u, [=](const concurrency::index<1>& idx) restrict(amp) {
			reverse(begin(r) + idx[0], end(r) - idx[0], r.extent.size() / 2u);
		});
	}

	template<typename I>
	void reverse(I f, I l)
	{
		if (f >= l) return; // Empty range passed, probably an error.

		reverse(concurrency::array_view<Value_type<I>>(std::distance(f, l), &*f));
	}
}	   // Namespace experimental.
#endif // _AMP_REVERSE_HPP_BUMPTZI