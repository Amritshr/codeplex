// For find we use a pretty straightforward linear search, with some opportunities
// for early out if one particular thread finds the sought-for value. We also provide
// a (n overloaded) version for restrict(amp) contexts, which can allow for find
// using all lanes associated with a thread.
#pragma once
#ifndef _AMP_FIND_HPP_BUMPTZI
#define _AMP_FIND_HPP_BUMPTZI

#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename I, typename T>
	inline I find(I f, I l, const T& val, unsigned int off = 1u) restrict(amp)
	{
		while (f < l) {
			if (*f == val) return (f);
			f += off;
		}

		return (l);
	}

	template<typename AV, typename T>
	inline unsigned int find(const AV& r, const T& val)
	{
		const concurrency::array_view<unsigned int, 1> found_idx(1);
		found_idx[{0}] = r.extent.size();

		concurrency::parallel_for_each(r.extent, [=](const concurrency::index<1>& idx) restrict(amp) {
			if (found_idx[{0}] != r.extent.size()) return; // The value has already been found;

			if (R[idx] == val) found_idx[{0}] = idx[0];
		});

		return (found_idx[{0}]);
	}
	template<typename I, typename T>
	I find(I f, I l, const T& val)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		return (f + find(concurrency::array_view<const Iterator_value_type<I>, 1>(std::distance(f, l), &*f), val));
	}
}	   // Namespace experimental.
#endif // _AMP_FIND_HPP_BUMPTZI