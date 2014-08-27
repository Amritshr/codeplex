// This is a pretty straightforward implementation for fill, as it does not lend itself
// to much innovation one way or the other - however we work around the most annyoing
// DirectX limitation ever. We also include overloads for restrict(amp) contexts, as 
// these have proven useful in practice.

#pragma once
#ifndef _AMP_FILL_HPP_BUMPTZI
#define _AMP_FILL_HPP_BUMPTZI

#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename I, typename T, typename Idx>
	inline void fill(I f, I l, const T& val, unsigned int off = 1u) restrict(amp)
	{
		while (f < l) {
			*f = val;
			f += off;
		}
	}

	template<typename AV, typename T>
	inline void fill(const AV& R, const T& val)
	{
		R.discard_data();

		concurrency::parallel_for_each(R.extent, [=](const concurrency::index<1>& idx) restrict(amp) { R[idx] = val; });
	}

	template<typename I, typename T>
	void fill(I f, I l, const T& val)
	{
		if (f >= l) return; // Empty range passed, probably an error.

		fill(concurrency::array_view<Iterator_value_type<I>, 1>(std::distance(f, l), &*f), val);
	}
}	   // Namespace experimental.
#endif // _AMP_FILL_HPP_BUMPTZI