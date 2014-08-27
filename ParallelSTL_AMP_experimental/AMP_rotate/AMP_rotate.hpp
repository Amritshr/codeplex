// We do rotate by means of the reversal algorithm, which can be found e.g. in
// Bentley, J. L. (1999) Programming Pearls, pp. 209-211, and is abbreviated in
// http://www.cs.bell-labs.com/cm/cs/pearls/s02b.pdf. Investigating Gries-Mills is
// something to do in the future. We also provide a(n overloaded)version for 
// restrict(amp) contexts.
#pragma once
#ifndef _AMP_ROTATE_HPP_BUMPTZI
#define _AMP_ROTATE_HPP_BUMPTZI

#include "AMP_helpers.hpp"
#include "AMP_reverse.hpp"

#include <algorithm>
#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename I>
	inline void rotate(I f, I m, I l, unsigned int off = 1u) restrict(amp)
	{
		reverse(f, m, off);
		reverse(m, l, off);
		reverse(f, l, off);
	}

	template<typename AV>
	inline void rotate(const AV& r, unsigned int m)
	{
		reverse(r.section(concurrency::extent<1>(m)));
		reverse(r.section(concurrency::index<1>(m)));
		reverse(r);
	}

	template<typename I>
	inline void rotate(I f, I m, I l)
	{
		if (f >= l) return; // Empty range passed, probably an error.

		rotate(concurrency::array_view<Value_type<I>>(std::distance(f, l), &*f), std::distance(f, m));
	}
}	   // Namespace experimental.
#endif // _AMP_ROTATE_HPP_BUMPTZI